/*
 *      Handling routines for Mesh in 802.11 RX
 * 
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_RX_C_

#ifdef CONFIG_RTL8192CD
#include "../rtl8192cd/8192cd.h"
#include "../rtl8192cd/8192cd_headers.h"
#else
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#endif
#include "./mesh.h"
#include "./mesh_route.h"

#ifdef CONFIG_RTK_MESH

/*
	Check the "to_fr_ds" field:

						FromDS =1
						ToDS =1
	NOTE: The functuion duplicate from rtl8190_rx_dispatch_wds (8190n_rx.c)
*/
int rx_dispatch_mesh(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
	int reuse = 1;
	unsigned char *pframe = get_pframe(pfrinfo);
	unsigned int frtype = GetFrameType(pframe);
	unsigned short frame_type = GetFrameSubType(pframe);
	
	struct net_device *dev = priv->mesh_dev;
	unsigned char	*da = pfrinfo->da;
	int fragnum;
  
	if (memcmp(GET_MY_HWADDR, GetAddr1Ptr(pframe), MACADDRLEN)
		&& !IS_MCAST(GetAddr1Ptr(pframe))	)
	{
		DEBUG_INFO("Rx a 11s packet but which addr1 is not matched own, drop it!\n");
		goto out;
	}

	if (!netif_running(dev)) {
		DEBUG_ERR("Rx a 11s packet but which interface is not up, drop it!\n");
		goto out;
	}
	
	fragnum = GetFragNum(pframe);
	if (GetMFrag(pframe) || fragnum) {
		DEBUG_ERR("Rx a fragment 11s packet, drop it!\n");
		goto out;
	}

	if ( (frtype != WIFI_DATA_TYPE) && (frtype != WIFI_EXT_TYPE)) {
		DEBUG_ERR("Rx a Mesh packet but which type is not valid, drop it!\n");
		goto out;
	}
	
	// Unsupported (8186): 802.11s action frame, data frame --> type = 11 (i.e., WIFI_EXT_TYPE)
	// Hence, we had to use WIFI_DATA_TYPE in 8186, 8187B and now 8190.
	if (frtype == WIFI_DATA_TYPE) {	
		struct stat_info	*pstat;
        #ifndef SMP_SYNC
		unsigned long		flags;
        #endif               
		SAVE_INT_AND_CLI(flags);
		pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
		
		// action which has lowest state is "Link state announcement"
		if((pstat == NULL) 
				|| !((pstat->mesh_neighbor_TBL.State == MP_SUPERORDINATE_LINK_UP) || (pstat->mesh_neighbor_TBL.State == MP_SUBORDINATE_LINK_UP)
				|| (pstat->mesh_neighbor_TBL.State == MP_SUBORDINATE_LINK_DOWN_E))) // Recived Local Link State Announcement from  DOWN to UP
		{
		
			RESTORE_INT(flags);
			if( !IS_MCAST(da) && frame_type == WIFI_11S_MESH && pstat == NULL)
				issue_deauth_MP(priv, (void *)GetAddr2Ptr(pframe), _RSON_UNSPECIFIED_, 1);
//
			DEBUG_ERR("Rx a Mesh packet but there does not have a corresponding station info\n");
			goto out;
		}
		RESTORE_INT(flags);
		pfrinfo->is_11s = 1;
		
#ifndef MESH_BOOTSEQ_STRESS_TEST	// Stop update expire when stress test.
		// chris: update neighbor timer
		pstat->mesh_neighbor_TBL.expire = jiffies + MESH_EXPIRE_TO;
#endif
	} 
					
	// if( frame_type == WIFI_ACTION ) { // In D1.06 next line replace by this line (And remove SetFrameSubType(pframe, WIFI_ACTION);) by popen
	if( frame_type == WIFI_11S_MESH_ACTION ) 
	{
		int TTL;
		unsigned short seq;
		//struct path_sel_entry *pPathselEntry;
						
		seq = *((unsigned short*)(pframe+32));
		TTL = *GetMeshHeaderTTLWithOutQOS(pframe);
					
		if(TTL == 0)	// filter packets due to duplicate
			goto out;
						
		*GetMeshHeaderTTLWithOutQOS(pframe) = TTL-1;				
		SetFrameSubType(pframe, WIFI_WMM_ACTION); // it's easier for mgt_handler

		// IF Draft modify Action frame to Date frame, Remove line between here to goto out ?
#ifdef RTL8190_DIRECT_RX
		DRV_RX_MGNTFRAME(priv, NULL, pfrinfo);
#else
		list_add_tail(&(pfrinfo->rx_list), &(priv->rx_mgtlist));
#endif
		reuse = 0;
		goto	out;	// Success finish, reuse = 0 (Because not Date frame) (Remove to here)
	}

	else if( frame_type == WIFI_11S_MESH ) 
	{
	
		memcpy(pfrinfo->prehop_11s, GetAddr2Ptr(pframe), MACADDRLEN);
	} 
	else { 
		DEBUG_ERR("Rx a 11s packet but which type is not DATA, drop it!\n");
		goto out;
	} 

#ifdef RTL8190_DIRECT_RX
	DRV_RX_DATA(priv, NULL, pfrinfo);
#else
	list_add_tail(&pfrinfo->rx_list, &priv->rx_datalist);
#endif
	reuse = 0;
out:
	return reuse;
}



