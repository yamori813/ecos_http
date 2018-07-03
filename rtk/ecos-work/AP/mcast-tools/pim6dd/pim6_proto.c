/*	$KAME: pim6_proto.c,v 1.9 2003/12/24 10:21:09 suz Exp $	*/

/*
 * Copyright (C) 1998 WIDE Project.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 *  Copyright (c) 1998 by the University of Oregon.
 *  All rights reserved.
 *
 *  Permission to use, copy, modify, and distribute this software and
 *  its documentation in source and binary forms for lawful
 *  purposes and without fee is hereby granted, provided
 *  that the above copyright notice appear in all copies and that both
 *  the copyright notice and this permission notice appear in supporting
 *  documentation, and that any documentation, advertising materials,
 *  and other materials related to such distribution and use acknowledge
 *  that the software was developed by the University of Oregon.
 *  The name of the University of Oregon may not be used to endorse or 
 *  promote products derived from this software without specific prior 
 *  written permission.
 *
 *  THE UNIVERSITY OF OREGON DOES NOT MAKE ANY REPRESENTATIONS
 *  ABOUT THE SUITABILITY OF THIS SOFTWARE FOR ANY PURPOSE.  THIS SOFTWARE IS
 *  PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, TITLE, AND 
 *  NON-INFRINGEMENT.
 *
 *  IN NO EVENT SHALL UO, OR ANY OTHER CONTRIBUTOR BE LIABLE FOR ANY
 *  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES, WHETHER IN CONTRACT,
 *  TORT, OR OTHER FORM OF ACTION, ARISING OUT OF OR IN CONNECTION WITH,
 *  THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *  Other copyrights might apply to parts of this software and are so
 *  noted when applicable.
 */
/*
 *  Questions concerning this software should be directed to 
 *  Kurt Windisch (kurtw@antc.uoregon.edu)
 */
/*
 * Part of this program has been derived from PIM sparse-mode pimd.
 * The pimd program is covered by the license in the accompanying file
 * named "LICENSE.pimd".
 *  
 * The pimd program is COPYRIGHT 1998 by University of Southern California.
 *
 * Part of this program has been derived from mrouted.
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE.mrouted".
 * 
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 *
 */

#include "defs.h"

/*
 * Local functions definitions.
 */
static int pim6dd_parse_pim6_hello __P((char *pktPtr, int datalen,
				 struct sockaddr_in6 *src,
				 u_int16 *holdtime));
static void pim6dd_delayed_join_job __P((void *));
static void pim6dd_schedule_delayed_join __P((mrtentry_t *, struct sockaddr_in6 *));
static void pim6dd_delayed_prune_job __P((void *));
static void pim6dd_schedule_delayed_prune __P((mrtentry_t *, mifi_t, u_int16));
static int pim6dd_compare_metrics __P((u_int32 local_preference,
				u_int32 local_metric,
				struct sockaddr_in6 *pim6dd_local_address,
				u_int32 remote_preference,
				u_int32 remote_metric,
				struct sockaddr_in6 *remote_address));
static int pim6dd_retransmit_pim6_graft __P((mrtentry_t *));
static void pim6dd_retransmit_all_pim6_grafts __P((void *));

if_set pim6dd_nbr_mifs;    /* Mifs that have one or more neighbors attached */

/************************************************************************
 *                        PIM_HELLO
 ************************************************************************/
int
pim6dd_receive_pim6_hello(src, pim_message, datalen)
	struct sockaddr_in6 *src;
	register char *pim_message;
	int datalen;
{
	mifi_t mifi;
	struct uvif *v;
	register pim_nbr_entry_t *nbr, *prev_nbr, *new_nbr;
	u_int16 holdtime;
	u_int8  *data_ptr;
	int state_change;
	srcentry_t *srcentry_ptr;
	srcentry_t *srcentry_ptr_next;
	mrtentry_t *mrtentry_ptr;
	u_long random_delay;

	if ((mifi = pim6dd_find_vif_direct(src)) == NO_VIF) {
		/* Either a local vif or somehow received PIM_HELLO from
		 * non-directly connected router. Ignore it.
		 */
		if (pim6dd_local_address(src) == NO_VIF)
			pim6dd_log_msg(LOG_INFO, 0,
			    "Ignoring PIM_HELLO from non-neighbor router %s",
			    pim6dd_inet6_fmt(&src->sin6_addr));
		return(FALSE);
	}

	v = &pim6dd_uvifs[mifi];
	if (v->uv_flags & (VIFF_DOWN | VIFF_DISABLED))
		return(FALSE);    /* Shoudn't come on this interface */
	data_ptr = (u_int8 *)(pim_message + sizeof(struct pim));

	/* Get the Holdtime (in seconds) from the message. Return if error. */
	if (pim6dd_parse_pim6_hello(pim_message, datalen, src, &holdtime) == FALSE)
		return(FALSE);
	IF_DEBUG(DEBUG_PIM_HELLO | DEBUG_PIM_TIMER)
		pim6dd_log_msg(LOG_DEBUG, 0, "PIM HELLO holdtime from %s is %u",
		    pim6dd_inet6_fmt(&src->sin6_addr), holdtime);
    
	for (prev_nbr = (pim_nbr_entry_t *)NULL, nbr = v->uv_pim_neighbors;
	     nbr != (pim_nbr_entry_t *)NULL;
	     prev_nbr = nbr, nbr = nbr->next) {
		/* The PIM neighbors are sorted in decreasing order of the
		 * network addresses (note that to be able to compare them
		 * correctly we must translate the addresses in host order.
		 */
		if (pim6dd_inet6_lessthan(src, &nbr->address))
			continue;
		if (pim6dd_inet6_equal(src, &nbr->address)) {
			/* We already have an entry for this host */
			if (0 == holdtime) {
				/*
				 * Looks like we have a nice neighbor who is
				 * going down and wants to inform us by sending
				 * "holdtime=0". Thanks buddy and see you again!
				 */
				pim6dd_log_msg(LOG_INFO, 0,
				    "PIM HELLO received: neighbor %s going down",
				    pim6dd_inet6_fmt(&src->sin6_addr));
				pim6dd_delete_pim6_nbr(nbr);
				return(TRUE);
			}
			/* Set the timer */
			nbr->timer = holdtime;
			return(TRUE);
		}
		else
			/*
			 * No entry for this neighbor. Exit the loop and create an
			 * entry for it.
			 */
			break;
	}

	/*
	 * This is a new neighbor. Create a new entry for it.
	 * It must be added right after `prev_nbr`
	 */
	new_nbr = (pim_nbr_entry_t *)malloc(sizeof(pim_nbr_entry_t));
	new_nbr->address          = *src;
	new_nbr->vifi             = mifi;
	new_nbr->timer            = holdtime;
	new_nbr->next             = nbr;
	new_nbr->prev             = prev_nbr;

	if (prev_nbr != (pim_nbr_entry_t *)NULL)
		prev_nbr->next  = new_nbr;
	else
		v->uv_pim_neighbors = new_nbr;
	if (new_nbr->next != (pim_nbr_entry_t *)NULL)
		new_nbr->next->prev = new_nbr;

	v->uv_flags &= ~VIFF_NONBRS;
	v->uv_flags |= VIFF_PIM_NBR;
	IF_SET(mifi, &pim6dd_nbr_mifs);

	/* Elect a new DR */
	if (pim6dd_inet6_lessthan(&v->uv_linklocal->pa_addr,
			   &v->uv_pim_neighbors->address)) {
		/* The first address is the new potential remote
		 * DR address and it wins (is >) over the local address.
		 */
		v->uv_flags &= ~VIFF_DR;
	}

	/*
	 * Since a new neighbour has come up, let it know your existence ASAP;
	 * compute a random value, and reset the value to the hello timer
	 * if it's smaller than the rest of the timer.
	 * XXX: not in the spec...
	 */
	random_delay = 1 + (RANDOM() % (long)(PIM_TIMER_HELLO_PERIOD - 1));
	if (random_delay < v->uv_pim_hello_timer)
		v->uv_pim_hello_timer = random_delay;

	/* Update the source entries */
	for (srcentry_ptr = pim6dd_srclist; srcentry_ptr != (srcentry_t *)NULL;
	     srcentry_ptr = srcentry_ptr_next) {
		srcentry_ptr_next = srcentry_ptr->next;

		if (srcentry_ptr->incoming == mifi)
			continue;

		for (mrtentry_ptr = srcentry_ptr->mrtlink;
		     mrtentry_ptr != (mrtentry_t *)NULL;
		     mrtentry_ptr = mrtentry_ptr->srcnext) {

			if(!(IF_ISSET(mifi, &mrtentry_ptr->oifs))) {
				state_change = 
					pim6dd_change_interfaces(mrtentry_ptr,
							  srcentry_ptr->incoming,
							  &mrtentry_ptr->pruned_oifs,
							  &mrtentry_ptr->leaves,
							  &mrtentry_ptr->asserted_oifs);
                #if 1
				if(state_change == 1)
					pim6dd_trigger_join_alert(mrtentry_ptr);
                #endif
			}
		}
	}
    
	IF_DEBUG(DEBUG_PIM_HELLO)
		pim6dd_dump_vifs(stderr);     	/* Show we got a new neighbor */
	return(TRUE);
}