#ifdef RX_RL_SHORTCUT
int mesh_shortcut_update(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, 
             struct stat_info *pstat, int idx, struct path_sel_entry *pEntry, unsigned char *to_mesh, struct MESH_HDR **meshHdrPt)
{
#ifdef SMP_SYNC
    unsigned long flags;
#endif
    unsigned char *pframe = get_pframe(pfrinfo);
    struct path_sel_entry *pPathselEntry;
    struct proxy_table_entry *pProxyEntry;
    struct MESH_HDR * mesh_header;            
    DRV_PRIV * orig_priv = priv;

    *to_mesh = 0;
    if (memcmp(GetAddr4Ptr(pframe), pstat->rx_sc_ent[idx].rx_wlanhdr.addr4, MACADDRLEN)){
        return -1;
    }

    priv = (DRV_PRIV *)priv->mesh_priv_first;
    if (memcmp(GetAddr3Ptr(pframe), GET_MY_HWADDR, MACADDRLEN)) {

        SMP_LOCK_MESH_PATH(flags);
        pPathselEntry = HASH_SEARCH(priv->pathsel_table, GetAddr3Ptr(pframe));
        if (pPathselEntry) {	            
            memcpy(pEntry, pPathselEntry, sizeof(struct path_sel_entry));
            *to_mesh = 1;
        }
        SMP_UNLOCK_MESH_PATH(flags);
        
        if(pPathselEntry == NULL)
            return -1;	
    }

    mesh_header = *meshHdrPt = (struct MESH_HDR *)getMeshHeader(orig_priv, orig_priv->pmib->dot11sKeysTable.dot11Privacy, pframe); 
    if( mesh_header ) 
    {
        if(1 > mesh_header->TTL || mesh_header->mesh_flag != 0x01) 
            return -1;

        if (memcmp(mesh_header->DestMACAddr, pstat->rx_sc_ent[idx].rx_wlanhdr.meshhdr.DestMACAddr, 6) ||
            memcmp(mesh_header->SrcMACAddr, pstat->rx_sc_ent[idx].rx_wlanhdr.meshhdr.SrcMACAddr, 6)) {
            return -1;
        }

    } 
    else {
        return -1;
    }
	
	
    //6 addresss format, update pathsel and proxy timer 
    SMP_LOCK_MESH_PATH(flags);
    pPathselEntry = HASH_SEARCH(priv->pathsel_table, pfrinfo->sa); 
    if (pPathselEntry != NULL)
        pPathselEntry->update_time = jiffies;
    SMP_UNLOCK_MESH_PATH(flags);

    SMP_LOCK_MESH_PROXY(flags);
    pProxyEntry = (struct proxy_table_entry*) HASH_SEARCH(priv->proxy_table, mesh_header->SrcMACAddr);
    if (pProxyEntry != NULL)
        pProxyEntry->aging_time = 0;

    pProxyEntry = (struct proxy_table_entry*) HASH_SEARCH(priv->proxy_table, mesh_header->DestMACAddr); 
    if (pProxyEntry != NULL && pProxyEntry->aging_time >= 0) 
        pProxyEntry->aging_time = 0;
    SMP_UNLOCK_MESH_PROXY(flags);


    return 0;
}

#endif

/*
	pfrinfo->is_11s =1  => 802.11 header
	pfrinfo->is_11s =8  => 802.3  header + mesh header
*/
int relay_11s_dataframe_mcast(DRV_PRIV *priv, struct sk_buff *skb, int privacy, struct rx_frinfo *pfrinfo)
{	
    struct list_head *phead, *plist;
    struct stat_info *pstat;
    struct sk_buff *pnewskb = NULL, *pnewskb_send = NULL;
    unsigned char prehop[MACADDRLEN], nexthop[MACADDRLEN];
    unsigned char force_m2u = 0, sta_count = 0;
    struct path_sel_entry* pPathEntry;    
    struct tx_insn tx_insn;
    DRV_PRIV *xmit_priv = NULL;    
    UINT16 seqNum;  
    int i,j,table_num;


    unsigned long flags = 0;

    memcpy(prehop, GetAddr2Ptr(skb->data), MACADDRLEN);
    pnewskb = skb_copy(skb, GFP_ATOMIC);
    if(skb_p80211_to_ether(skb->dev, privacy, pfrinfo) == FAIL){ // for e.g., CISCO CDP which has unsupported LLC's vendor ID
        DEBUG_ERR("RX DROP: skb_p80211_to_ether fail!\n");
        return -1; 
    } 

    if(1 > pfrinfo->mesh_header.TTL) {        
        return 0;
    }

    if (pnewskb) {  
        get_pskb(pfrinfo) = pnewskb;
        if(pfrinfo->is_11s&1) 
        {
            pfrinfo->is_11s = 2;/*replace wlan_header with eth_header, but keep the mesh header*/
            if(skb_p80211_to_ether(pnewskb->dev, privacy, pfrinfo) == FAIL){ // for e.g., CISCO CDP which has unsupported LLC's vendor ID        
                pfrinfo->is_11s = 1;
                goto relay_end;
            }
            pfrinfo->is_11s = 1;
        }
    }
    else {           
        goto relay_end;
    }         

               
    if (IP_MCAST_MAC(skb->data)
#ifdef  TX_SUPPORT_IPV6_MCAST2UNI
        || IPV6_MCAST_MAC(skb->data)
#endif
    )
    {
        // all multicast managment packet try do m2u
        if( isSpecialFloodMac(priv,skb) || IS_MDNSV4_MAC(skb->data)||IS_MDNSV6_MAC(skb->data)||IS_IGMP_PROTO(skb->data) || isICMPv6Mng(skb) || IS_ICMPV6_PROTO(skb->data)|| isMDNS(skb->data))
        {
            force_m2u = 1;
        }    
    }
    else {
        force_m2u = 1;
    }     


    sta_count = 0;
    if(force_m2u == 0) {/* if it is not a multicast management or broadcast frame, check if it is a video  frame need to be multicasted*/   
        /*forward to every mesh nodes*/
        seqNum = getMeshMulticastSeq(priv);
        table_num = 1 << priv->pathsel_table->table_size_power;
        for (i = 0; i < table_num; i++) {

            xmit_priv = NULL;
            SMP_LOCK_MESH_PATH(flags);                
            if (priv->pathsel_table->entry_array[i].dirty) 
            { 
                pPathEntry = ((struct path_sel_entry*)priv->pathsel_table->entry_array[i].data);      
                for (j=0; j<MAX_IP_MC_ENTRY; j++) {
                    if (pPathEntry->ipmc[j].used && !memcmp(&pPathEntry->ipmc[j].mcmac[0], skb->data, MACADDRLEN)) {
                        if(memcmp(pPathEntry->destMAC, pfrinfo->sa, MACADDRLEN) == 0 ||
                           memcmp(pPathEntry->nexthopMAC, prehop, MACADDRLEN) == 0 || 
                           memcmp(pPathEntry->nexthopMAC, pfrinfo->sa, MACADDRLEN) == 0) {
                            break;
                        } 

                        memcpy(nexthop, pPathEntry->nexthopMAC, MACADDRLEN);
                        xmit_priv = pPathEntry->priv;
                    }
                }
            }
            SMP_UNLOCK_MESH_PATH(flags);

            if(xmit_priv) {
                pstat = get_stainfo(xmit_priv, nexthop);
                if(pstat && pstat->mesh_neighbor_TBL.seqNum != seqNum) {
                    pstat->mesh_neighbor_TBL.seqNum = seqNum;
                    
                    pnewskb_send = skb_copy(pnewskb, GFP_ATOMIC);
                    if(pnewskb_send == NULL) {
                        goto relay_end;
                    }       
                    sta_count++;
                    tx_insn.is_11s = RELAY_11S;
                    tx_insn.priv = xmit_priv;  
                    pnewskb_send->dev = xmit_priv->mesh_dev;


                    priv = xmit_priv;
                    SAVE_INT_AND_CLI(flags);
                    SMP_LOCK_XMIT(flags);
                    __rtl8192cd_start_xmit_out(pnewskb_send, pstat, &tx_insn);
                    RESTORE_INT(flags);
                    SMP_UNLOCK_XMIT(flags);
                    priv = priv->mesh_priv_first;
                }    
            }
        }
    }
    
    if(force_m2u == 1 || (sta_count == 0 && !priv->pshare->rf_ft_var.mc2u_drop_unknown_mesh)) 
    {
        /*forward to every mesh nodes*/
        seqNum = getMeshMulticastSeq(priv);
        table_num = 1 << priv->pathsel_table->table_size_power;
        for (i = 0; i < table_num; i++) {

            xmit_priv = NULL;
            SMP_LOCK_MESH_PATH(flags);                
            if (priv->pathsel_table->entry_array[i].dirty) 
            { 
                pPathEntry = ((struct path_sel_entry*)priv->pathsel_table->entry_array[i].data);

                if(memcmp(pPathEntry->destMAC, pfrinfo->sa, MACADDRLEN) &&
                   memcmp(pPathEntry->nexthopMAC, prehop, MACADDRLEN) &&
                   memcmp(pPathEntry->nexthopMAC, pfrinfo->sa, MACADDRLEN)) {
                    xmit_priv = pPathEntry->priv;
                    memcpy(nexthop, pPathEntry->nexthopMAC, MACADDRLEN);  
                }                                                                                                                                    
            }
            SMP_UNLOCK_MESH_PATH(flags);

            if(xmit_priv) {
                pstat = get_stainfo(xmit_priv, nexthop);
                if(pstat && pstat->mesh_neighbor_TBL.seqNum != seqNum) {
                    pstat->mesh_neighbor_TBL.seqNum = seqNum;

                    pnewskb_send = skb_copy(pnewskb, GFP_ATOMIC);
                    if(pnewskb_send == NULL) {
                        goto relay_end;
                    }       
                    
                    pnewskb_send->cb[2] = (char)0xff; // do not aggregation
                    tx_insn.is_11s = RELAY_11S;
                    tx_insn.priv = xmit_priv;  
                    pnewskb_send->dev = priv->mesh_dev;


                    priv = xmit_priv;
                    SAVE_INT_AND_CLI(flags);
                    SMP_LOCK_XMIT(flags);
                    __rtl8192cd_start_xmit_out(pnewskb_send, pstat, &tx_insn);
                    RESTORE_INT(flags);
                    SMP_UNLOCK_XMIT(flags);
                    priv = priv->mesh_priv_first;
                }    
            }
        }
    }
         
relay_end:
    if(pnewskb) {
        dev_kfree_skb_any(pnewskb);
    }
    /*roll back to original value*/
    get_pskb(pfrinfo) = skb;
    return 0;

}