void
pim6dd_delete_pim6_nbr(nbr_delete)
	pim_nbr_entry_t *nbr_delete;
{
	srcentry_t *srcentry_ptr;
	srcentry_t *srcentry_ptr_next;
	mrtentry_t *mrtentry_ptr;
	struct uvif *v;
	int state_change;

	v = &pim6dd_uvifs[nbr_delete->vifi];
    
	/* Delete the entry from the pim_nbrs chain */
	if (nbr_delete->prev != (pim_nbr_entry_t *)NULL)
		nbr_delete->prev->next = nbr_delete->next;
	else
		v->uv_pim_neighbors = nbr_delete->next;
	if (nbr_delete->next != (pim_nbr_entry_t *)NULL)
		nbr_delete->next->prev = nbr_delete->prev;
    
	if (v->uv_pim_neighbors == (pim_nbr_entry_t *)NULL) {
		/* This was our last neighbor. */
		v->uv_flags &= ~VIFF_PIM_NBR;
		v->uv_flags |= (VIFF_NONBRS | VIFF_DR | VIFF_QUERIER);
		IF_CLR(nbr_delete->vifi, &pim6dd_nbr_mifs);
	}
	else {
		if (pim6dd_inet6_greaterthan(&v->uv_linklocal->pa_addr,
				      &v->uv_pim_neighbors->address)) {
			/* The first address is the new potential remote
			 * DR address, but the local address is the winner.
			 */
			v->uv_flags |= VIFF_DR;
		}
	}

	/* Update the source entries:
	 * If the deleted nbr was my upstream, then reset incoming and
	 * update all (S,G) entries for sources reachable through it.
	 * If the deleted nbr was the last on a non-iif vif, then recalcuate
	 * outgoing interfaces.
	 */
	for (srcentry_ptr = pim6dd_srclist; srcentry_ptr != (srcentry_t *)NULL;
	     srcentry_ptr = srcentry_ptr_next) {
		srcentry_ptr_next = srcentry_ptr->next;
	
		/* The only time we don't need to scan all mrtentries is
		 * when the nbr was on the iif, but not the upstream nbr! 
		 */
		if (nbr_delete->vifi == srcentry_ptr->incoming &&
		    srcentry_ptr->upstream != nbr_delete)
			continue;

		/* Reset the next hop (PIM) router */
		if(srcentry_ptr->upstream == nbr_delete) 
			if (pim6dd_set_incoming(srcentry_ptr, PIM_IIF_SOURCE) == FALSE) {
				/*
				 * Couldn't reset it. Sorry, the next hop router
				 * toward that source is probably not
				 * a PIM router, or cannot find route at all,
				 * hence I cannot handle this source and have to
				 * delete it.
				 */
				pim6dd_delete_srcentry(srcentry_ptr);
				free((char *)nbr_delete);
				return;
			}

		for (mrtentry_ptr = srcentry_ptr->mrtlink;
		     mrtentry_ptr != (mrtentry_t *)NULL;
		     mrtentry_ptr = mrtentry_ptr->srcnext) {
			mrtentry_ptr->incoming   = srcentry_ptr->incoming;
			mrtentry_ptr->upstream   = srcentry_ptr->upstream;
			mrtentry_ptr->metric     = srcentry_ptr->metric;
			mrtentry_ptr->preference = srcentry_ptr->preference;
			state_change = 
				pim6dd_change_interfaces(mrtentry_ptr,
						  srcentry_ptr->incoming,
						  &mrtentry_ptr->pruned_oifs,
						  &mrtentry_ptr->leaves,
						  &mrtentry_ptr->asserted_oifs);
            #if 1
			if(state_change == -1) {
				pim6dd_trigger_prune_alert(mrtentry_ptr);
			} else if(state_change == 1) {
				pim6dd_trigger_join_alert(mrtentry_ptr);
			}
            #endif
		}
	}
    
	free((char *)nbr_delete);
}


/* TODO: simplify it! */
static int
pim6dd_parse_pim6_hello(pim_message, datalen, src, holdtime)
	char *pim_message;
	int datalen;
	struct sockaddr_in6 *src;
	u_int16 *holdtime;
{
	u_int8 *pim_hello_message;
	u_int8 *data_ptr;
	u_int16 option_type;
	u_int16 option_length;
	int holdtime_received_ok = FALSE;
	int option_total_length;

	pim_hello_message = (u_int8 *)(pim_message + sizeof(struct pim));
	datalen -= sizeof(struct pim);
	for ( ; datalen >= sizeof(pim_hello_t); ) {
		/* Ignore any data if shorter than (pim_hello header) */
		data_ptr = pim_hello_message;
		GET_HOSTSHORT(option_type, data_ptr);
		GET_HOSTSHORT(option_length, data_ptr);
		switch (option_type) {
		 case PIM_MESSAGE_HELLO_HOLDTIME:
			 if (PIM_MESSAGE_HELLO_HOLDTIME_LENGTH != option_length) {
				 IF_DEBUG(DEBUG_PIM_HELLO)
					 pim6dd_log_msg(LOG_DEBUG, 0,
					     "PIM HELLO Holdtime from %s: "
					     "invalid OptionLength = %u",
					     pim6dd_inet6_fmt(&src->sin6_addr),
					     option_length);
				 return (FALSE);
			 }
			 GET_HOSTSHORT(*holdtime, data_ptr);
			 holdtime_received_ok = TRUE;
			 break;
		 default:
			 /* skip any unknown options */
			 data_ptr += option_length;
			 break;
		}

		/* Move to the next option */
		/* XXX: TODO: If we are padding to the end of the 32 bit boundary,
		 * use the first method to move to the next option, otherwise
		 * simply (sizeof(pim_hello_t) + option_length).
		 */
#ifdef BOUNDARY_32_BIT
		option_total_length = (sizeof(pim_hello_t) + (option_length & ~0x3) +
				       ((option_length & 0x3) ? 4 : 0));
#else
		option_total_length = (sizeof(pim_hello_t) + option_length);
#endif /* BOUNDARY_32_BIT */
		datalen -= option_total_length;
		pim_hello_message += option_total_length;
	}
	return (holdtime_received_ok);
}


int
pim6dd_send_pim6_hello(v, holdtime)
	struct uvif *v;
	u_int16 holdtime;
{
	char   *buf;
	u_int8 *data_ptr;

	int datalen;

	buf = pim6dd_pim6_send_buf + sizeof(struct pim);
	data_ptr = (u_int8 *)buf;
	PUT_HOSTSHORT(PIM_MESSAGE_HELLO_HOLDTIME, data_ptr);
	PUT_HOSTSHORT(PIM_MESSAGE_HELLO_HOLDTIME_LENGTH, data_ptr);
	PUT_HOSTSHORT(holdtime, data_ptr);

	datalen = data_ptr - (u_int8 *)buf;

	pim6dd_send_pim6(pim6dd_pim6_send_buf, &v->uv_linklocal->pa_addr,
		  &pim6dd_allpim6routers_group, PIM_HELLO, datalen);
	v->uv_pim_hello_timer = PIM_TIMER_HELLO_PERIOD;
	return(TRUE);
}


/************************************************************************
 *                        PIM_JOIN_PRUNE
 ************************************************************************/

typedef struct {
	struct sockaddr_in6 source;
	struct sockaddr_in6 group;
	struct sockaddr_in6 target;
} join_delay_cbk_t;