/*
	pfrinfo->is_11s =1  => 802.11 header
	pfrinfo->is_11s =8  => 802.3  header + mesh header
*/
int relay_11s_dataframe_ucast(DRV_PRIV *priv, struct sk_buff *skb, int privacy, struct rx_frinfo *pfrinfo)
{
    unsigned long flags = 0;
    struct path_sel_entry *pEntry;	    
    DRV_PRIV *xmit_priv = NULL;
    unsigned char nexthop[MACADDRLEN];
    DOT11s_GEN_RERR_PACKET rerr_event;			
    struct stat_info *pstat;
    struct tx_insn tx_insn;

    if(1 > pfrinfo->mesh_header.TTL) {        
        return -1;
    }

   
    SMP_LOCK_MESH_PATH(flags);
    pEntry = HASH_SEARCH(priv->pathsel_table, pfrinfo->da );		
    if(pEntry) {
        xmit_priv = pEntry->priv;
        memcpy(nexthop, pEntry->nexthopMAC, MACADDRLEN);
    }
    SMP_UNLOCK_MESH_PATH(flags);

    if(pEntry == NULL) // not have valid route path
    {
        memset((void*)&rerr_event, 0x0, sizeof(DOT11s_GEN_RERR_PACKET));
        rerr_event.EventId = DOT11_EVENT_PATHSEL_GEN_RERR;
        rerr_event.IsMoreEvent = 0;
        memcpy(rerr_event.MyMACAddr,  GET_MY_HWADDR ,MACADDRLEN); 
        memcpy(rerr_event.SorNMACAddr,  pfrinfo->da ,MACADDRLEN);
        memcpy(rerr_event.DataDestMAC,  pfrinfo->sa ,MACADDRLEN);

        // this field will be used by Path Selection daemon for the following case 
        // when MP want to generate RERR to data source but no path to the data source now, it will send the RERR which set Addr2 = Prehop.

        memcpy(rerr_event.PrehopMAC, pfrinfo->prehop_11s, MACADDRLEN);

        rerr_event.TTL = _MESH_HEADER_TTL_;
        rerr_event.Seq_num = getMeshSeq(priv);
        rerr_event.Flag = 2;// flag = 2 means this MP doesn't have the nexthop information for the destination in pathseleciton table
        DOT11_EnQueue2((unsigned long)priv, priv->pathsel_queue, (unsigned char*)&rerr_event, sizeof(DOT11s_GEN_RERR_PACKET));
        notifyPathSelection(priv);
        return -1;

    } // if(pEntry == (struct path_sel_entry *)-1)
    else {
        if(pfrinfo->is_11s&1) 
        {
            pfrinfo->is_11s = 2;
            if(skb_p80211_to_ether(skb->dev, privacy, pfrinfo) == FAIL) { // for e.g., CISCO CDP which has unsupported LLC's vendor ID
                return -1;
            }
        }

        pstat = get_stainfo(xmit_priv, nexthop);
        tx_insn.is_11s = RELAY_11S;
        tx_insn.priv = xmit_priv;  
        skb->dev = priv->mesh_dev;

        priv = xmit_priv;
        SAVE_INT_AND_CLI(flags);
        SMP_LOCK_XMIT(flags);
        __rtl8192cd_start_xmit_out(skb, pstat, &tx_insn);
        RESTORE_INT(flags);
        SMP_UNLOCK_XMIT(flags);
        priv = priv->mesh_priv_first;         
    }

    return 0;
}

/*
	pfrinfo->is_11s =1  => 802.11 header
	pfrinfo->is_11s =8  => 802.3  header + mesh header
*/
int process_11s_datafrme(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, struct stat_info *pstat)
{
    unsigned char 	*pframe;
    struct MESH_HDR *meshHdrPtr;
    pframe = get_pframe(pfrinfo);

    if((*GetQosControl(pframe))& BIT(7)) {// Process A-MSDU
        pfrinfo->pskb->dev = priv->mesh_dev;
        if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
              priv->pmib->reorderCtrlEntry.ReorderCtrlEnable &&
              !IS_MCAST(GetAddr1Ptr(pframe))) {
            *(unsigned int *)&(pfrinfo->pskb->cb[4]) = 0;
            (pfrinfo->pskb->cb[3]) = 1;
            if (reorder_ctrl_check(priv, pstat, pfrinfo)) {
                process_amsdu(priv, pstat, pfrinfo);
            }
        }
        else
            process_amsdu(priv, pstat, pfrinfo);
        return SUCCESS;
    } else 	{		    
        meshHdrPtr = (struct MESH_HDR *)getMeshHeader(priv, priv->pmib->dot11sKeysTable.dot11Privacy, pframe); 	
        return __process_11s_datafrme(priv, pfrinfo, pstat, meshHdrPtr);		
    }
}