typedef struct {
	mifi_t mifi;
	struct sockaddr_in6 source;
	struct sockaddr_in6 group;
	u_int16 holdtime;
} prune_delay_cbk_t;
    
static void 
pim6dd_delayed_join_job(arg)
	void *arg;
{
	mrtentry_t *mrtentry_ptr;

	join_delay_cbk_t *cbk = (join_delay_cbk_t *)arg;

	mrtentry_ptr = pim6dd_find_route(&cbk->source, &cbk->group,
				  MRTF_SG, DONT_CREATE);
	if(mrtentry_ptr == (mrtentry_t *)NULL) 
		return;

	if(mrtentry_ptr->join_delay_timerid)
		pim6dd_timer_clearTimer(mrtentry_ptr->join_delay_timerid);

	if(mrtentry_ptr->upstream)
		pim6dd_send_pim6_jp(mrtentry_ptr, PIM_ACTION_JOIN,
			     mrtentry_ptr->incoming,
			     &mrtentry_ptr->upstream->address, 0, 0);

	free(cbk);
}

static void 
pim6dd_schedule_delayed_join(mrtentry_ptr, target) 
	mrtentry_t *mrtentry_ptr;
	struct sockaddr_in6 *target;
{
	u_long random_delay;
	join_delay_cbk_t *cbk;
    
	/* Delete existing timer */
	if(mrtentry_ptr->join_delay_timerid) 
		pim6dd_timer_clearTimer(mrtentry_ptr->join_delay_timerid);

#ifdef SYSV
	random_delay = lrand48() % (long)PIM_RANDOM_DELAY_JOIN_TIMEOUT;
#else
	random_delay = RANDOM() % (long)PIM_RANDOM_DELAY_JOIN_TIMEOUT;
#endif
    
	IF_DEBUG(DEBUG_PIM_JOIN_PRUNE)
		pim6dd_log_msg(LOG_DEBUG, 0, "Scheduling join for src %s, grp %s, delay %ld",
		    pim6dd_inet6_fmt(&mrtentry_ptr->source->address.sin6_addr),
		    pim6dd_inet6_fmt(&mrtentry_ptr->group->group.sin6_addr),
		    random_delay);

	if(random_delay == 0 && mrtentry_ptr->upstream) {
		pim6dd_send_pim6_jp(mrtentry_ptr, PIM_ACTION_JOIN,
			     mrtentry_ptr->incoming,
			     &mrtentry_ptr->upstream->address, 0, 0);
		return;
	}

	cbk = (join_delay_cbk_t *)malloc(sizeof(join_delay_cbk_t));
	cbk->source = mrtentry_ptr->source->address;
	cbk->group = mrtentry_ptr->group->group;
	cbk->target = *target;

#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, 0);
#endif

	mrtentry_ptr->join_delay_timerid = 
		pim6dd_timer_setTimer(random_delay, pim6dd_delayed_join_job, cbk);
}


static void 
pim6dd_delayed_prune_job(arg)
	void *arg;
{
	mrtentry_t *mrtentry_ptr;
	if_set new_pruned_oifs;
	int state_change;

	prune_delay_cbk_t *cbk = (prune_delay_cbk_t *)arg;

	mrtentry_ptr = pim6dd_find_route(&cbk->source, &cbk->group,
				  MRTF_SG, DONT_CREATE);
	if(mrtentry_ptr == (mrtentry_t *)NULL) 
		return;
    
	if(mrtentry_ptr->prune_delay_timerids[cbk->mifi])
		pim6dd_timer_clearTimer(mrtentry_ptr->prune_delay_timerids[cbk->mifi]);

	if(IF_ISSET(cbk->mifi, &mrtentry_ptr->oifs)) {
		IF_DEBUG(DEBUG_PIM_JOIN_PRUNE)
			pim6dd_log_msg(LOG_DEBUG, 0,
			    "Deleting pruned mif %d for src %s, grp %s",
			    cbk->mifi, 
			    pim6dd_inet6_fmt(&cbk->source.sin6_addr),
			    pim6dd_inet6_fmt(&cbk->group.sin6_addr));

		IF_COPY(&mrtentry_ptr->pruned_oifs, &new_pruned_oifs);
		IF_SET(cbk->mifi, &new_pruned_oifs);
		SET_TIMER(mrtentry_ptr->prune_timers[cbk->mifi], cbk->holdtime);

		state_change = 
			pim6dd_change_interfaces(mrtentry_ptr,
					  mrtentry_ptr->incoming,
					  &new_pruned_oifs,
					  &mrtentry_ptr->leaves,
					  &mrtentry_ptr->asserted_oifs);

		/* Handle transition to negative cache */
        #if 1
		if(state_change == -1)
			pim6dd_trigger_prune_alert(mrtentry_ptr);
        #endif
	}

	free(cbk);
}

static void 
pim6dd_schedule_delayed_prune(mrtentry_ptr, mifi, holdtime) 
	mrtentry_t *mrtentry_ptr;
	mifi_t mifi;
	u_int16 holdtime;
{
	prune_delay_cbk_t *cbk;
    
	/* Delete existing timer */
	if(mrtentry_ptr->prune_delay_timerids[mifi]) 
		pim6dd_timer_clearTimer(mrtentry_ptr->prune_delay_timerids[mifi]);

	cbk = (prune_delay_cbk_t *)malloc(sizeof(prune_delay_cbk_t));
	cbk->mifi = mifi;
	cbk->source = mrtentry_ptr->source->address;
	cbk->group = mrtentry_ptr->group->group;
	cbk->holdtime = holdtime;

#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, 0);
#endif

	mrtentry_ptr->prune_delay_timerids[mifi] = 
		pim6dd_timer_setTimer((u_int16)PIM_RANDOM_DELAY_JOIN_TIMEOUT, 
			       pim6dd_delayed_prune_job, cbk);
}


/* TODO: when parsing, check if we go beyong message size */
int
pim6dd_receive_pim6_join_prune(src, pim_message, datalen)
	struct sockaddr_in6 *src;
	char *pim_message;
	register int datalen;
{
	mifi_t mifi;
	struct uvif *v;
	pim6_encod_uni_addr_t uni_target_addr;
	pim6_encod_grp_addr_t encod_group;
	pim6_encod_src_addr_t encod_src;
	u_int8 *data_ptr;
	u_int8 num_groups;
	u_int16 holdtime;
	u_int16 num_j_srcs, num_p_srcs;
	struct sockaddr_in6 source, group, target;
	struct in6_addr s_mask, g_mask;
	u_int8 s_flags;
	u_int8 reserved;
	mrtentry_t *mrtentry_ptr;
	pim_nbr_entry_t *upstream_router;
	if_set new_pruned_oifs;
	int state_change;

	if ((mifi = pim6dd_find_vif_direct(src)) == NO_VIF) {
		/*
		 * Either a local vif or somehow received PIM_JOIN_PRUNE from
		 * non-directly connected router. Ignore it.
		 */
		if (pim6dd_local_address(src) == NO_VIF)
			pim6dd_log_msg(LOG_INFO, 0,
			    "Ignoring PIM_JOIN_PRUNE from non-neighbor router %s",
			    pim6dd_inet6_fmt(&src->sin6_addr));
		return(FALSE);
	}
 
	v = &pim6dd_uvifs[mifi];
	if (pim6dd_uvifs[mifi].uv_flags & (VIFF_DOWN | VIFF_DISABLED | VIFF_NONBRS))
		return(FALSE);    /* Shoudn't come on this interface */
	data_ptr = (u_int8 *)(pim_message + sizeof(struct pim));

	/* Get the target address */
	GET_EUADDR6(&uni_target_addr, data_ptr);
	GET_BYTE(reserved, data_ptr);
	GET_BYTE(num_groups, data_ptr);
	if (num_groups == 0)
		return (FALSE);    /* No indication for groups in the message */
	GET_HOSTSHORT(holdtime, data_ptr);
	target.sin6_len = sizeof(target);
	target.sin6_family = AF_INET6;
	target.sin6_addr = uni_target_addr.unicast_addr;
	target.sin6_scope_id = pim6dd_inet6_uvif2scopeid(&target, v);

	IF_DEBUG(DEBUG_PIM_JOIN_PRUNE)
		pim6dd_log_msg(LOG_DEBUG, 0,
		    "PIM Join/Prune received from %s : target %s, holdtime %d",
		    pim6dd_inet6_fmt(&src->sin6_addr),
		    pim6dd_inet6_fmt(&target.sin6_addr),
		    holdtime);
    
	if (!pim6dd_inet6_localif_address(&target, v) &&
	    !IN6_IS_ADDR_UNSPECIFIED(&uni_target_addr.unicast_addr)) {
		/* if I am not the target of the join or prune message */
		/*
		 * Join Suppression: when receiving a join not addressed to me,
		 * if I am delaying a join for this (S,G) then cancel the delayed 
		 * join.
		 * Prune Soliticiting Joins: when receiving a prune not
		 * addressed to me on a LAN, schedule delayed join if I have
		 * downstream receivers.
		 */
		upstream_router = pim6dd_find_pim6_nbr(&target);
		if (upstream_router == (pim_nbr_entry_t *)NULL)
			return (FALSE);   /* I have no such neighbor */
		group.sin6_len = sizeof(group);
		group.sin6_family = AF_INET6;
		source.sin6_len = sizeof(source);
		source.sin6_family = AF_INET6;
		while (num_groups--) {
			GET_EGADDR6(&encod_group, data_ptr);
			GET_HOSTSHORT(num_j_srcs, data_ptr);
			GET_HOSTSHORT(num_p_srcs, data_ptr);
			if (encod_group.masklen > (sizeof(struct in6_addr) << 3))
				continue;
			MASKLEN_TO_MASK6(encod_group.masklen, g_mask);
			group.sin6_addr = encod_group.mcast_addr;
			group.sin6_scope_id = pim6dd_inet6_uvif2scopeid(&group, v);
			if (!IN6_IS_ADDR_MULTICAST(&group.sin6_addr)) {
				data_ptr +=
					(num_j_srcs + num_p_srcs) * sizeof(pim6_encod_src_addr_t);
				continue; /* Ignore this group and jump to the next */
			}

			while (num_j_srcs--) {
				GET_ESADDR6(&encod_src, data_ptr);
				source.sin6_addr = encod_src.src_addr;
				source.sin6_scope_id = pim6dd_inet6_uvif2scopeid(&source,
									  v);

				/* sanity checks */
				if (!pim6dd_inet6_valid_host(&source))
					continue;
				if (encod_src.masklen >
				    (sizeof(struct in6_addr) << 3))
					continue;

				s_flags = encod_src.flags;
				MASKLEN_TO_MASK6(encod_src.masklen, s_mask);

				/* (S,G) Join suppresion */
				mrtentry_ptr = pim6dd_find_route(&source, &group,
							   MRTF_SG, DONT_CREATE);
				if(mrtentry_ptr == (mrtentry_t *)NULL)
					continue;

				IF_DEBUG(DEBUG_PIM_JOIN_PRUNE)
					pim6dd_log_msg(LOG_DEBUG, 0,
					    "\tJOIN src %s, group %s - canceling "
					    "delayed join",
					    pim6dd_inet6_fmt(&source.sin6_addr),
					    pim6dd_inet6_fmt(&group.sin6_addr));

				/* Cancel the delayed join */
				if(mrtentry_ptr->join_delay_timerid) {
					pim6dd_timer_clearTimer(mrtentry_ptr->join_delay_timerid);
					mrtentry_ptr->join_delay_timerid = 0;
				}
			}

			while (num_p_srcs--) {
				GET_ESADDR6(&encod_src, data_ptr);
				source.sin6_addr = encod_src.src_addr;
				source.sin6_scope_id = pim6dd_inet6_uvif2scopeid(&source,
									  v);
				/* sanity checks */
				if (!pim6dd_inet6_valid_host(&source))
					continue;
				if (encod_src.masklen >
				    (sizeof(struct in6_addr) << 3))
					continue;

				s_flags = encod_src.flags;

				/* if P2P link (not addressed to me) ignore 
				 */
				if(pim6dd_uvifs[mifi].uv_flags & VIFF_POINT_TO_POINT) 
					continue;

				/*
				 * if non-null oiflist then schedule delayed join.
				 */
				mrtentry_ptr = pim6dd_find_route(&source, &group,
							  MRTF_SG, DONT_CREATE);
				if(mrtentry_ptr == (mrtentry_t *)NULL) 
					continue;

				if(!(IF_ISEMPTY(&mrtentry_ptr->oifs))) {
					IF_DEBUG(DEBUG_PIM_JOIN_PRUNE)
						pim6dd_log_msg(LOG_DEBUG, 0,
						    "\tPRUNE src %s, group %s "
						    "- scheduling delayed join",
						    pim6dd_inet6_fmt(&source.sin6_addr),
						    pim6dd_inet6_fmt(&group.sin6_addr));
		    
					pim6dd_schedule_delayed_join(mrtentry_ptr,
							      &target);
				}
			}

		} /* while groups */

		return(TRUE);
	} /* if not unicast target */

	/* I am the target of this join/prune:
	 * For joins, cancel delayed prunes that I have scheduled.
	 * For prunes, echo the prune and schedule delayed prunes on LAN or 
	 * prune immediately on point-to-point links.
	 */
	else {
		while (num_groups--) {
			GET_EGADDR6(&encod_group, data_ptr);
			GET_HOSTSHORT(num_j_srcs, data_ptr);
			GET_HOSTSHORT(num_p_srcs, data_ptr);
			IF_DEBUG(DEBUG_PIM_JOIN_PRUNE)
				pim6dd_log_msg(LOG_DEBUG, 0,
				    "PIM Join/Prune received: grp: %s plen: %d, "
				    "%d jsrc, %d psrc",
				    pim6dd_inet6_fmt(&encod_group.mcast_addr),
				    encod_group.masklen, num_j_srcs, num_p_srcs);
			if (encod_group.masklen > (sizeof(struct in6_addr) << 3))
				continue; /* Ignore this group */
			MASKLEN_TO_MASK6(encod_group.masklen, g_mask);
			group.sin6_addr = encod_group.mcast_addr;
			group.sin6_scope_id = pim6dd_inet6_uvif2scopeid(&group, v);
			if (!IN6_IS_ADDR_MULTICAST(&group.sin6_addr)) {
				data_ptr +=
					(num_j_srcs + num_p_srcs) * sizeof(pim6_encod_src_addr_t);
				continue; /* Ignore this group and jump to the next */
			}

			while (num_j_srcs--) {
				GET_ESADDR6(&encod_src, data_ptr);
				source.sin6_addr = encod_src.src_addr;
				source.sin6_scope_id = pim6dd_inet6_uvif2scopeid(&source,
									  v);
				if (!pim6dd_inet6_valid_host(&source))
					continue;
				if (encod_src.masklen >
				    (sizeof(struct in6_addr) << 3))
					continue;

				s_flags = encod_src.flags;
				MASKLEN_TO_MASK6(encod_src.masklen, s_mask);
	    
				mrtentry_ptr = pim6dd_find_route(&source, &group,
							  MRTF_SG, DONT_CREATE);
				if(mrtentry_ptr == (mrtentry_t *)NULL) 
					continue;

				IF_DEBUG(DEBUG_PIM_JOIN_PRUNE)
					pim6dd_log_msg(LOG_DEBUG, 0,
					    "\tJOIN src %s, group %s - canceling "
					    "delayed prune",
					    pim6dd_inet6_fmt(&source.sin6_addr),
					    pim6dd_inet6_fmt(&group.sin6_addr));

				/* Cancel the delayed prune */
				if(mrtentry_ptr->prune_delay_timerids[mifi]) {
					pim6dd_timer_clearTimer(mrtentry_ptr->prune_delay_timerids[mifi]);
					mrtentry_ptr->prune_delay_timerids[mifi] = 0;
				}	    
			}

			while (num_p_srcs--) {
				GET_ESADDR6(&encod_src, data_ptr);
				source.sin6_addr = encod_src.src_addr;
				source.sin6_scope_id = pim6dd_inet6_uvif2scopeid(&source,
									  v);
				if (!pim6dd_inet6_valid_host(&source))
					continue;
				s_flags = encod_src.flags;


				mrtentry_ptr = pim6dd_find_route(&source, &group,
							  MRTF_SG, DONT_CREATE);
				if(mrtentry_ptr == (mrtentry_t *)NULL) 
					continue;

				/* if P2P link (addressed to me) prune immediately
				 */
				if(pim6dd_uvifs[mifi].uv_flags & VIFF_POINT_TO_POINT) {
					if(IF_ISSET(mifi, &mrtentry_ptr->oifs)) {

						IF_DEBUG(DEBUG_PIM_JOIN_PRUNE)
							pim6dd_log_msg(LOG_DEBUG, 0,
							    "\tPRUNE(P2P) src %s,"
							    "group %s - pruning "
							    "mif",
							    pim6dd_inet6_fmt(&source.sin6_addr),
							    pim6dd_inet6_fmt(&group.sin6_addr));
		
						IF_DEBUG(DEBUG_MRT)
							pim6dd_log_msg(LOG_DEBUG, 0, "Deleting pruned mif %d for src %s, grp %s",
							    mifi, 
							    pim6dd_inet6_fmt(&source.sin6_addr),
							    pim6dd_inet6_fmt(&group.sin6_addr));

						IF_COPY(&mrtentry_ptr->pruned_oifs, &new_pruned_oifs);
						IF_SET(mifi, &new_pruned_oifs);
						SET_TIMER(mrtentry_ptr->prune_timers[mifi], holdtime);

						state_change = 
							pim6dd_change_interfaces(mrtentry_ptr,
									  mrtentry_ptr->incoming,
									  &new_pruned_oifs,
									  &mrtentry_ptr->leaves,
									  &mrtentry_ptr->asserted_oifs);
			
						/* Handle transition to negative cache */
                        #if 1
						if(state_change == -1)
							pim6dd_trigger_prune_alert(mrtentry_ptr);
                        #endif
					} /* if is pruned */
				} /* if p2p */

				/* if LAN link, echo the prune and schedule delayed
				 * oif deletion
				 */
				else {
					IF_DEBUG(DEBUG_PIM_JOIN_PRUNE)
						pim6dd_log_msg(LOG_DEBUG, 0,
						    "\tPRUNE(LAN) src %s, group "
						    "%s - scheduling delayed prune",
						    pim6dd_inet6_fmt(&source.sin6_addr),
						    pim6dd_inet6_fmt(&group.sin6_addr));

					pim6dd_send_pim6_jp(mrtentry_ptr,
						     PIM_ACTION_PRUNE, mifi,
						     &target, holdtime, 1);
					pim6dd_schedule_delayed_prune(mrtentry_ptr,
							       mifi, holdtime);
				}
			}
		} /* while groups */
	} /* else I am unicast target */

	return(TRUE);
}