int __process_11s_datafrme(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, struct stat_info *pstat, struct MESH_HDR *meshHdrPtr)
{    
#ifdef SMP_SYNC
    unsigned long flags = 0;
#endif
    unsigned char *addr5, *addr6;
    unsigned int  	privacy;
    DRV_PRIV * orig_priv = priv;
    unsigned char 	*pframe;
    struct sk_buff	 *pskb  = NULL;
    struct path_sel_entry *pPathselEntry;
    struct stat_info *dapstat = NULL;    
    int do_process;
    int ret;

    
    pframe = get_pframe(pfrinfo);
    pskb = get_pskb(pfrinfo);    
    priv = (DRV_PRIV *)priv->mesh_priv_first;


    if( meshHdrPtr ) 
    {
        /*strictly check mesh header to  filter packets due to duplicate, 
                        or due to 92C,92D, 88C HW bug which may recieved garbage packets when using CCMP encryption*/
        if(meshHdrPtr->TTL < 1 || _MESH_HEADER_TTL_ < meshHdrPtr->TTL || meshHdrPtr->mesh_flag != 0x01 || meshHdrPtr->SrcMACAddr[0] & 0x01)
            return FAIL;
            
        meshHdrPtr->TTL--;
    }       
    else
        return FAIL;
    
    SMP_LOCK_MESH_PATH(flags);
    
    pPathselEntry = HASH_SEARCH(priv->pathsel_table, pfrinfo->sa);
    if(pPathselEntry) {
    
        // Usually, IS_MCAST(A3) = IS_MCAST(A1) for a multicast frame
        // However, when using unicast to simulate multicast, A1 is unicast but A3 is not 
        // On the other hand, if a maliciuos host issues multicast for A1 but unicast for A3, a flooding would happen if not chcking A1
        if(IS_MCAST(pfrinfo->da) ){ // filter packets due to duplicate                                       
            if(!chkMeshSeq(pPathselEntry, pfrinfo->sa, meshHdrPtr->segNum)) {  
                SMP_UNLOCK_MESH_PATH(flags);
                return FAIL;
            }            
        }     
        pPathselEntry->update_time = jiffies;
    }
    else {/*drop loop multicast packet*/
        SMP_UNLOCK_MESH_PATH(flags);
        return FAIL;
    }
    SMP_UNLOCK_MESH_PATH(flags);
    
    memcpy(&(pfrinfo->mesh_header), meshHdrPtr, sizeof(struct lls_mesh_header) );



    /*update proxy table*/
    addr5 = pfrinfo->mesh_header.DestMACAddr;
    addr6 = pfrinfo->mesh_header.SrcMACAddr;

    //if frame is 6 addresss format, update proxy table 
    if(memcmp(pfrinfo->sa, addr6, MACADDRLEN)) { // addr4 <> addr 6, save it to proxy table
        mesh_proxy_update(priv, pfrinfo->sa, addr6);
    }

    privacy = get_sta_encrypt_algthm(orig_priv, pstat);
    if(IS_MCAST(pfrinfo->da))
    {  

       if (!orig_priv->pmib->dot11OperationEntry.block_relay)
        {
            ret = relay_11s_dataframe_mcast(priv,pskb, /*pskb->dev,*/ privacy, pfrinfo);
            if(ret < 0) {
                return FAIL;
            }
            else {
                goto to_bridge;
            }            
        }
        goto receive_packet;       
    } 
    else			
    {

        if( memcmp(pfrinfo->da, GET_MY_HWADDR, MACADDRLEN)== 0)
        {     
            // filter packets that SA is myself 
            if ( memcmp(pfrinfo->sa, GET_MY_HWADDR, MACADDRLEN) == 0)
                return FAIL;

#ifdef PU_STANDARD_RX
            if( Is_6AddrFormat && memcmp(GET_MY_HWADDR, addr5, MACADDRLEN) )
            {
                struct proxyupdate_table_entry Entry;
                struct proxy_table_entry *pEntry=NULL;
                struct path_sel_entry *pdstEntry=NULL;

                //check if addr5 is my associating STA
                if(get_stainfo(priv, addr5)!=NULL)
                    goto receive_packet;

                // check if addr5 is any other AP's STA
                SMP_LOCK_MESH_PROXY(flags);
                pEntry = HASH_SEARCH(priv->proxy_table, addr5);
                if( pEntry )
                {
                    pEntry->aging_time = 0;
                    // case: my bridged PC
                    //     da=me, STA = NULL, proxy != NULL 
                    if(memcmp(GET_MY_HWADDR, pEntry->owner, MACADDRLEN)==0) {
                        SMP_UNLOCK_MESH_PROXY(flags);
                        goto receive_packet;
                    }

                    // case: I know the proxied node resided in (but not behind me)
                    //     da=me, STA =NULL, proxy->owner=other node -> proxy update:add
                    else 
                    {
                        Entry.PUflag = PU_add;
                        memcpy((void *)Entry.proxymac, pEntry->owner, MACADDRLEN);
                    }
                }
                else					
                {
                    // case: unknown entry
                    //  da=me, STA=NULL, proxy = NULL
                    //  it might be an un-recorded bridged PC happens for pEntry = NULL
                    Entry.PUflag = PU_delete;
                    memcpy((void *)Entry.proxymac, GET_MY_HWADDR, MACADDRLEN);
                }                    
                SMP_UNLOCK_MESH_PROXY(flags);

                SMP_LOCK_MESH_PATH(flags);
                pdstEntry = HASH_SEARCH(priv->pathsel_table, pfrinfo->sa);
                if( pdstEntry){
                    Entry.isMultihop = pdstEntry->hopcount ;
                    memcpy(Entry.nexthopmac ,pdstEntry->nexthopMAC ,MACADDRLEN);
                    SMP_UNLOCK_MESH_PATH(flags);
                }
                else{
                    SMP_UNLOCK_MESH_PATH(flags);
                    memset((void *)Entry.nexthopmac ,0xff ,MACADDRLEN);
                    Entry.isMultihop =0;
                }
                memcpy(Entry.destproxymac ,pfrinfo->sa ,MACADDRLEN);
                memcpy((void *)Entry.proxiedmac, addr5 ,MACADDRLEN);								
                Entry.retry = 1U;
                Entry.PUSN = getPUSeq(priv);
                Entry.STAcount = 0x0001;
                Entry.update_time = jiffies;
                issue_proxyupdate_MP(priv,&Entry);
            }

#endif // PU_STANDARD_RX

            goto receive_packet;
							
        } // if( !memcmp(pfrinfo->da, GET_MY_HWADDR, MACADDRLEN) )
        else if (orig_priv->pmib->dot11OperationEntry.block_relay == 0)
        { 
            // dest is not me & relay is allow
            dapstat = get_stainfo(priv, pfrinfo->da);
            if(dapstat != NULL && isSTA(dapstat))
            {
                // filter packets that SA is myself 
                if ( memcmp(pfrinfo->sa, GET_MY_HWADDR, MACADDRLEN) == 0)
                    return FAIL;
                else
                {
receive_packet:
                    if (skb_p80211_to_ether(pskb->dev, privacy, pfrinfo) == FAIL) {
                        orig_priv->ext_stats.rx_data_drops++;
                        DEBUG_ERR("RX DROP: skb_p80211_to_ether fail!\n");
                        return FAIL;
                    }

to_bridge:
    
                    do_process = 1;
                    if(pfrinfo->is_11s&1) {
                        pskb->dev = priv->mesh_dev;
                        if ((orig_priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
                            orig_priv->pmib->reorderCtrlEntry.ReorderCtrlEnable) {
                            *(unsigned int *)&(pfrinfo->pskb->cb[4]) = 0;
                            pfrinfo->pskb->cb[3] = 0;
                            if (reorder_ctrl_check(orig_priv, pstat, pfrinfo) == FALSE) {
                                do_process = 0;
                            }
                        }    

                    }

                    if(do_process) {
                        rtl_netif_rx(orig_priv, pskb, pstat);
                    }

                    return SUCCESS;					
                }
            } 

            if (relay_11s_dataframe_ucast(priv, pskb, privacy, pfrinfo))
                return FAIL;	
        } 
		return SUCCESS;
    }   
}

#endif	// CONFIG_RTK_MESH