int
pim6dd_send_pim6_jp(mrtentry_ptr, action, mifi, target_addr, holdtime, echo)
	mrtentry_t *mrtentry_ptr;
	int action;           /* PIM_ACTION_JOIN or PIM_ACTION_PRUNE */
	mifi_t mifi;          /* vif to send join/prune on */
	struct sockaddr_in6 *target_addr;  /* encoded unicast target neighbor */
	u_int16 holdtime;     /* holdtime */
	int echo;
{
	u_int8 *data_ptr, *data_start_ptr;
	
	data_ptr = (u_int8 *)(pim6dd_pim6_send_buf + sizeof(struct pim));
	data_start_ptr = data_ptr;

	if(echo == 0 && mrtentry_ptr->upstream == (pim_nbr_entry_t *)NULL) {
		/* No upstream neighbor - don't send */
		return(FALSE);
	}
    
	IF_DEBUG(DEBUG_PIM_JOIN_PRUNE)
		pim6dd_log_msg(LOG_DEBUG, 0,
		    "Sending %s:  vif %s, src %s, group %s, "
		    "target %s, holdtime %d",
		    action==PIM_ACTION_JOIN ? "JOIN" : "PRUNE",
		    pim6dd_inet6_fmt(&pim6dd_uvifs[mifi].uv_linklocal->pa_addr.sin6_addr),
		    pim6dd_inet6_fmt(&mrtentry_ptr->source->address.sin6_addr),
		    pim6dd_inet6_fmt(&mrtentry_ptr->group->group.sin6_addr),
		    pim6dd_inet6_fmt(&target_addr->sin6_addr), holdtime);

	PUT_EUADDR6(target_addr->sin6_addr, data_ptr);   /* encoded unicast target addr */
	PUT_BYTE(0, data_ptr);		     /* Reserved */
	*data_ptr++ = (u_int8)1;             /* number of groups */
	PUT_HOSTSHORT(holdtime, data_ptr);   /* holdtime */

	/* data_ptr points at the first, and only encoded mcast group */
	PUT_EGADDR6(mrtentry_ptr->group->group.sin6_addr,
		    SINGLE_GRP_MSK6LEN, 0, data_ptr);

	/* set the number of join and prune sources */
	if(action == PIM_ACTION_JOIN) {
		PUT_HOSTSHORT(1, data_ptr);
		PUT_HOSTSHORT(0, data_ptr);
	} else if(action == PIM_ACTION_PRUNE) {
		PUT_HOSTSHORT(0, data_ptr);
		PUT_HOSTSHORT(1, data_ptr);
	}	
    
	PUT_ESADDR6(mrtentry_ptr->source->address.sin6_addr, SINGLE_SRC_MSK6LEN,
		    0, data_ptr);

	/* Cancel active graft */
	if (echo == 0)
		pim6dd_delete_pim6_graft_entry(mrtentry_ptr);
    
	pim6dd_send_pim6(pim6dd_pim6_send_buf, &pim6dd_uvifs[mifi].uv_linklocal->pa_addr,
		  &pim6dd_allpim6routers_group, PIM_JOIN_PRUNE,
		  data_ptr - data_start_ptr);
    
	return(TRUE);
}


/************************************************************************
 *                        PIM_ASSERT
 ************************************************************************/

/* Notes on assert prefs/metrics
 *   - For downstream routers, compare pref/metric previously received from
 *     winner against those in message.
 *     ==> store assert winner's pref/metric in mrtentry
 *   - For upstream router compare my actualy pref/metric for the source
 *     against those received in message.
 *     ==> store my actual pref/metric in srcentry
 */

int
pim6dd_receive_pim6_assert(src, pim_message, datalen)
	struct sockaddr_in6 *src;
	register char *pim_message;
	int datalen;
{
	mifi_t mifi;
	pim6_encod_uni_addr_t eusaddr;
	pim6_encod_grp_addr_t egaddr;
	struct sockaddr_in6 source, group;
	mrtentry_t *mrtentry_ptr;
	u_int8 *data_ptr;
	struct uvif *v;
	u_int32 assert_preference;
	u_int32 assert_metric;
	u_int32 local_metric;
	u_int32 local_preference;
	u_int8  local_wins;
	if_set new_pruned_oifs, new_leaves;
	int state_change;
    
	if ((mifi = pim6dd_find_vif_direct(src)) == NO_VIF) {
		/* Either a local vif or somehow received PIM_ASSERT from
		 * non-directly connected router. Ignore it.
		 */
		if (pim6dd_local_address(src) == NO_VIF)
			pim6dd_log_msg(LOG_INFO, 0,
			    "Ignoring PIM_ASSERT from non-neighbor router %s",
			    pim6dd_inet6_fmt(&src->sin6_addr));
		return(FALSE);
	}
    
	v = &pim6dd_uvifs[mifi];
	if (pim6dd_uvifs[mifi].uv_flags & (VIFF_DOWN | VIFF_DISABLED | VIFF_NONBRS))
		return(FALSE);    /* Shoudn't come on this interface */
	data_ptr = (u_int8 *)(pim_message + sizeof(struct pim));

	/* Get the group and source addresses */
	GET_EGADDR6(&egaddr, data_ptr);
	GET_EUADDR6(&eusaddr, data_ptr);
    
	/* Get the metric related info */
	GET_HOSTLONG(assert_preference, data_ptr);
	GET_HOSTLONG(assert_metric, data_ptr);

	source.sin6_addr = eusaddr.unicast_addr;
	source.sin6_scope_id = pim6dd_inet6_uvif2scopeid(&source, v);
	group.sin6_addr = egaddr.mcast_addr;
	group.sin6_scope_id = pim6dd_inet6_uvif2scopeid(&group, v);
 
	IF_DEBUG(DEBUG_PIM_ASSERT)
		pim6dd_log_msg(LOG_DEBUG, 0,
		    "PIM Assert received from %s: src %s, grp %s, "
		    "pref %d, metric %d",
		    pim6dd_inet6_fmt(&src->sin6_addr),
		    pim6dd_inet6_fmt(&source.sin6_addr),
		    pim6dd_inet6_fmt(&group.sin6_addr),
		    assert_preference, assert_metric);
 
	if ((mrtentry_ptr = pim6dd_find_route(&source, &group, MRTF_SG, CREATE))
	    == NULL) {
		IF_DEBUG(DEBUG_PIM_ASSERT)
			pim6dd_log_msg(LOG_INFO, 0,
			    "\tFailed to create a mrtentry src:%s grp:%s",
			    pim6dd_inet6_fmt(&source.sin6_addr),
			    pim6dd_inet6_fmt(&group.sin6_addr));
		return(FALSE);
	}
	if(mrtentry_ptr->flags & MRTF_NEW) {
		/* For some reason, it's possible for asserts to be processed
		 * before the data alerts a cache miss.  Therefore, when an
		 * assert is received, create (S,G) state and continue, since
		 * we know by the assert that there are upstream forwarders. 
		 */
		IF_DEBUG(DEBUG_PIM_ASSERT)
			pim6dd_log_msg(LOG_DEBUG, 0, "\tNo MRT entry - creating...");

		mrtentry_ptr->flags &= ~MRTF_NEW;

		/* Set oifs */	
		pim6dd_set_leaves(mrtentry_ptr);
		pim6dd_calc_oifs(mrtentry_ptr, &(mrtentry_ptr->oifs));
	
		/* Add it to the kernel */
		pim6dd_k_chg_mfc(pim6dd_mld6_socket, &source, &group, mrtentry_ptr->incoming,
			  &mrtentry_ptr->oifs);

#ifdef RSRR
		pim6dd_rsrr_cache_send(mrtentry_ptr, RSRR_NOTIFICATION_OK);
#endif /* RSRR */

		/* No need to call pim6dd_change_interfaces, but check for NULL oiflist */
		if(IF_ISEMPTY(&mrtentry_ptr->oifs))
			pim6dd_trigger_prune_alert(mrtentry_ptr);	
	}

	/* If arrived on iif, I'm downstream of the asserted LAN.
	 * If arrived on oif, I'm upstream of the asserted LAN.
	 */
	if (mifi == mrtentry_ptr->incoming) {
		/* assert arrived on iif ==> I'm a downstream router */

		/* Determine local (really that of upstream nbr!) pref/metric */
		local_metric = mrtentry_ptr->metric;
		local_preference = mrtentry_ptr->preference;
 
		if(mrtentry_ptr->upstream &&
		   pim6dd_inet6_equal(&mrtentry_ptr->upstream->address, src) &&
		   assert_preference == local_preference &&
		   assert_metric == local_metric)

			/* if assert from previous winner w/ same pref/metric, 
			 * then assert sender wins again */
			local_wins = FALSE;

		else
			/* assert from someone else or something changed */
			local_wins = pim6dd_compare_metrics(local_preference,
						     local_metric,
						     &mrtentry_ptr->upstream->address,
						     assert_preference,
						     assert_metric, src);

		/*
		 * This is between the assert sender and previous winner or rpf 
		 * (who is the "local" in this case).
		 */
		if(local_wins == TRUE) {
			/* the assert-sender loses, so discard the assert */
			IF_DEBUG(DEBUG_PIM_ASSERT)
				pim6dd_log_msg(LOG_DEBUG, 0,
				    "\tAssert sender %s loses",
				    pim6dd_inet6_fmt(&src->sin6_addr));
			return(TRUE);
		}

		/* The assert sender wins: upstream must be changed to the winner */
		IF_DEBUG(DEBUG_PIM_ASSERT)
			pim6dd_log_msg(LOG_DEBUG, 0, "\tAssert sender %s wins",
			    pim6dd_inet6_fmt(&src->sin6_addr));
		if(pim6dd_inet6_equal(&mrtentry_ptr->upstream->address, src)) {
			IF_DEBUG(DEBUG_PIM_ASSERT)
				pim6dd_log_msg(LOG_DEBUG, 0,
				    "\tChanging upstream nbr to %s",
				    pim6dd_inet6_fmt(&src->sin6_addr));
			mrtentry_ptr->preference = assert_preference;
			mrtentry_ptr->metric = assert_metric;
			mrtentry_ptr->upstream = pim6dd_find_pim6_nbr(src);
		}
		SET_TIMER(mrtentry_ptr->assert_timer, PIM_ASSERT_TIMEOUT);
		mrtentry_ptr->flags |= MRTF_ASSERTED;

		/* Send a join for the S,G if oiflist is non-empty */
		if(!(IF_ISEMPTY(&mrtentry_ptr->oifs))) 
			pim6dd_send_pim6_jp(mrtentry_ptr, PIM_ACTION_JOIN,
				     mrtentry_ptr->incoming, src, 0, 0);

	} /* if assert on iif */

	/* If the assert arrived on an oif: */
	else {
		if(!(IF_ISSET(mifi, &mrtentry_ptr->oifs))) 
			return(FALSE);
		/* assert arrived on oif ==> I'm a upstream router */

		/* Determine local pref/metric */
		local_metric = mrtentry_ptr->source->metric;
		local_preference = mrtentry_ptr->source->preference;

		local_wins = pim6dd_compare_metrics(local_preference, local_metric,
					     &v->uv_linklocal->pa_addr,
					     assert_preference,
					     assert_metric, src);

		if(local_wins == FALSE) {

			/* Assert sender wins - prune the interface */

			IF_DEBUG(DEBUG_PIM_ASSERT)
				pim6dd_log_msg(LOG_DEBUG, 0,
				    "\tAssert sender %s wins - pruning...",
				    pim6dd_inet6_fmt(&src->sin6_addr));

			IF_COPY(&mrtentry_ptr->pruned_oifs, &new_pruned_oifs);
			IF_SET(mifi, &new_pruned_oifs);
			IF_SET(mifi, &mrtentry_ptr->asserted_oifs);
			SET_TIMER(mrtentry_ptr->prune_timers[mifi], 
				  PIM_JOIN_PRUNE_HOLDTIME);

			if (IF_ISSET(mifi, &mrtentry_ptr->leaves)) {
				IF_COPY(&mrtentry_ptr->leaves, &new_leaves);
				IF_CLR(mifi, &new_leaves);
				state_change =
					pim6dd_change_interfaces(mrtentry_ptr,
							  mrtentry_ptr->incoming,
							  &new_pruned_oifs,
							  &mrtentry_ptr->leaves,
							  &new_leaves);
			}
			else {
				state_change =
					pim6dd_change_interfaces(mrtentry_ptr,
							  mrtentry_ptr->incoming,
							  &new_pruned_oifs,
							  &mrtentry_ptr->leaves,
							  &mrtentry_ptr->asserted_oifs);
			}

			/* Handle transition to negative cache */
            #if 1
			if(state_change == -1)
				pim6dd_trigger_prune_alert(mrtentry_ptr);
            #endif

		} /* assert sender wins */

		else {

			/* Local wins (assert sender loses):
			 * send assert and schedule prune
			 */

			IF_DEBUG(DEBUG_PIM_ASSERT)
				pim6dd_log_msg(LOG_DEBUG, 0,
				    "\tAssert sender %s loses - "
				    "sending assert and scheuling prune", 
				    pim6dd_inet6_fmt(&src->sin6_addr));

			if(!(IF_ISSET(mifi, &mrtentry_ptr->leaves))) {
				/*
				 * No directly connected receivers - delay prune
				 */
				pim6dd_send_pim6_jp(mrtentry_ptr, PIM_ACTION_PRUNE,
					     mifi, &v->uv_linklocal->pa_addr,
					     PIM_JOIN_PRUNE_HOLDTIME, 0);
				pim6dd_schedule_delayed_prune(mrtentry_ptr, mifi, 
						       PIM_JOIN_PRUNE_HOLDTIME);
			}
			pim6dd_send_pim6_assert(&source, &group, mifi, mrtentry_ptr);
		}

	} /* if assert on oif */
	
	return(TRUE);
}


int 
pim6dd_send_pim6_assert(source, group, mifi, mrtentry_ptr)
	struct sockaddr_in6 *source;
	struct sockaddr_in6 *group;
	mifi_t mifi;
	mrtentry_t *mrtentry_ptr;
{
	u_int8 *data_ptr;
	u_int8 *data_start_ptr;
	u_int32 local_preference;
	u_int32 local_metric;

	data_ptr = (u_int8 *)(pim6dd_pim6_send_buf + sizeof(struct pim));
	data_start_ptr = data_ptr;
	PUT_EGADDR6(group->sin6_addr, SINGLE_GRP_MSK6LEN, 0, data_ptr);
	PUT_EUADDR6(source->sin6_addr, data_ptr);

	local_metric = mrtentry_ptr->source->metric;
	local_preference = mrtentry_ptr->source->preference;

	PUT_HOSTLONG(local_preference, data_ptr);
	PUT_HOSTLONG(local_metric, data_ptr);

	IF_DEBUG(DEBUG_PIM_ASSERT)
		pim6dd_log_msg(LOG_DEBUG, 0,
		    "PIM Assert sending: src %s, grp %s, "
		    "pref %d, metric %d",
		    pim6dd_inet6_fmt(&source->sin6_addr),
		    pim6dd_inet6_fmt(&group->sin6_addr),
		    local_metric, local_preference);

	pim6dd_send_pim6(pim6dd_pim6_send_buf, &pim6dd_uvifs[mifi].uv_linklocal->pa_addr,
		  &pim6dd_allpim6routers_group, PIM_ASSERT,
		  data_ptr - data_start_ptr);

	return(TRUE);
}


/* Return TRUE if the local win, otherwise FALSE */
static int
pim6dd_compare_metrics(local_preference, local_metric, local_address,
		remote_preference, remote_metric, remote_address)
	u_int32 local_preference;
	u_int32 local_metric;
	struct sockaddr_in6 *local_address;
	u_int32 remote_preference;
	u_int32 remote_metric;
	struct sockaddr_in6 *remote_address;
{
	/* Now lets see who has a smaller gun (aka "asserts war") */
	/* FYI, the smaller gun...err metric wins, but if the same
	 * caliber, then the bigger network address wins. The order of
	 * treatment is: preference, metric, address.
	 */
	/* The RPT bits are already included as the most significant bits
	 * of the preferences.
	 */
	if (remote_preference > local_preference)
		return TRUE;
	if (remote_preference < local_preference)
		return FALSE;
	if (remote_metric > local_metric)
		return TRUE;
	if (remote_metric < local_metric)
		return FALSE;
	if (pim6dd_inet6_greaterthan(local_address, remote_address))
		return TRUE;
	return FALSE;
}




/************************************************************************
 *                        PIM_GRAFT
 ************************************************************************/


static u_long              pim6dd_graft_retrans_timer;   /* Graft retransmission timer */
static pim_graft_entry_t   *pim6dd_graft_list;           /* Active grafting entries */


void
pim6dd_delete_pim6_graft_entry(mrtentry_ptr)
	mrtentry_t *mrtentry_ptr;
{
	pim_graft_entry_t *graft_entry;

	if(mrtentry_ptr->graft == (pim_graft_entry_t *)NULL)
		return;
	graft_entry = mrtentry_ptr->graft;
    
	if(graft_entry->prev)
		graft_entry->prev->next = graft_entry->next;
	else 
		pim6dd_graft_list = graft_entry->next;
	if(graft_entry->next)
		graft_entry->next->prev = graft_entry->prev;
	mrtentry_ptr->graft = (pim_graft_entry_t *)NULL;
	free(graft_entry);

	/* Stop the timer if there are no more entries */
	if(!pim6dd_graft_list) {
		pim6dd_timer_clearTimer(pim6dd_graft_retrans_timer);
		pim6dd_graft_retrans_timer = 0;
	}
}


static int
pim6dd_retransmit_pim6_graft(mrtentry_ptr)
	mrtentry_t *mrtentry_ptr;
{
	u_int8 *data_ptr, *data_start_ptr;
	
	data_ptr = (u_int8 *)(pim6dd_pim6_send_buf + sizeof(struct pim));
	data_start_ptr = data_ptr;

	if (mrtentry_ptr->upstream == (pim_nbr_entry_t *)NULL) {
		/* No upstream neighbor - don't send */
		return(FALSE);
	}

	IF_DEBUG(DEBUG_PIM_GRAFT)
		pim6dd_log_msg(LOG_DEBUG, 0,
		    "Sending GRAFT:  vif %s, src %s, grp %s, dst %s",
		    pim6dd_inet6_fmt(&pim6dd_uvifs[mrtentry_ptr->incoming].uv_linklocal->pa_addr.sin6_addr),
		    pim6dd_inet6_fmt(&mrtentry_ptr->source->address.sin6_addr),
		    pim6dd_inet6_fmt(&mrtentry_ptr->group->group.sin6_addr),
		    pim6dd_inet6_fmt(&mrtentry_ptr->upstream->address.sin6_addr));


	/* unicast target */
	PUT_EUADDR6(mrtentry_ptr->upstream->address.sin6_addr, data_ptr);
	PUT_BYTE(0, data_ptr);		 /* Reserved */
	*data_ptr++ = (u_int8)1;             /* number of groups */
	PUT_HOSTSHORT(0, data_ptr);          /* no holdtime */

	/* data_ptr points at the first, and only encoded mcast group */
	PUT_EGADDR6(mrtentry_ptr->group->group.sin6_addr,
		    SINGLE_GRP_MSK6LEN, 0, data_ptr);

	/* set the number of join(graft) and prune sources */
	PUT_HOSTSHORT(1, data_ptr);
	PUT_HOSTSHORT(0, data_ptr);

	PUT_ESADDR6(mrtentry_ptr->source->address.sin6_addr, SINGLE_SRC_MSK6LEN,
		    0, data_ptr);

	pim6dd_send_pim6(pim6dd_pim6_send_buf,
		  &pim6dd_uvifs[mrtentry_ptr->incoming].uv_linklocal->pa_addr,
		  &mrtentry_ptr->upstream->address,
		  PIM_GRAFT, data_ptr - data_start_ptr);

	return(TRUE);
}


static void
pim6dd_retransmit_all_pim6_grafts(arg)
	void *arg; /* UNUSED */
{
	pim_graft_entry_t *graft_ptr;

	IF_DEBUG(DEBUG_PIM_GRAFT)
		pim6dd_log_msg(LOG_DEBUG, 0, "Retransmitting all pending PIM-Grafts");
  

	for(graft_ptr = pim6dd_graft_list; 
	    graft_ptr != NULL; 
	    graft_ptr = graft_ptr->next) {

		IF_DEBUG(DEBUG_PIM_GRAFT)
			pim6dd_log_msg(LOG_DEBUG, 0, "\tGRAFT src %s, grp %s",
			    pim6dd_inet6_fmt(&graft_ptr->mrtlink->source->address.sin6_addr),
			    pim6dd_inet6_fmt(&graft_ptr->mrtlink->group->group.sin6_addr));

		pim6dd_retransmit_pim6_graft(graft_ptr->mrtlink);
	}

	if (pim6dd_graft_list)
		pim6dd_timer_setTimer(PIM_GRAFT_RETRANS_PERIOD,
			       pim6dd_retransmit_all_pim6_grafts, (void *)NULL);
}


int
pim6dd_receive_pim6_graft(src, pim_message, datalen, pimtype)
	struct sockaddr_in6 *src;
	register char *pim_message;
	int datalen;
	int pimtype;
{
	mifi_t mifi;
	struct uvif *v;
	pim6_encod_uni_addr_t uni_target_addr;
	pim6_encod_grp_addr_t encod_group;
	pim6_encod_src_addr_t encod_src;
	u_int8 *data_ptr;
	u_int8 num_groups;
	u_int16 holdtime;
	u_int16 num_j_srcs;
	u_int16 num_p_srcs;
	struct sockaddr_in6 source, group;
	struct in6_addr s_mask, g_mask;
	u_int8 s_flags;
	u_int8 reserved;
	mrtentry_t *mrtentry_ptr;
	int state_change;

	if ((mifi = pim6dd_find_vif_direct(src)) == NO_VIF) {
		/* Either a local vif or somehow received PIM_GRAFT from
		 * non-directly connected router. Ignore it.
		 */
		if (pim6dd_local_address(src) == NO_VIF)
			pim6dd_log_msg(LOG_INFO, 0,
			    "Ignoring PIM_GRAFT from non-neighbor router %s",
			    pim6dd_inet6_fmt(&src->sin6_addr));
		return(FALSE);
	}

	v = &pim6dd_uvifs[mifi];
	if (pim6dd_uvifs[mifi].uv_flags & (VIFF_DOWN | VIFF_DISABLED | VIFF_NONBRS))
		return(FALSE);    /* Shoudn't come on this interface */
	data_ptr = (u_int8 *)(pim_message + sizeof(struct pim));

	/* Get the target address */
	GET_EUADDR6(&uni_target_addr, data_ptr);
	GET_BYTE(reserved, data_ptr);
	GET_BYTE(num_groups, data_ptr);
	if (num_groups == 0)
		return (FALSE);    /* No indication for groups in the message */
	GET_HOSTSHORT(holdtime, data_ptr);

	IF_DEBUG(DEBUG_PIM_GRAFT)
		pim6dd_log_msg(LOG_DEBUG, 0,
		    "PIM %s received from %s on mif %d, grps: %d",
		    pimtype == PIM_GRAFT ? "GRAFT" : "GRAFT-ACK",
		    pim6dd_inet6_fmt(&src->sin6_addr), mifi, num_groups);
    
	group.sin6_len = sizeof(group);
	group.sin6_family = AF_INET6;
	source.sin6_len = sizeof(source);
	source.sin6_family = AF_INET6;
	while (num_groups--) {
		GET_EGADDR6(&encod_group, data_ptr);
		GET_HOSTSHORT(num_j_srcs, data_ptr);
		GET_HOSTSHORT(num_p_srcs, data_ptr);
		IF_DEBUG(DEBUG_PIM_GRAFT)
			pim6dd_log_msg(LOG_DEBUG, 0,
			    "  PIM graft: grp: %s, plen: %d, %d jsrcs, %d psrcs",
			    pim6dd_inet6_fmt(&encod_group.mcast_addr),
			    encod_group.masklen, num_j_srcs, num_p_srcs);
		if (encod_group.masklen > (sizeof(struct in6_addr) << 3))
			continue;
		MASKLEN_TO_MASK6(encod_group.masklen, g_mask);
		group.sin6_addr = encod_group.mcast_addr;
		group.sin6_scope_id = pim6dd_inet6_uvif2scopeid(&group, v);
		if (!IN6_IS_ADDR_MULTICAST(&group.sin6_addr)) {
			data_ptr +=
				(num_j_srcs + num_p_srcs) * sizeof(pim6_encod_src_addr_t);
			continue; /* Ignore this group and jump to the next */
		}
	
		while (num_j_srcs--) {
			GET_ESADDR6(&encod_src, data_ptr);
			if (encod_src.masklen > (sizeof(struct in6_addr) << 3))
				continue;
			source.sin6_addr = encod_src.src_addr;
			source.sin6_scope_id = pim6dd_inet6_uvif2scopeid(&source, v);
			if (!pim6dd_inet6_valid_host(&source))
				continue;
			s_flags = encod_src.flags;
			MASKLEN_TO_MASK6(encod_src.masklen, s_mask);

			mrtentry_ptr = pim6dd_find_route(&source, &group, MRTF_SG,
						  DONT_CREATE);
			if(mrtentry_ptr == (mrtentry_t *)NULL) 
				continue;

			if(pimtype == PIM_GRAFT) {
				/* Graft */
				IF_DEBUG(DEBUG_PIM_GRAFT)
					pim6dd_log_msg(LOG_DEBUG, 0,
					    "\tGRAFT src %s, group %s - "
					    "forward data on mif %d",
					    pim6dd_inet6_fmt(&source.sin6_addr),
					    pim6dd_inet6_fmt(&group.sin6_addr),
					    mifi);

				/* Cancel any delayed prune */
				if(mrtentry_ptr->prune_delay_timerids[mifi]) {
					pim6dd_timer_clearTimer(mrtentry_ptr->prune_delay_timerids[mifi]);
					mrtentry_ptr->prune_delay_timerids[mifi] = 0;
				}	    
		
				/* Add to oiflist (unprune) */
				if (IF_ISSET(mifi, &mrtentry_ptr->pruned_oifs)) {
					IF_CLR(mifi, &mrtentry_ptr->pruned_oifs);
					IF_CLR(mifi, &mrtentry_ptr->asserted_oifs);
					SET_TIMER(mrtentry_ptr->prune_timers[mifi], 0);
					state_change = 
						pim6dd_change_interfaces(mrtentry_ptr,
								  mrtentry_ptr->incoming,
								  &mrtentry_ptr->pruned_oifs,
								  &mrtentry_ptr->leaves,
								  &mrtentry_ptr->asserted_oifs);
                    #if 1
					if(state_change == 1)
						pim6dd_trigger_join_alert(mrtentry_ptr);
                    #endif
				}
			}  /* Graft */
			else {
				/* Graft-Ack */
				IF_DEBUG(DEBUG_PIM_GRAFT)
					pim6dd_log_msg(LOG_DEBUG, 0,
					    "\tGRAFT-ACK src %s, group %s - "
					    "forward data on mif %d",
					    pim6dd_inet6_fmt(&source.sin6_addr),
					    pim6dd_inet6_fmt(&group.sin6_addr),
					    mifi);
				if(mrtentry_ptr->graft)
					pim6dd_delete_pim6_graft_entry(mrtentry_ptr);
			}
		}
		/* Ignore anything in the prune portion of the message! */
	}

	/* Respond to graft with a graft-ack */
	if(pimtype == PIM_GRAFT) {
		IF_DEBUG(DEBUG_PIM_GRAFT)
			pim6dd_log_msg(LOG_DEBUG, 0, "Sending GRAFT-ACK: mif %s, dst %s",
			    pim6dd_inet6_fmt(&pim6dd_uvifs[mifi].uv_linklocal->pa_addr.sin6_addr),
			    pim6dd_inet6_fmt(&src->sin6_addr));
		bcopy(pim_message, pim6dd_pim6_send_buf, datalen);
		pim6dd_send_pim6(pim6dd_pim6_send_buf, &pim6dd_uvifs[mifi].uv_linklocal->pa_addr,
			  src, PIM_GRAFT_ACK, datalen - sizeof(struct pim));
	}

	return(TRUE);
}

int 
pim6dd_send_pim6_graft(mrtentry_ptr)
	mrtentry_t *mrtentry_ptr;
{
	pim_graft_entry_t *new_graft;
	int was_sent = 0;

	if(mrtentry_ptr->graft != (pim_graft_entry_t *)NULL)
		/* Already sending grafts */
		return(FALSE);

	/* Send the first graft */
	was_sent = pim6dd_retransmit_pim6_graft(mrtentry_ptr);
	if(!was_sent)
		return(FALSE);

	/* Set up retransmission */
	new_graft = (pim_graft_entry_t *)malloc(sizeof(pim_graft_entry_t));
	if (new_graft == (pim_graft_entry_t *)NULL) {
		pim6dd_log_msg(LOG_WARNING, 0, 
		    "Memory allocation error for graft entry src %s, grp %s",
		    pim6dd_inet6_fmt(&mrtentry_ptr->source->address.sin6_addr),
		    pim6dd_inet6_fmt(&mrtentry_ptr->group->group.sin6_addr));
		return(FALSE);
	}
	new_graft->next = pim6dd_graft_list;
	new_graft->prev = (pim_graft_entry_t *)NULL;
	new_graft->mrtlink = mrtentry_ptr;
	if(pim6dd_graft_list)
		pim6dd_graft_list->prev = new_graft;
	pim6dd_graft_list = new_graft;
	mrtentry_ptr->graft = new_graft;

	/* Set up timer if not running */
	if(!pim6dd_graft_retrans_timer) 
		pim6dd_graft_retrans_timer = pim6dd_timer_setTimer(PIM_GRAFT_RETRANS_PERIOD,
						     pim6dd_retransmit_all_pim6_grafts, 
						     (void *)NULL);

	return(TRUE);
}
