/*Web server handler routines for IPv6
  *
  *Authors hf_shi	(hf_shi@realsil.com.cn) 2008.1.24
  *
  */

/*-- System inlcude files --*/
#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <cyg/hal/hal_tables.h>
#include <cyg/fileio/fileio.h>
#include <dirent.h>
#include <network.h>

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
//#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys_utility.h>

//#include "boa.h"
//#include "asp_page.h"
#include "apmib.h"
//#include "apform.h"
//#include "utility.h"
//#include "../system/sysconf.h"

#include "asp.h"
#include "asp_form.h"
#include "fmget.h"
#include "socket.h"

#ifdef HOME_GATEWAY
#ifdef CONFIG_IPV6

// system/sysconf.h

#define IPV6_ROUTE_PROC "/proc/net/ipv6_route"
#define IPV6_ADDR_PROC "/proc/net/if_inet6"

// src/apform.h
//#define _IPV6_RADVD_SCRIPT_PROG "radvd.sh"
//#define _IPV6_DNSMASQ_SCRIPT_PROG "dnsv6.sh"
//#define _IPV6_DHCPV6S_SCRIPT_PROG "dhcp6s"

// for in6_addr
#define s6_addr16 __u6_addr.__u6_addr16

#define req_get_cstream_var(wp, format, args...) get_cstream_var(postData, len, format, ##args)
#define req_format_write(wp, format, args...) __req_format_write(format, ##args)

static int __req_format_write(char *format, ...)
{
	int len;
	char tmpbuf[TMP_BUF_SIZE];
    va_list args;

    if (!format)
        return 0;

    va_start(args, format);
    len = vsnprintf(tmpbuf, sizeof(tmpbuf), format, args);
    va_end(args);

	if (len < 0)
		return len;

	return cyg_httpd_write_chunked(tmpbuf, len);
}

int getRadvdInfo(radvdCfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_RADVD_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}
#ifdef CONFIG_IPV6_WAN_RA
int getWanRadvdInfo(radvdCfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_RADVD_PARAM_WAN,(void *)entry)){
		return -1 ;        
	}
	return 0;
}
#endif

// TODO: dns v6
#if 0
int getDnsv6Info(dnsv6CfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_DNSV6_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}
#endif

int getDhcpv6sInfo(dhcp6sCfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_DHCPV6S_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}

// TODO: dhcp v6
#if 0
int getDhcpv6cInfo(dhcp6cCfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_DHCPV6C_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}
#endif

// TODO: ipv6 wan
#if 0
int getWanAdd6Info(addr6CfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_ADDR_WAN_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}

int getLanAddv6Info(addr6CfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_ADDR_LAN_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}

int getLanv6PrefixInfo(addr6CfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_ADDR_PFEFIX_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}

int getGatewayv6Info(addr6CfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_ADDR_GW_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}

int getDnsAddv6Info(addr6CfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_ADDR_DNS_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}
#endif

// TODO: tunnel6
#if 0
int getTunnel6Info(tunnelCfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_TUNNEL_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}
#endif

int create_Dhcp6CfgFile(dhcp6sCfgParam_t *dhcp6sCfg)
{
	FILE *fp;
	/*open /var/radvd.conf*/
	fp = fopen("/var/dhcp6s.conf", "w");
	if(NULL == fp)
		return -1;
	
	fprintf(fp,"#Dns\n");
	fprintf(fp,"option domain-name-servers %s;\n\n",dhcp6sCfg->DNSaddr6);

	fprintf(fp,"#Interface\n");
	fprintf(fp,"interface %s {\n",dhcp6sCfg->interfaceNameds);
	fprintf(fp,"	address-pool pool1 3600;\n");
	fprintf(fp,"};\n\n");

	fprintf(fp,"#Addrs Pool\n");
	fprintf(fp,"pool pool1 {\n");
	fprintf(fp,"	range %s to %s ;\n",dhcp6sCfg->addr6PoolS,dhcp6sCfg->addr6PoolE);
	fprintf(fp,"};\n\n");
	
	fclose(fp);
	return 0;
}

int getAddr6Info(addrIPv6CfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_ADDR_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}

int set_RadvdInterfaceParam(char *postData, int len, radvdCfgParam_t *pradvdCfgParam)
{
	char *tmp;
	uint32 value;

	/*check if enabled*/
	/*get cfg data from web*/
	tmp=req_get_cstream_var(wp,"interfacename","");
	if(strcmp(tmp,pradvdCfgParam->interface.Name))
	{
		/*interface name changed*/
		strcpy(pradvdCfgParam->interface.Name, tmp);
	}
	value =atoi(req_get_cstream_var(wp,"MaxRtrAdvInterval",""));
	if(value != pradvdCfgParam->interface.MaxRtrAdvInterval)
	{
		pradvdCfgParam->interface.MaxRtrAdvInterval = value;
	}
	value =atoi(req_get_cstream_var(wp,"MinRtrAdvInterval",""));
	if(value != pradvdCfgParam->interface.MinRtrAdvInterval)
	{
		pradvdCfgParam->interface.MinRtrAdvInterval = value;
	}
	value =atoi(req_get_cstream_var(wp,"MinDelayBetweenRAs",""));
	if(value != pradvdCfgParam->interface.MinDelayBetweenRAs)
	{
		pradvdCfgParam->interface.MinDelayBetweenRAs = value;
	}
	value =atoi(req_get_cstream_var(wp,"AdvManagedFlag",""));
	if(value > 0)
	{
		pradvdCfgParam->interface.AdvManagedFlag = 1;
	}
	else
	{
		pradvdCfgParam->interface.AdvManagedFlag =0; 
	}
	value =atoi(req_get_cstream_var(wp,"AdvOtherConfigFlag",""));
	if(value >0)
	{
		pradvdCfgParam->interface.AdvOtherConfigFlag = 1;
	}
	else
	{
		pradvdCfgParam->interface.AdvOtherConfigFlag =0;
	}
	value =atoi(req_get_cstream_var(wp,"AdvLinkMTU",""));
	if(value != pradvdCfgParam->interface.AdvLinkMTU)
	{
		pradvdCfgParam->interface.AdvLinkMTU = value;
	}
	/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
	tmp = req_get_cstream_var(wp,"AdvReachableTime","");
	value = strtoul(tmp,NULL,10);
	if(value != pradvdCfgParam->interface.AdvReachableTime)
	{
		pradvdCfgParam->interface.AdvReachableTime = value;
	}
	
	/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
	tmp = req_get_cstream_var(wp,"AdvRetransTimer","");
	value = strtoul(tmp,NULL,10);	
	if(value != pradvdCfgParam->interface.AdvRetransTimer)
	{
		pradvdCfgParam->interface.AdvRetransTimer = value;
	}
	value =atoi(req_get_cstream_var(wp,"AdvCurHopLimit",""));
	if(value != pradvdCfgParam->interface.AdvCurHopLimit)
	{
		pradvdCfgParam->interface.AdvCurHopLimit = value;
	}
	value =atoi(req_get_cstream_var(wp,"AdvDefaultLifetime",""));
	if(value != pradvdCfgParam->interface.AdvDefaultLifetime)
	{
		pradvdCfgParam->interface.AdvDefaultLifetime = value;
	}
	tmp=req_get_cstream_var(wp,"AdvDefaultPreference","");
	if(strcmp(tmp,pradvdCfgParam->interface.AdvDefaultPreference))
	{
		/*interface name changed*/
		strcpy(pradvdCfgParam->interface.AdvDefaultPreference, tmp);
	}
	value =atoi(req_get_cstream_var(wp,"AdvSourceLLAddress",""));
	
	if(value > 0)
	{
		pradvdCfgParam->interface.AdvSourceLLAddress = 1;
	}
	else
	{
		pradvdCfgParam->interface.AdvSourceLLAddress=0; 
	}
	value =atoi(req_get_cstream_var(wp,"UnicastOnly",""));
	if(value > 0)
	{
		pradvdCfgParam->interface.UnicastOnly = 1;
	}
	else
	{
		pradvdCfgParam->interface.UnicastOnly =0;
	}

	return 0;
}

int set_RadvdPrefixParam(char *postData, int len, radvdCfgParam_t *pradvdCfgParam)
{
	/*get cfg data from web*/
	char *tmpstr;
	char tmpname[30]={0};
	char tmpaddr[30]={0};
	uint32 value;
	int i,j;

	for(j=0;j<MAX_PREFIX_NUM;j++)
	{
		/*get prefix j*/
		sprintf(tmpname,"Enabled_%d",j);
		value=atoi(req_get_cstream_var(wp,tmpname,""));
		if(value >0)
		{
			pradvdCfgParam->interface.prefix[j].enabled = 1;
		}
		else
		{
			pradvdCfgParam->interface.prefix[j].enabled = 0;
		}
		
		for(i=0;i<8;i++)
		{			
			sprintf(tmpname,"radvdprefix%d_%d",j, i+1);
			sprintf(tmpaddr,"0x%s",req_get_cstream_var(wp, tmpname, ""));
			value =strtol(tmpaddr,NULL,16);
			pradvdCfgParam->interface.prefix[j].Prefix[i]= value;
		}

		sprintf(tmpname,"radvdprefix%d_len",j);
		value =atoi(req_get_cstream_var(wp,tmpname,""));
		if(value != pradvdCfgParam->interface.prefix[j].PrefixLen)
		{
			pradvdCfgParam->interface.prefix[j].PrefixLen = value;
		}
		sprintf(tmpname,"AdvOnLinkFlag_%d",j);
		value =atoi(req_get_cstream_var(wp,tmpname,""));
		if(value >0)
		{
			pradvdCfgParam->interface.prefix[j].AdvOnLinkFlag = 1;
		}
		else
		{
			pradvdCfgParam->interface.prefix[j].AdvOnLinkFlag = 0;
		}

		sprintf(tmpname,"AdvAutonomousFlag_%d",j);
		value =atoi(req_get_cstream_var(wp,tmpname,""));
		if(value >0)
		{
			pradvdCfgParam->interface.prefix[j].AdvAutonomousFlag = 1;
		}
		else
		{					
			pradvdCfgParam->interface.prefix[j].AdvAutonomousFlag = 0;
		}		
		sprintf(tmpname,"AdvValidLifetime_%d",j);
		tmpstr = req_get_cstream_var(wp,tmpname,"");
		/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
		value = strtoul(tmpstr,NULL,10);
		if(value != pradvdCfgParam->interface.prefix[j].AdvValidLifetime)
		{
			pradvdCfgParam->interface.prefix[j].AdvValidLifetime = value;
		}
		sprintf(tmpname,"AdvPreferredLifetime_%d",j);
		tmpstr = req_get_cstream_var(wp,tmpname,"");
		/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
		value = strtoul(tmpstr,NULL,10);
		if(value != pradvdCfgParam->interface.prefix[j].AdvPreferredLifetime)
		{
			pradvdCfgParam->interface.prefix[j].AdvPreferredLifetime = value;
		}
		sprintf(tmpname,"AdvRouterAddr_%d",j);
		value =atoi(req_get_cstream_var(wp,tmpname,""));
		if(value >0)
		{
			pradvdCfgParam->interface.prefix[j].AdvRouterAddr = 1;
		}
		else
		{
			pradvdCfgParam->interface.prefix[j].AdvRouterAddr=0;
		}
		sprintf(tmpname,"if6to4_%d",j);
		tmpstr =req_get_cstream_var(wp,tmpname,"");
		if(strcmp(pradvdCfgParam->interface.prefix[j].if6to4, tmpstr))
		{
			/*interface name changed*/
			strcpy(pradvdCfgParam->interface.prefix[j].if6to4, tmpstr);
		}
	}

	return 0;
}

int  set_RadvdParam(char *postData, int len, radvdCfgParam_t *pradvdCfgParam)
{
	
	int enable;
	/*get the configured paramter*/

	/*check if enabled*/
	/*get cfg data from web*/
	enable=atoi(req_get_cstream_var(wp,"enable_radvd",""));
	if(enable ^ pradvdCfgParam->enabled )
	{
       	pradvdCfgParam->enabled = enable;
	}
	if(enable)
	{
		/*set interface data*/
		set_RadvdInterfaceParam(postData, len, pradvdCfgParam);
		/*set prefix data*/
		set_RadvdPrefixParam(postData, len, pradvdCfgParam);
	}
	return 0;
}

int write_V6Prefix_V6Addr(char *buf, uint16 prefix[] , uint8 len)	
{
	/*valid check*/
	if(NULL == buf )
		return -1;
	if(len>128)
		return -1;
	/*an ipv6 address.full form*/
	sprintf(buf,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",prefix[0], prefix[1], prefix[2], prefix[3],
		prefix[4], prefix[5], prefix[6], prefix[7]);
	if(len<128)
		sprintf(buf+strlen(buf),"/%d",len);
	return 0;
}
int create_RadvdPrefixCfg(FILE *fp,struct AdvPrefix *prefix)
{
	char tmp[256];
	if(NULL == fp)
		return -1;
	/*create prefix part of radvd.conf file*/
 	memset(tmp,0,256);
	write_V6Prefix_V6Addr(tmp,prefix->Prefix,prefix->PrefixLen);
      fprintf(fp,"prefix %s\n",tmp);
      fprintf(fp,"{\n"); 
      /*on/off*/
      if(prefix->AdvOnLinkFlag)
	  	fprintf(fp,"AdvOnLink on;\n");
	/*on/off*/
      if(prefix->AdvAutonomousFlag)
	  	fprintf(fp,"AdvAutonomous on;\n");
	/*seconds|infinity Default: 2592000 seconds (30 days)*/
      /*if(prefix->AdvValidLifetime)*/
	fprintf(fp,"AdvValidLifetime %u;\n",prefix->AdvValidLifetime);
	/*seconds|infinity Default: 604800 seconds (7 days)*/
      /*if(prefix->AdvPreferredLifetime)*/
 	fprintf(fp,"AdvPreferredLifetime %u;\n",prefix->AdvPreferredLifetime);
        /* Mobile IPv6 extensions on/off*/
       if(prefix->AdvRouterAddr)
 	  	fprintf(fp,"AdvRouterAddr on;\n");
        /*6to4 interface*/
	 if(prefix->if6to4[0])
	 	fprintf(fp,"Base6to4Interface %s;\n",prefix->if6to4);
       fprintf(fp,"};\n");
	 return 0;
}

#ifdef SUPPORT_RDNSS_OPTION
void	write_radvdconf_rdnss_option(FILE *fp,struct Interface *interface)
{
	//add RDNSS 
	int dnsMode;
	char tmpBuf[256];
	char tmpStr[256];
	FILE*fpdns6=NULL;
	apmib_get(MIB_IPV6_DNS_AUTO, (void *)&dnsMode);		
	if(dnsMode==0)  //Set DNS Manually 
	{
		addr6CfgParam_t addr6_dns;
		
		apmib_get(MIB_IPV6_ADDR_DNS_PARAM,  (void *)&addr6_dns);
			
		snprintf(tmpBuf, sizeof(tmpBuf), "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", 
		addr6_dns.addrIPv6[0], addr6_dns.addrIPv6[1], addr6_dns.addrIPv6[2], addr6_dns.addrIPv6[3], 
		addr6_dns.addrIPv6[4], addr6_dns.addrIPv6[5], addr6_dns.addrIPv6[6], addr6_dns.addrIPv6[7]);

		//strcpy(tmpBuf, "2001:0000:0000:0000:0000:0000:0000:0045");
		if(strstr(tmpBuf, "0000:0000:0000:0000:0000:0000:0000:0000")==NULL)	
		{
			//add RDNSS 
			fprintf(fp, "RDNSS %s\n",tmpBuf);			
			fprintf(fp,"{\n");			
			fprintf(fp, "AdvRDNSSLifetime %d;\n", interface->MaxRtrAdvInterval);
			fprintf(fp, "};\n");
			
		}			
	}
	else
	{
		if(isFileExist("/var/dns6.conf"))
		{
			if((fpdns6=fopen("/var/dns6.conf","r"))!=NULL)
			{				
				memset(tmpStr, 0, sizeof(tmpStr));
				while(fgets(tmpBuf, sizeof(tmpBuf), fpdns6))
				{
					tmpBuf[strlen(tmpBuf)-1]=0;
					strcat(tmpStr, tmpBuf+strlen("nameserver")+1);
					strcat(tmpStr, " ");					
				}
				if(strlen(tmpStr>1)==0)
				{
					tmpStr[strlen(tmpStr)-1]=0;				
					fprintf(fp, "RDNSS %s\n",tmpStr);
					fprintf(fp, "{\n");
					fprintf(fp,"AdvRDNSSLifetime %d;\n", interface->MaxRtrAdvInterval);
					fprintf(fp,"};\n");
				}
				fclose(fpdns6);	
			}
		}
	}
}
#endif

#ifdef SUPPORT_DNSSL_OPTION
void	write_radvdconf_dnsl_option(FILE *fp,struct Interface *interface)
{
	char tmpBuf[256];

	//add DNSSL
	memset(tmpBuf, 0, sizeof(tmpBuf));
	apmib_get(MIB_DOMAIN_NAME, (void *)tmpBuf);	
	if(strlen(tmpBuf)>0)
	{
		fprintf(fp,"DNSSL %s.com %s.com.cn\n", tmpBuf, tmpBuf);
		fprintf(fp, "{\n");
		fprintf(fp, "AdvDNSSLLifetime %d;\n", interface->MaxRtrAdvInterval);
		fprintf(fp, "};\n");
	}
}
#endif

int create_radvdIntCfg(FILE *fp,struct Interface *interface)
{
	int i;
	if(NULL == fp)
		return -1;
	
	/*write the conf file according the radvdcfg*/
	fprintf(fp,"interface %s \n{\n",interface->Name);
	/*default send advertisement*/
	fprintf(fp,"AdvSendAdvert on;\n");
	/*seconds*/
	/*if the parameter default value !=0. but now not specified we take it as 0*/
	/*if(interface->MaxRtrAdvInterval)*/
	fprintf(fp,"MaxRtrAdvInterval %u;\n",interface->MaxRtrAdvInterval);
	/*seconds*/
      /*if(interface->MinRtrAdvInterval)*/
	 fprintf(fp,"MinRtrAdvInterval %u;\n",interface->MinRtrAdvInterval);
	/*seconds*/
      /*if(interface->MinDelayBetweenRAs)*/
	fprintf(fp,"MinDelayBetweenRAs %u;\n",interface->MinDelayBetweenRAs);
	/*on/off*/
      if(interface->AdvManagedFlag)
	  	fprintf(fp,"AdvManagedFlag on;\n");
	/*on/off*/
      if(interface->AdvOtherConfigFlag)
	  	fprintf(fp,"AdvOtherConfigFlag on;\n");
	/*integer*/
      /*if(interface->AdvLinkMTU)*/
	fprintf(fp,"AdvLinkMTU %d;\n",interface->AdvLinkMTU);
	/*milliseconds*/
	/*the following 2  default value is 0.*/
      //if(interface->AdvReachableTime)
	fprintf(fp,"AdvReachableTime %u;\n",interface->AdvReachableTime);
	/*milliseconds*/
      //if(interface->AdvRetransTimer)
 	fprintf(fp,"AdvRetransTimer %u;\n",interface->AdvRetransTimer);
	/*integer*/
      /*if(interface->AdvCurHopLimit)*/
	fprintf(fp,"AdvCurHopLimit %d;\n",interface->AdvCurHopLimit);
	/*seconds*/
      /*if(interface->AdvDefaultLifetime)*/
	fprintf(fp,"AdvDefaultLifetime %d;\n",interface->AdvDefaultLifetime);
      /*low,medium,high default medium*/
      if(interface->AdvDefaultPreference[0])
	  	fprintf(fp,"AdvDefaultPreference %s;\n",interface->AdvDefaultPreference);
	/*on/off*/
      if(interface->AdvSourceLLAddress)
	  	fprintf(fp,"AdvSourceLLAddress on;\n");
	/*on/off*/
      if(interface->UnicastOnly)
	  	fprintf(fp,"UnicastOnly on;\n");

      /*write prefix cfg*/
	for(i=0;i<MAX_PREFIX_NUM;i++)
	{
		if(interface->prefix[i].enabled)
			create_RadvdPrefixCfg(fp,&(interface->prefix[i]));
	}

#ifdef SUPPORT_RDNSS_OPTION
	write_radvdconf_rdnss_option(fp,interface);
#endif
#ifdef SUPPORT_DNSSL_OPTION
	write_radvdconf_dnsl_option(fp,interface);
#endif
	fprintf(fp,"};\n");

	return 0;
}
int create_RadvdCfgFile()
{
	FILE *fp;
	radvdCfgParam_t radvdcfg={0};
#ifdef CONFIG_IPV6_WAN_RA
	radvdCfgParam_t radvdcfg_wan={0};
#endif
	apmib_get(MIB_IPV6_RADVD_PARAM,&radvdcfg);
#ifdef CONFIG_IPV6_WAN_RA
	apmib_get(MIB_IPV6_RADVD_PARAM_WAN,&radvdcfg_wan);
#endif
	/*open /var/radvd.conf*/
	fp = fopen("/etc/radvd.conf", "w");
	if(NULL == fp)
		return -1;
	if( radvdcfg.enabled )
		create_radvdIntCfg(fp,&(radvdcfg.interface));
#ifdef CONFIG_IPV6_WAN_RA
	if(radvdcfg_wan.enabled)
		create_radvdIntCfg(fp,&(radvdcfg_wan.interface));
#endif
    fclose(fp);
	return 0;
}
void formRadvd(char *postData, int len)
{
#ifdef HAVE_RADVD
#define RADVD_IMMEDIATE
#endif
//	int pid;
//	char tmpBuf[256];
	char *submitUrl;
	char* value;
	radvdCfgParam_t radvdCfgParam;
	extern void request_radvd_reload( int enable );

	/*Get parameters*/
	getRadvdInfo(&radvdCfgParam);
	
	/*Set parameters*/
	value=req_get_cstream_var(wp,"submit","");
	if(0 == strcmp(value,"Save"))
	{
		set_RadvdParam(postData, len, &radvdCfgParam);
	}
	
	/*Set to pMIb*/
	apmib_set(MIB_IPV6_RADVD_PARAM,&radvdCfgParam);
	
	/*Update it to flash*/
	apmib_update(CURRENT_SETTING);

	/*create the config file*/
	create_RadvdCfgFile();
#ifdef RADVD_IMMEDIATE
	/*reload config (radvd is always alive)*/
	request_radvd_reload(1);
#endif
	/*start the Daemon*/
#if 0
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _IPV6_RADVD_SCRIPT_PROG);
		execl( tmpBuf, _IPV6_RADVD_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
#ifdef RADVD_IMMEDIATE
	send_redirect_perm(submitUrl);
#else
	OK_MSG(submitUrl);
#endif

  	return;
	
}
#ifdef CONFIG_IPV6_WAN_RA
void formRadvd_wan(char *postData, int len)
{
#ifdef HAVE_RADVD
#define RADVD_IMMEDIATE
#endif
//	int pid;
//	char tmpBuf[256];
	char *submitUrl;
	char* value;
	radvdCfgParam_t radvdCfgParam_wan;
	extern void request_radvd_reload( int enable );
	/*Get parameters*/
	getWanRadvdInfo(&radvdCfgParam_wan);
	
	/*Set parameters*/
	value=req_get_cstream_var(wp,"submit","");
	if(0 == strcmp(value,"Save"))
	{
		set_RadvdParam(postData, len, &radvdCfgParam_wan);
	}
	/*Set to pMIb*/
	apmib_set(MIB_IPV6_RADVD_PARAM_WAN,&radvdCfgParam_wan);
	/*Update it to flash*/
	apmib_update(CURRENT_SETTING);
	/*create the config file*/
	create_RadvdCfgFile();
#ifdef RADVD_IMMEDIATE
	/*reload config (radvd is always alive)*/
	request_radvd_reload(1);
#endif
	/*start the Daemon*/
#if 0
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _IPV6_RADVD_SCRIPT_PROG);
		execl( tmpBuf, _IPV6_RADVD_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
#ifdef RADVD_IMMEDIATE
	send_redirect_perm(submitUrl);
#else
	OK_MSG(submitUrl);
#endif

  	return;
	
}
#endif

// TODO: dns v6
#if 0
int  set_DnsParam(char *postData, int len, dnsv6CfgParam_t *pdnsv6CfgParam)
{
	char *value;
	int enable;
	/*check if enabled*/
	enable=atoi(req_get_cstream_var(wp,"enable_dnsv6",""));
	if(enable ^ pdnsv6CfgParam->enabled )
	{
       	pdnsv6CfgParam->enabled = enable;
	}
	if(enable)
	{
		value = req_get_cstream_var(wp,"routername","");
		strcpy(pdnsv6CfgParam->routerName,value);
	}
	return 0;
}
#endif

// TODO: dhcp v6
#if 0
int  set_DhcpSParam(char *postData, int len, dhcp6sCfgParam_t *dhcp6sCfgParam)
{
	char *value;
	int enable;
	/*check if enabled*/
	enable=atoi(req_get_cstream_var(wp,"enable_dhcpv6s",""));
	if(enable ^ dhcp6sCfgParam->enabled )
	{
       	dhcp6sCfgParam->enabled = enable;
	}

	value = req_get_cstream_var(wp,"dnsaddr","");
	strcpy(dhcp6sCfgParam->DNSaddr6,value);
	
	value = req_get_cstream_var(wp,"interfacenameds","");
	if(!strcmp(value,""))
	{
		sprintf(dhcp6sCfgParam->interfaceNameds,"%s","br0");
	}
	else
	{
		strcpy(dhcp6sCfgParam->interfaceNameds,value);
	}

	value = req_get_cstream_var(wp,"addrPoolStart","");
	strcpy(dhcp6sCfgParam->addr6PoolS,value);

	value = req_get_cstream_var(wp,"addrPoolEnd","");
	strcpy(dhcp6sCfgParam->addr6PoolE,value);
	
	return 0;
}

int  set_lan_addr6(char *postData, int len, addr6CfgParam_t *addr6)
{
	char *value;
	
	value=req_get_cstream_var(wp,"lan_ip_0","");	
	if(value!=NULL)
		addr6->addrIPv6[0]=strtol(value,NULL,16);
	
	value=req_get_cstream_var(wp,"lan_ip_1","");
	if(value!=NULL)
		addr6->addrIPv6[1]=strtol(value,NULL,16);
	
	value=req_get_cstream_var(wp,"lan_ip_2","");
	if(value!=NULL)
		addr6->addrIPv6[2]=strtol(value,NULL,16);
	
	value=req_get_cstream_var(wp,"lan_ip_3","");
	if(value!=NULL)
		addr6->addrIPv6[3]=strtol(value,NULL,16);
	
	value=req_get_cstream_var(wp,"lan_ip_4","");
	if(value!=NULL)
		addr6->addrIPv6[4]=strtol(value,NULL,16);
	
	value=req_get_cstream_var(wp,"lan_ip_5","");
	if(value!=NULL)
		addr6->addrIPv6[5]=strtol(value,NULL,16);
	
	value=req_get_cstream_var(wp,"lan_ip_6","");
	if(value!=NULL)
		addr6->addrIPv6[6]=strtol(value,NULL,16);	

	value=req_get_cstream_var(wp,"lan_ip_7","");
	if(value!=NULL)
		addr6->addrIPv6[7]=strtol(value,NULL,16);

	value=req_get_cstream_var(wp,"prefix_len_lan","");
	if(value!=NULL)					
		addr6->prefix_len=atoi(value);				
	
	return 0;
}
#endif


int  check_Addr6Param(char *postData, int len,addrIPv6CfgParam_t *addrIPv6CfgParam)
{
	char *tmpvalue;
	int iLoop;
	char paramName[20];
	uint32 validFlag=0;
	uint32 changeFlag=0;
	

	for(iLoop=1;iLoop<=8;iLoop++)
	{
		sprintf(paramName,"addr_1_%d",iLoop);
		tmpvalue = req_get_cstream_var(wp,paramName,"");
		if((strtol(tmpvalue,NULL,16) != 0x0)) break;
		//bzero(tmpvalue,sizeof(tmpvalue));
		bzero(paramName,sizeof(paramName));
	}		
	if(iLoop < 9)	validFlag |= 0x10;	
	
	for(iLoop=1;iLoop<=8;iLoop++)
	{
		sprintf(paramName,"addr_2_%d",iLoop);
		tmpvalue = req_get_cstream_var(wp,paramName,"");
		if((strtol(tmpvalue,NULL,16) != 0x0)) break;
		//bzero(tmpvalue,sizeof(tmpvalue));
		bzero(paramName,sizeof(paramName));
	}		
	if(iLoop < 9)	validFlag |= 0x20;	

	if(validFlag != 0x0) 
	{
		if((validFlag & 0x00f0) & 0x10)
		{
			for(iLoop=1;iLoop<=8;iLoop++)
			{
				sprintf(paramName,"addr_1_%d",iLoop);
				tmpvalue = req_get_cstream_var(wp,paramName,"");
				if((strtol(tmpvalue,NULL,16) != addrIPv6CfgParam->addrIPv6[0][iLoop-1])) 
				{
					changeFlag |=0x1;
					break;
				}
				//bzero(tmpvalue,sizeof(tmpvalue));
				bzero(paramName,sizeof(paramName));
			}	
			
			tmpvalue = req_get_cstream_var(wp,"prefix_len_1","");
			if((atoi(tmpvalue) != addrIPv6CfgParam->prefix_len[0])) 
			{
				addrIPv6CfgParam->prefix_len[0]=atoi(tmpvalue);
				changeFlag |=0x4;
			}
			//bzero(tmpvalue,sizeof(tmpvalue));
		}

		if((validFlag & 0x00f0) & 0x20)
		{
			for(iLoop=1;iLoop<=8;iLoop++)
			{
				sprintf(paramName,"addr_2_%d",iLoop);
				tmpvalue = req_get_cstream_var(wp,paramName,"");
				if((strtol(tmpvalue,NULL,16) != addrIPv6CfgParam->addrIPv6[1][iLoop-1])) 
				{
					changeFlag |=0x2;
					break;
				}
				//bzero(tmpvalue,sizeof(tmpvalue));
				bzero(paramName,sizeof(paramName));
			}	

			tmpvalue = req_get_cstream_var(wp,"prefix_len_2","");
			if((atoi(tmpvalue) != addrIPv6CfgParam->prefix_len[1])) 
			{
				addrIPv6CfgParam->prefix_len[1]=atoi(tmpvalue);
				changeFlag |=0x8;
			}
			//bzero(tmpvalue,sizeof(tmpvalue));
		}
	}
	
	changeFlag |= validFlag;	
	return changeFlag;		
}

void  del_PreAddr6Param(addrIPv6CfgParam_t addrIPv6CfgParam,uint32 _changFlag)
{
	#ifndef NO_ACTION
	char tmpBuf[128];
	#ifdef __ECOS
	int i, B_len, b_len;
	unsigned int mask;
	#endif

//	if(addrIPv6CfgParam.enabled == 1)	
	{
		if((_changFlag & 0x1) || (_changFlag & 0x4))
		{
			bzero(tmpBuf,sizeof(tmpBuf));

			#ifdef __ECOS
			sprintf(tmpBuf,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d", 
							addrIPv6CfgParam.addrIPv6[0][0],addrIPv6CfgParam.addrIPv6[0][1],
							addrIPv6CfgParam.addrIPv6[0][2],addrIPv6CfgParam.addrIPv6[0][3],
							addrIPv6CfgParam.addrIPv6[0][4],addrIPv6CfgParam.addrIPv6[0][5],
							addrIPv6CfgParam.addrIPv6[0][6],addrIPv6CfgParam.addrIPv6[0][7],
							addrIPv6CfgParam.prefix_len[0]);
			RunSystemCmd(NULL_FILE, "ifconfig", _IPV6_LAN_INTERFACE, "inet6", tmpBuf, "delete",  NULL_STR);

#ifdef HAVE_RADVD_SHELL
			/* Address for LAN Anycast */
			B_len = addrIPv6CfgParam.prefix_len[0]/16;
			b_len = addrIPv6CfgParam.prefix_len[0]%16;

			mask = 0;
			for (i=0;i<b_len;i++) mask |= 1 << 15-i;

			memset(tmpBuf,0,sizeof(tmpBuf));
			for (i=0;i<B_len;i++)
				sprintf(tmpBuf + strlen(tmpBuf),"%04x:",addrIPv6CfgParam.addrIPv6[0][i]);

			if (mask) 	sprintf(tmpBuf + strlen(tmpBuf),"%04x",addrIPv6CfgParam.addrIPv6[0][i] & mask);
			else 		tmpBuf[strlen(tmpBuf)-1]=0;  /* Remove ':' */

			if(B_len < 8) sprintf(tmpBuf + strlen(tmpBuf),"::");

			sprintf(tmpBuf+strlen(tmpBuf),"\/%d",addrIPv6CfgParam.prefix_len[0]);
			RunSystemCmd(NULL_FILE, "ifconfig", _IPV6_LAN_INTERFACE, "inet6", tmpBuf, "delete", NULL_STR);
			//printf("\nB_len=%d, b_len=%d, make=0x%04x\n",B_len, b_len, mask);
			//printf("ipv6 anycast %s\n", tmpBuf);
#endif
			#else
			sprintf(tmpBuf,"ifconfig %s delete %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d",_IPV6_LAN_INTERFACE, 
							addrIPv6CfgParam.addrIPv6[0][0],addrIPv6CfgParam.addrIPv6[0][1],
							addrIPv6CfgParam.addrIPv6[0][2],addrIPv6CfgParam.addrIPv6[0][3],
							addrIPv6CfgParam.addrIPv6[0][4],addrIPv6CfgParam.addrIPv6[0][5],
							addrIPv6CfgParam.addrIPv6[0][6],addrIPv6CfgParam.addrIPv6[0][7],
							addrIPv6CfgParam.prefix_len[0]);
			system(tmpBuf);
			#endif
		}
		if((_changFlag & 0x2) || (_changFlag & 0x8))
		{
			bzero(tmpBuf,sizeof(tmpBuf));
			#ifdef __ECOS
			sprintf(tmpBuf,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d", 
							addrIPv6CfgParam.addrIPv6[1][0],addrIPv6CfgParam.addrIPv6[1][1],
							addrIPv6CfgParam.addrIPv6[1][2],addrIPv6CfgParam.addrIPv6[1][3],
							addrIPv6CfgParam.addrIPv6[1][4],addrIPv6CfgParam.addrIPv6[1][5],
							addrIPv6CfgParam.addrIPv6[1][6],addrIPv6CfgParam.addrIPv6[1][7],
							addrIPv6CfgParam.prefix_len[1]);
			RunSystemCmd(NULL_FILE, "ifconfig", _IPV6_WAN_INTERFACE, "inet6", tmpBuf, "delete", NULL_STR);
			
#if 0
			/* Address for WAN Anycast */
			B_len = addrIPv6CfgParam.prefix_len[1]/16;
			b_len = addrIPv6CfgParam.prefix_len[1]%16;

			mask = 0;
			for (i=0;i<b_len;i++) mask |= 1 << 15-i;

			memset(tmpBuf,0,sizeof(tmpBuf));
			for (i=0;i<B_len;i++)
				sprintf(tmpBuf + strlen(tmpBuf),"%04x:",addrIPv6CfgParam.addrIPv6[1][i]);

			if (mask) 	sprintf(tmpBuf + strlen(tmpBuf),"%04x",addrIPv6CfgParam.addrIPv6[1][i] & mask);
			else 		tmpBuf[strlen(tmpBuf)-1]=0; /* Remove ':' */

			if(B_len < 8) sprintf(tmpBuf + strlen(tmpBuf),"::");

			sprintf(tmpBuf+strlen(tmpBuf),"\/%d",addrIPv6CfgParam.prefix_len[1]);
			RunSystemCmd(NULL_FILE, "ifconfig", _IPV6_WAN_INTERFACE, "inet6", tmpBuf, "delete", NULL_STR);
			//printf("\nB_len=%d, b_len=%d, make=0x%04x\n",B_len, b_len, mask);
			//printf("ipv6 anycast %s\n", tmpBuf);
#endif
			#else
			sprintf(tmpBuf,"ifconfig %s delete %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d",_IPV6_WAN_INTERFACE, 
							addrIPv6CfgParam.addrIPv6[1][0],addrIPv6CfgParam.addrIPv6[1][1],
							addrIPv6CfgParam.addrIPv6[1][2],addrIPv6CfgParam.addrIPv6[1][3],
							addrIPv6CfgParam.addrIPv6[1][4],addrIPv6CfgParam.addrIPv6[1][5],
							addrIPv6CfgParam.addrIPv6[1][6],addrIPv6CfgParam.addrIPv6[1][7],
							addrIPv6CfgParam.prefix_len[1]);
			system(tmpBuf);
			#endif
		}
	}
	#endif	
	return;
}


int  set_Addr6Param(char *postData, int len, addrIPv6CfgParam_t *addrIPv6CfgParam,uint32 _changFlag)
{
	char *value;

	if(_changFlag & 0x1)
	{
		value = req_get_cstream_var(wp,"addr_1_1","");
		addrIPv6CfgParam->addrIPv6[0][0]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_1_2","");
		addrIPv6CfgParam->addrIPv6[0][1]=strtol(value,NULL,16);
		value = req_get_cstream_var(wp,"addr_1_3","");
		addrIPv6CfgParam->addrIPv6[0][2]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_1_4","");
		addrIPv6CfgParam->addrIPv6[0][3]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_1_5","");
		addrIPv6CfgParam->addrIPv6[0][4]=strtol(value,NULL,16);
		value = req_get_cstream_var(wp,"addr_1_6","");
		addrIPv6CfgParam->addrIPv6[0][5]=strtol(value,NULL,16);
		value = req_get_cstream_var(wp,"addr_1_7","");
		addrIPv6CfgParam->addrIPv6[0][6]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_1_8","");
		addrIPv6CfgParam->addrIPv6[0][7]=strtol(value,NULL,16);	
	}	
		
	if(_changFlag & 0x2)
	{
		value = req_get_cstream_var(wp,"addr_2_1","");
		addrIPv6CfgParam->addrIPv6[1][0]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_2_2","");
		addrIPv6CfgParam->addrIPv6[1][1]=strtol(value,NULL,16);
		value = req_get_cstream_var(wp,"addr_2_3","");
		addrIPv6CfgParam->addrIPv6[1][2]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_2_4","");
		addrIPv6CfgParam->addrIPv6[1][3]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_2_5","");
		addrIPv6CfgParam->addrIPv6[1][4]=strtol(value,NULL,16);
		value = req_get_cstream_var(wp,"addr_2_6","");
		addrIPv6CfgParam->addrIPv6[1][5]=strtol(value,NULL,16);
		value = req_get_cstream_var(wp,"addr_2_7","");
		addrIPv6CfgParam->addrIPv6[1][6]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_2_8","");
		addrIPv6CfgParam->addrIPv6[1][7]=strtol(value,NULL,16);	
	}

	addrIPv6CfgParam->enabled=1;
	return 0;
}
int get_v6address(uint16 addr[])
{
	unsigned char mac[6];
	unsigned char zero[6]={0};
	apmib_get(MIB_ELAN_MAC_ADDR,mac);
	if(!memcmp(mac,zero,6))
		apmib_get(MIB_HW_NIC0_ADDR,mac);
	
	addr[0]=0xfe80;
	addr[1]=0x0000;
	addr[2]=0x0000;
	addr[3]=0x0000;
	addr[4]=(mac[0]<<8 | mac[1]) | 0x0200;
	addr[5]=0x00ff  |(mac[2]<<8);
	addr[6]=0xfe00 | (mac[3]);
	addr[7]=(mac[4]<<8 | mac[5]);
	return 0;
}

// TODO: dns v6
#if 0
int create_Dnsv6CfgFile(dnsv6CfgParam_t *dnsv6cfg)
{
      FILE *fp;
	uint16 v6linkaddr[8];
	/*open /var/dnsmasq.conf*/
	fp = fopen("/var/dnsmasq.conf","w");
	if(NULL == fp)
		return -1;
	/*Never forward plain names (without a dot or domain part)*/
	fprintf(fp,"domain-needed\n");
	/*Never forward addresses in the non-routed address spaces*/
	fprintf(fp,"bogus-priv\n");
	/*refer to /etc/resolv.conf*/
	fprintf(fp,"resolv-file=/etc/resolv.conf\n");
	/*strict-order disable*/
	fprintf(fp,"#strict-order\n");
	/*no resolv disable*/
	fprintf(fp,"#no-resolv\n");
	/*no poll disable*/
	fprintf(fp,"#no-poll\n");
	
	/*add router name and link-local address for ipv6 address query*/
	get_v6address(v6linkaddr);

	/*get route address eth1 link local ?*/
	if(dnsv6cfg->routerName[0])
	{
		
		fprintf(fp,"address=/%s/%x::%x:%x:%x:%x\n",dnsv6cfg->routerName,v6linkaddr[0],v6linkaddr[4],
			v6linkaddr[5],v6linkaddr[6],v6linkaddr[7]);	
	}
	else/*default name myrouter*/
	{
		fprintf(fp,"address=/myrouter/%x::%x:%x:%x:%x",v6linkaddr[0],v6linkaddr[4],
			v6linkaddr[5],v6linkaddr[6],v6linkaddr[7]);
	}
	fprintf(fp,"#listen-address=\n");
	fprintf(fp,"#bind-interfaces\n");
	fprintf(fp,"#no-hosts\n");
	fclose(fp);
	return 0;
}

void formDnsv6(char *postData, int len)
{
//	int pid;
//	char tmpBuf[256];
	char *submitUrl;
	char* value;
	dnsv6CfgParam_t dnsCfgParam;

	/*Get parameters*/
	getDnsv6Info(&dnsCfgParam);

	/*Set to Parameters*/
	value=req_get_cstream_var(wp,"submit","");
	if(0 == strcmp(value, "Save"))
	{
		set_DnsParam(postData, len, &dnsCfgParam);
	}
	
	/*Set to pMIb*/
	apmib_set(MIB_IPV6_DNSV6_PARAM,&dnsCfgParam);

	/*Update it to flash*/
	apmib_update(CURRENT_SETTING);

#if 0
	/*create the config file*/
	create_Dnsv6CfgFile(&dnsCfgParam);
	
	/*start the Daemon*/
#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _IPV6_DNSMASQ_SCRIPT_PROG);
		execl( tmpBuf, _IPV6_DNSMASQ_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);
	return;
}
#endif

// TODO: dhcp v6
#if 0
void formDhcpv6s(char *postData, int len)
{
	dhcp6sCfgParam_t dhcp6sCfgParam;
	addr6CfgParam_t	addr6;
	char tmpBuf[256];
	char *submitUrl;
	char* value;
	
	/*Get parameters**/
	getDhcpv6sInfo(&dhcp6sCfgParam);
	
	getLanAddv6Info(&addr6);
	sprintf(tmpBuf,"ifconfig br0 del %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d 2> /dev/null",
						addr6.addrIPv6[0],addr6.addrIPv6[1],addr6.addrIPv6[2],
						addr6.addrIPv6[3],addr6.addrIPv6[4],addr6.addrIPv6[5],
						addr6.addrIPv6[6],addr6.addrIPv6[7],addr6.prefix_len);
	system(tmpBuf);

	/*Set to Parameters*/
	value=req_get_cstream_var(wp,"submit","");
	if(0 == strcmp(value, "Save"))
	{
		set_lan_addr6(postData, len, &addr6);
		apmib_set(MIB_IPV6_ADDR_LAN_PARAM,&addr6);
		
		set_DhcpSParam(postData, len, &dhcp6sCfgParam);
		apmib_set(MIB_IPV6_DHCPV6S_PARAM,&dhcp6sCfgParam);

		/*Update it to flash*/
		apmib_update(CURRENT_SETTING);	
	}	
	
	/*create the config file*/
	create_Dhcp6CfgFile(&dhcp6sCfgParam);	
	
	/*start the Daemon*/
#if 0
	sprintf(tmpBuf,"ifconfig br0 add %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d",
					addr6.addrIPv6[0],addr6.addrIPv6[1],addr6.addrIPv6[2],
					addr6.addrIPv6[3],addr6.addrIPv6[4],addr6.addrIPv6[5],
					addr6.addrIPv6[6],addr6.addrIPv6[7],addr6.prefix_len);
	system(tmpBuf);
	sprintf(tmpBuf,"%s %s",_IPV6_DHCPV6S_SCRIPT_PROG, dhcp6sCfgParam.interfaceNameds);
	system(tmpBuf);
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);
	return;
}
#endif

// TODO: ipv6 wan
#if 0
void formIpv6Setup(char *postData, int len)
{
		addr6CfgParam_t	addr6_wan;
		addr6CfgParam_t addr6_gw;
		addr6CfgParam_t addr6_dns;
		addr6CfgParam_t addr6_prefix;
		dhcp6cCfgParam_t dhcp6cCfgParam;
		dnsv6CfgParam_t dnsCfgParam;
		char *submitUrl;
		char* strval;
		uint32 val;
			
		/*Set to Parameters*/
		strval=req_get_cstream_var(wp,"save","");
	
		if(0 == strcmp(strval, "Apply Changes"))
		{			
			strval=req_get_cstream_var(wp,"wan_enable","");
			val= atoi(strval);
			apmib_set(MIB_IPV6_WAN_ENABLE,&val);
		
			if(val){
				strval=req_get_cstream_var(wp,"OriginType","");
				if(strval!=NULL){
					val= atoi(strval);
					apmib_set(MIB_IPV6_ORIGIN_TYPE,&val);
				}
				
				strval=req_get_cstream_var(wp,"linkType","");
				if(strval!=NULL){
					val=atoi(strval);			
					apmib_set(MIB_IPV6_LINK_TYPE,&val);
				}

				strval=req_get_cstream_var(wp,"dnsType","");
				if(strval[0]){
					val=atoi(strval);			
					apmib_set(MIB_IPV6_DNS_AUTO,&val);
				}
				
				strval=req_get_cstream_var(wp,"enable_dhcpv6pd","");	
				
				if(strval!=NULL){
					val = atoi(strval);
					dhcp6cCfgParam.enabled=val;
				}
				strval=req_get_cstream_var(wp,"interfacename_pd","");	
				
				if(strval!=NULL){
					strcpy(&dhcp6cCfgParam.dhcp6pd.ifName,strval);
				}
				strval=req_get_cstream_var(wp,"sla_len","");	
				if(strval!=NULL){
					val = atoi(strval);
					dhcp6cCfgParam.dhcp6pd.sla_len=val;
				}
				strval=req_get_cstream_var(wp,"sla_id","");	
				if(strval!=NULL){
					val = atoi(strval);
					dhcp6cCfgParam.dhcp6pd.sla_id=val;
				}
				apmib_set(MIB_IPV6_DHCPV6C_PARAM,&dhcp6cCfgParam);
				
				strval=req_get_cstream_var(wp,"wan_ip_0","");	
				if(strval!=NULL)
					addr6_wan.addrIPv6[0]=strtol(strval,NULL,16);		
				strval=req_get_cstream_var(wp,"wan_ip_1","");	
				if(strval!=NULL)
					addr6_wan.addrIPv6[1]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_ip_2","");
				if(strval!=NULL)
					addr6_wan.addrIPv6[2]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_ip_3","");
				if(strval!=NULL)
					addr6_wan.addrIPv6[3]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_ip_4","");
				if(strval!=NULL)
					addr6_wan.addrIPv6[4]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_ip_5","");
				if(strval!=NULL)
					addr6_wan.addrIPv6[5]=strtol(strval,NULL,16);				
				strval=req_get_cstream_var(wp,"wan_ip_6","");
				if(strval!=NULL)
					addr6_wan.addrIPv6[6]=strtol(strval,NULL,16);	
				strval=req_get_cstream_var(wp,"wan_ip_7","");
				if(strval!=NULL)
					addr6_wan.addrIPv6[7]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"prefix_len_ip","");
				if(strval!=NULL)					
					addr6_wan.prefix_len=atoi(strval);				
				apmib_set(MIB_IPV6_ADDR_WAN_PARAM,&addr6_wan);

				strval=req_get_cstream_var(wp,"wan_gw_0","");
				if(strval!=NULL)
					addr6_gw.addrIPv6[0]=strtol(strval,NULL,16);	
				strval=req_get_cstream_var(wp,"wan_gw_1","");
				if(strval!=NULL)
					addr6_gw.addrIPv6[1]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_gw_2","");
				if(strval!=NULL)
					addr6_gw.addrIPv6[2]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_gw_3","");
				if(strval!=NULL)
					addr6_gw.addrIPv6[3]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_gw_4","");
				if(strval!=NULL)
					addr6_gw.addrIPv6[4]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_gw_5","");
				if(strval!=NULL)
					addr6_gw.addrIPv6[5]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_gw_6","");
				if(strval!=NULL)
					addr6_gw.addrIPv6[6]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_gw_7","");
				if(strval!=NULL)
					addr6_gw.addrIPv6[7]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"prefix_len_gw","");
				if(strval!=NULL)
					addr6_gw.prefix_len=atoi(strval);
				apmib_set(MIB_IPV6_ADDR_GW_PARAM,&addr6_gw);

				strval=req_get_cstream_var(wp,"child_prefix_0","");
				if(strval!=NULL)
					addr6_prefix.addrIPv6[0]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"child_prefix_1","");
				if(strval!=NULL)
					addr6_prefix.addrIPv6[1]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"child_prefix_2","");
				if(strval!=NULL)
					addr6_prefix.addrIPv6[2]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"child_prefix_3","");
				if(strval!=NULL)
					addr6_prefix.addrIPv6[3]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"child_prefix_4","");
				if(strval!=NULL)
					addr6_prefix.addrIPv6[4]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"child_prefix_5","");
				if(strval!=NULL)
					addr6_prefix.addrIPv6[5]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"child_prefix_6","");
				if(strval!=NULL)
					addr6_prefix.addrIPv6[6]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"child_prefix_7","");
				if(strval!=NULL)
					addr6_prefix.addrIPv6[7]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"prefix_len_childPrefixAddr","");
				if(strval!=NULL)
					addr6_prefix.prefix_len=atoi(strval);
				apmib_set(MIB_IPV6_ADDR_PFEFIX_PARAM,&addr6_prefix);

				strval=req_get_cstream_var(wp,"wan_dns1_0","");
				if(strval!=NULL)
					addr6_dns.addrIPv6[0]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_dns1_1","");
				if(strval!=NULL)
					addr6_dns.addrIPv6[1]=strtol(strval,NULL,16);	
				strval=req_get_cstream_var(wp,"wan_dns1_2","");
				if(strval!=NULL)
					addr6_dns.addrIPv6[2]=strtol(strval,NULL,16);	
				strval=req_get_cstream_var(wp,"wan_dns1_3","");
				if(strval!=NULL)
					addr6_dns.addrIPv6[3]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_dns1_4","");
				if(strval!=NULL)
					addr6_dns.addrIPv6[4]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_dns1_5","");
				if(strval!=NULL)
					addr6_dns.addrIPv6[5]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_dns1_6","");
				if(strval!=NULL)
					addr6_dns.addrIPv6[6]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"wan_dns1_7","");
				if(strval!=NULL)
					addr6_dns.addrIPv6[7]=strtol(strval,NULL,16);
				strval=req_get_cstream_var(wp,"prefix_len_dns1","");
				if(strval!=NULL)
					addr6_dns.prefix_len=atoi(strval);
				apmib_set(MIB_IPV6_ADDR_DNS_PARAM,&addr6_dns);

				/*Get parameters*/
				getDnsv6Info(&dnsCfgParam);
				/*Set to Parameters*/
				set_DnsParam(postData, len, &dnsCfgParam);				
				/*Set to pMIb*/
				apmib_set(MIB_IPV6_DNSV6_PARAM,&dnsCfgParam);
				
				strval = req_get_cstream_var(wp, ("mldproxyEnabled"), "");
				if ( !strcmp(strval, "ON"))
					val = 0;
				else
					val = 1;
				if ( !apmib_set(MIB_MLD_PROXY_DISABLED, (void *)&val)) {
					printf ("Set MIB_MLD_PROXY_DISABLED error!");
					return;
				}
	
			}
			/*Update it to flash*/
			apmib_update(CURRENT_SETTING);	
		}
			
		submitUrl = req_get_cstream_var(wp, "submit-url", "");	 // hidden page
		OK_MSG(submitUrl);
		return;
		
}
#endif

void formIPv6Addr(char *postData, int len)
{
	addrIPv6CfgParam_t addrIPv6CfgParam,addrIPv6CfgParamBak;
	char tmpBuf[256];
	char *submitUrl;
	char *msg;
	char* value;
	uint32 isChangFlag=0;
	#ifdef __ECOS
	int i, B_len, b_len;
	unsigned int mask;
	#endif

	/*Get parameters**/
	getAddr6Info(&addrIPv6CfgParam);
	addrIPv6CfgParamBak = addrIPv6CfgParam;

	/*Set to Parameters*/
	value=req_get_cstream_var(wp,"submit","");
	if(0 == strcmp(value, "Save"))
	{
		isChangFlag=check_Addr6Param(postData, len, &addrIPv6CfgParam);

		//Case: invalid address
		if((isChangFlag & 0x00f0) == 0x0)
		{
			msg = "Invalid Addresses!";
			goto FAIL;
		}
		
		//Case: No change
		if((isChangFlag & 0x000f) == 0x0)
		{
			/*
			if((isChangFlag & 0x00f0)&0x20)
				msg = "Br's Address is invalid!";
			else if((isChangFlag & 0x00f0)&0x10)
				msg = "Eth0's Address is invalid!";
			else
			*/
				msg = "No Address Changed!";
			goto FAIL;
		}		
		
		//set to Parameters		
		set_Addr6Param(postData, len, &addrIPv6CfgParam,isChangFlag);
		
	}
	
	/*Set to pMIb*/
	apmib_set(MIB_IPV6_ADDR_PARAM,&addrIPv6CfgParam);

	/*Update it to flash*/
	apmib_update(CURRENT_SETTING);		

	#ifndef NO_ACTION	
	//Del Old Addr6
	del_PreAddr6Param(addrIPv6CfgParamBak,isChangFlag);

	//Add New Addr6
	if((isChangFlag & 0x1) ||(isChangFlag & 0x4))
	{
		#ifdef __ECOS
		sprintf(tmpBuf,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d", 
						addrIPv6CfgParam.addrIPv6[0][0],addrIPv6CfgParam.addrIPv6[0][1],
						addrIPv6CfgParam.addrIPv6[0][2],addrIPv6CfgParam.addrIPv6[0][3],
						addrIPv6CfgParam.addrIPv6[0][4],addrIPv6CfgParam.addrIPv6[0][5],
						addrIPv6CfgParam.addrIPv6[0][6],addrIPv6CfgParam.addrIPv6[0][7],
						addrIPv6CfgParam.prefix_len[0]);
		RunSystemCmd(NULL_FILE, "ifconfig", _IPV6_LAN_INTERFACE, "inet6", tmpBuf, NULL_STR);

#ifdef HAVE_RADVD_SHELL
		/* Address for LAN Anycast */
		B_len = addrIPv6CfgParam.prefix_len[0]/16;
		b_len = addrIPv6CfgParam.prefix_len[0]%16;

		mask = 0;
		for (i=0;i<b_len;i++) mask |= 1 << 15-i;

		memset(tmpBuf,0,sizeof(tmpBuf));
		for (i=0;i<B_len;i++)
			sprintf(tmpBuf + strlen(tmpBuf),"%04x:",addrIPv6CfgParam.addrIPv6[0][i]);

		if (mask) 	sprintf(tmpBuf + strlen(tmpBuf),"%04x",addrIPv6CfgParam.addrIPv6[0][i] & mask);
		else 		tmpBuf[strlen(tmpBuf)-1]=0;  /* Remove ':' */

		if(B_len < 8) sprintf(tmpBuf + strlen(tmpBuf),"::");

		sprintf(tmpBuf+strlen(tmpBuf),"\/%d",addrIPv6CfgParam.prefix_len[0]);
		RunSystemCmd(NULL_FILE, "ifconfig", _IPV6_LAN_INTERFACE, "inet6", tmpBuf, "anycast", NULL_STR);
		//printf("\nB_len=%d, b_len=%d, make=0x%04x\n",B_len, b_len, mask);
		//printf("ipv6 anycast %s\n", tmpBuf);
#endif
		#else
		bzero(tmpBuf,sizeof(tmpBuf));
		sprintf(tmpBuf,"ifconfig %s %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d",_IPV6_LAN_INTERFACE, 
						addrIPv6CfgParam.addrIPv6[0][0],addrIPv6CfgParam.addrIPv6[0][1],
						addrIPv6CfgParam.addrIPv6[0][2],addrIPv6CfgParam.addrIPv6[0][3],
						addrIPv6CfgParam.addrIPv6[0][4],addrIPv6CfgParam.addrIPv6[0][5],
						addrIPv6CfgParam.addrIPv6[0][6],addrIPv6CfgParam.addrIPv6[0][7],
						addrIPv6CfgParam.prefix_len[0]);
		system(tmpBuf);
		#endif
		bzero(tmpBuf,sizeof(tmpBuf));
	}

	if((isChangFlag & 0x2)||(isChangFlag & 0x8))
	{
		#ifdef __ECOS
		sprintf(tmpBuf,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d", 
						addrIPv6CfgParam.addrIPv6[1][0],addrIPv6CfgParam.addrIPv6[1][1],
						addrIPv6CfgParam.addrIPv6[1][2],addrIPv6CfgParam.addrIPv6[1][3],
						addrIPv6CfgParam.addrIPv6[1][4],addrIPv6CfgParam.addrIPv6[1][5],
						addrIPv6CfgParam.addrIPv6[1][6],addrIPv6CfgParam.addrIPv6[1][7],
						addrIPv6CfgParam.prefix_len[1]);
		RunSystemCmd(NULL_FILE, "ifconfig", _IPV6_WAN_INTERFACE, "inet6", tmpBuf, NULL_STR);

#if 0
			/* Address for WAN Anycast */
			B_len = addrIPv6CfgParam.prefix_len[1]/16;
			b_len = addrIPv6CfgParam.prefix_len[1]%16;

			mask = 0;
			for (i=0;i<b_len;i++) mask |= 1 << 15-i;

			memset(tmpBuf,0,sizeof(tmpBuf));
			for (i=0;i<B_len;i++)
				sprintf(tmpBuf + strlen(tmpBuf),"%04x:",addrIPv6CfgParam.addrIPv6[1][i]);

			if (mask) 	sprintf(tmpBuf + strlen(tmpBuf),"%04x",addrIPv6CfgParam.addrIPv6[1][i] & mask);
			else 		tmpBuf[strlen(tmpBuf)-1]=0; /* Remove ':' */

			if(B_len < 8) sprintf(tmpBuf + strlen(tmpBuf),"::");

			sprintf(tmpBuf+strlen(tmpBuf),"\/%d",addrIPv6CfgParam.prefix_len[1]);
			RunSystemCmd(NULL_FILE, "ifconfig", _IPV6_WAN_INTERFACE, "inet6", tmpBuf, "anycast", NULL_STR);
			//printf("\nB_len=%d, b_len=%d, make=0x%04x\n",B_len, b_len, mask);
			//printf("ipv6 anycast %s\n", tmpBuf);
#endif
		#else
		sprintf(tmpBuf,"ifconfig %s %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d",_IPV6_WAN_INTERFACE, 
						addrIPv6CfgParam.addrIPv6[1][0],addrIPv6CfgParam.addrIPv6[1][1],
						addrIPv6CfgParam.addrIPv6[1][2],addrIPv6CfgParam.addrIPv6[1][3],
						addrIPv6CfgParam.addrIPv6[1][4],addrIPv6CfgParam.addrIPv6[1][5],
						addrIPv6CfgParam.addrIPv6[1][6],addrIPv6CfgParam.addrIPv6[1][7],
						addrIPv6CfgParam.prefix_len[1]);
		system(tmpBuf);
		#endif
		bzero(tmpBuf,sizeof(tmpBuf));
	}
	#endif
	
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	//OK_MSG(submitUrl);
	//return;

#ifdef REBOOT_CHECK
	if(needReboot == 1)
	{
		OK_MSG(submitUrl);
		return;
	}
#endif

	if (submitUrl[0])
		send_redirect_perm(submitUrl);
  	return;


FAIL:
	ERR_MSG(msg);
	return;
}

// TODO: tunnel6
#if 0
void formTunnel6(char *postData, int len)
{
	tunnelCfgParam_t tunnelCfgParam;
	char tmpBuf[256];
	char *submitUrl;
	char* value;
	unsigned char buffer[50],wanIP[50];
	int enable;
	
	/*Get parameters**/
	getTunnel6Info(&tunnelCfgParam);

	/*Set to Parameters*/
	value=req_get_cstream_var(wp,"submit","");
	if(0 == strcmp(value, "Save"))
	{
		enable=atoi(req_get_cstream_var(wp,"enable_tunnel6",""));
		if(enable ^ tunnelCfgParam.enabled )
		{
	       	tunnelCfgParam.enabled = enable;
		}
	}

	/*Set to pMIb*/
	apmib_set(MIB_IPV6_TUNNEL_PARAM,&tunnelCfgParam);

	/*Update it to flash*/
	apmib_update(CURRENT_SETTING);	
	
	/*start the Daemon*/
#ifndef NO_ACTION
	//tunnel add 
	if ( !apmib_get( MIB_WAN_IP_ADDR,  (void *)buffer) ) goto setErr_tunnel;
	sprintf(wanIP,"%s",inet_ntoa(*((struct in_addr *)buffer)));
	sprintf(tmpBuf,"ip tunnel add tun mode sit remote any local %s",wanIP);
	system(tmpBuf);	

	bzero(tmpBuf,sizeof(tmpBuf));
	sprintf(tmpBuf,"ifconfig tun up");
	system(tmpBuf);

	char *p1,*p2,*p3,*p4;
	p1=strtok(wanIP,".");
	p2=strtok(NULL,".");
	p3=strtok(NULL,".");
	p4=strtok(NULL,".");

	bzero(tmpBuf,sizeof(tmpBuf));
	sprintf(tmpBuf,"ifconfig tun 2002:%02x%02x:%02x%02x:1::1\/16",atoi(p1),atoi(p2),atoi(p3),atoi(p4));
	system(tmpBuf);

	//br0
	bzero(tmpBuf,sizeof(tmpBuf));
	sprintf(tmpBuf,"ifconfig br0 2002:%02x%02x:%02x%02x:2::1\/64",atoi(p1),atoi(p2),atoi(p3),atoi(p4));
	system(tmpBuf);
	
#endif
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);
	return;
setErr_tunnel:
	
	return;
}
#endif

/* 
  *  Get constant string :Router Advertisement Setting
  */
uint32 getIPv6Info(int argc, char **argv)
{
	char	*name;
	radvdCfgParam_t radvdCfgParam;
// TODO: dns v6	
//	dnsv6CfgParam_t dnsv6CfgParam;
// TODO: dhcp v6	
//	dhcp6sCfgParam_t dhcp6sCfgParam;
// TODO: tunnel6
//	tunnelCfgParam_t tunnelCfgParam;
	
	//printf("get parameter=%s\n", argv[0]);
	name = argv[0];
	if (name == NULL) {
   		fprintf(stderr, "Insufficient args\n");
   		return -1;
   	}
	
//////MENU//////////////////////////////////////////////////
	if(!strcmp(name,"IPv6_Menu"))
	{
		req_format_write(wp,"menu.addItem(\"ipv6\");");
		req_format_write(wp,"ipv6 = new MTMenu();");
		req_format_write(wp,"ipv6.addItem(\"IPv6 Basic Setting\", \"ipv6_basic.htm\", \"\", \"Configure IPv6 Basic Setting\");\n");
// TODO: ipv6 wan
//		req_format_write(wp,"ipv6.addItem(\"IPv6 Wan Setting\", \"ipv6_wan.htm\", \"\", \"Configure IPv6 Wan Setting\");\n");
// TODO: dhcp v6
//		req_format_write(wp,"ipv6.addItem(\"IPv6 Lan Setting\", \"dhcp6s.htm\", \"\", \"Setup IPv6 Lan\");\n");
		req_format_write(wp,"ipv6.addItem(\"Router Advertisement Daemon\", \"radvd.htm\", \"\", \"Setup Radvd Daemon\");\n");
#ifdef CONFIG_IPV6_WAN_RA
		req_format_write(wp,"ipv6.addItem(\"WAN Router Advertisement Daemon\", \"radvd_wan.htm\", \"\", \"Setup Wan Radvd Daemon\");\n");
#endif
// TODO: dns v6
		//req_format_write(wp,"ipv6.addItem(\"DNS Proxy Daemon\", \"dnsv6.htm\", \"\", \"Setup Dnsmasq Daemon\");\n");
// TODO: tunnel6
//		req_format_write(wp,"ipv6.addItem(\"Tunnel (6 over 4)\", \"tunnel6.htm\", \"\", \"Tunnel (6to4)\");\n"); 
		req_format_write(wp,"menu.makeLastSubmenu(ipv6);\n");
		return 0;
	}

	if(!strcmp(name,"IPv6_nojs_Menu"))
	{
		req_format_write(wp,"<tr><td><b>IPv6</b></td></tr>");
		req_format_write(wp,"<tr><td><a href=\"ipv6_basic.htm\" target=\"view\">IPv6 Basic Setting</a></td></tr>");
// TODO: ipv6 wan
//		req_format_write(wp,"<tr><td><a href=\"ipv6_wan.htm\" target=\"view\">IPv6 Wan Setting</a></td></tr>");
// TODO: dhcp v6
//		req_format_write(wp,"<tr><td><a href=\"dhcp6s.htm\" target=\"view\">IPv6 LAN Setting</a></td></tr>");
		req_format_write(wp,"<tr><td><a href=\"radvd.htm\" target=\"view\">Router Advertisement Daemon</a></td></tr>");
// TODO: dns v6
//		req_format_write(wp,"<tr><td><a href=\"dnsv6.htm\" target=\"view\">DNS Proxy Daemon</a></td></tr>");
// TODO: tunnel6
//		req_format_write(wp,"<tr><td><a href=\"tunnel6.htm\" target=\"view\">Tunnel 6over4</a></td></tr>");
		return 0;
	}
//////////radvd///////////////////////////////////////////////////////////////
	if(getRadvdInfo(&radvdCfgParam)<0)
	{
		req_format_write(wp,"Read Radvd Configuration Error");
		return -1;
	}
      if(!strcmp(name,"enable_radvd"))
      {
       	if(radvdCfgParam.enabled)
        		req_format_write(wp,"checked");
      }
      else if(!strcmp(name,"radvdinterfacename"))
        {
        	req_format_write(wp,"%s",radvdCfgParam.interface.Name);
        }
      else if(!strcmp(name,"MaxRtrAdvInterval"))
        {
        	req_format_write(wp,"%u",radvdCfgParam.interface.MaxRtrAdvInterval);
        }
	else  if(!strcmp(name,"MinRtrAdvInterval"))
        {
        	req_format_write(wp,"%u",radvdCfgParam.interface.MinRtrAdvInterval);
        }
	else  if(!strcmp(name,("MinDelayBetweenRAs")))
        {
        	req_format_write(wp,"%u",radvdCfgParam.interface.MinDelayBetweenRAs);
        }
	else  if(!strcmp(name,("AdvManagedFlag")))
        {
		if(radvdCfgParam.interface.AdvManagedFlag)
        		req_format_write(wp,"checked");
        }
	 else if(!strcmp(name,("AdvOtherConfigFlag")))
        {
        	if(radvdCfgParam.interface.AdvOtherConfigFlag)
        		req_format_write(wp,"checked");
        }
	else  if(!strcmp(name,("AdvLinkMTU")))
        {
        	req_format_write(wp,"%d",radvdCfgParam.interface.AdvLinkMTU);
        }
	 else if(!strcmp(name,("AdvReachableTime")))
        {
        	req_format_write(wp,"%u",radvdCfgParam.interface.AdvReachableTime);
        }
	else  if(!strcmp(name,("AdvRetransTimer")))
        {
        	req_format_write(wp,"%u",radvdCfgParam.interface.AdvRetransTimer);
        }
	 else if(!strcmp(name,("AdvCurHopLimit")))
        {
        	req_format_write(wp,"%d",radvdCfgParam.interface.AdvCurHopLimit);
        }
	else  if(!strcmp(name,("AdvDefaultLifetime")))
        {
        	req_format_write(wp,"%d",radvdCfgParam.interface.AdvDefaultLifetime);
        }
	 else if(!strcmp(name,("AdvDefaultPreference_high")))
        {
        	if(!strcmp("high",radvdCfgParam.interface.AdvDefaultPreference))
			req_format_write(wp,"selected");	
         }
	 else if(!strcmp(name,("AdvDefaultPreference_medium")))
        {
        	if(!strcmp("medium",radvdCfgParam.interface.AdvDefaultPreference))
			req_format_write(wp,"selected");	
        }
	 else if(!strcmp(name,("AdvDefaultPreference_low")))
        {
        	if(!strcmp("low",radvdCfgParam.interface.AdvDefaultPreference))
			req_format_write(wp,"selected");	
        }
	 else if(!strcmp(name,("AdvSourceLLAddress")))
        {
                if(radvdCfgParam.interface.AdvSourceLLAddress)
        		req_format_write(wp,"checked");
        }
	else  if(!strcmp(name,("UnicastOnly")))
        {
                	if(radvdCfgParam.interface.UnicastOnly)
        		req_format_write(wp,"checked");
        }

	 /*prefix0*/
	else if(!strcmp(name,("Enabled_0")))
	{
		if(radvdCfgParam.interface.prefix[0].enabled)
			req_format_write(wp,"checked");
	}
	else if(!strcmp(name,("radvdprefix0_1")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[0]);
	 }
	else if(!strcmp(name,("radvdprefix0_2")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[1]);
	 }
	else if(!strcmp(name,("radvdprefix0_3")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[2]);
	 }
	else if(!strcmp(name,("radvdprefix0_4")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[3]);
	 }
	else if(!strcmp(name,("radvdprefix0_5")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[4]);
	 }
	else if(!strcmp(name,("radvdprefix0_6")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[5]);
	 }
	else if(!strcmp(name,("radvdprefix0_7")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[6]);
	 }
	else if(!strcmp(name,("radvdprefix0_8")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[7]);
	 }
	else if(!strcmp(name,("radvdprefix0_len")))
	 {
	 	req_format_write(wp,"%d",radvdCfgParam.interface.prefix[0].PrefixLen);
	 }


	else if(!strcmp(name,("AdvOnLinkFlag_0")))
	 {
	       if(radvdCfgParam.interface.prefix[0].AdvOnLinkFlag)
        		req_format_write(wp,"checked");
	 }
	
	else if(!strcmp(name,("AdvAutonomousFlag_0")))
	 {
	 	 if(radvdCfgParam.interface.prefix[0].AdvAutonomousFlag)
        		req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("AdvValidLifetime_0")))
	 {
	 	req_format_write(wp,"%u",radvdCfgParam.interface.prefix[0].AdvValidLifetime);
	 }
	else if(!strcmp(name,("AdvPreferredLifetime_0")))
	 {
	 	req_format_write(wp,"%u",radvdCfgParam.interface.prefix[0].AdvPreferredLifetime);
	 }
	else if(!strcmp(name,("AdvRouterAddr_0")))
	 {
	 	if(radvdCfgParam.interface.prefix[0].AdvRouterAddr)
        		req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("if6to4_0")))
	 {
	 	req_format_write(wp,"%s",radvdCfgParam.interface.prefix[0].if6to4);
	 }
	  
	 /*prefix1*/
	else if(!strcmp(name,("Enabled_1")))
	{
		if(radvdCfgParam.interface.prefix[1].enabled)
			req_format_write(wp,"checked");
	}
      else  if(!strcmp(name,("radvdprefix1_1")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[0]);
	 }
	else if(!strcmp(name,("radvdprefix1_2")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[1]);
	 }
	else if(!strcmp(name,("radvdprefix1_3")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[2]);
	 }
	else if(!strcmp(name,("radvdprefix1_4")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[3]);
	 }
	else if(!strcmp(name,("radvdprefix1_5")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[4]);
	 }
	else if(!strcmp(name,("radvdprefix1_6")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[5]);
	 }
	else if(!strcmp(name,("radvdprefix1_7")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[6]);
	 }
	else if(!strcmp(name,("radvdprefix1_8")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[7]);
	 }
	else if(!strcmp(name,("radvdprefix1_len")))
	 {
	 	req_format_write(wp,"%d",radvdCfgParam.interface.prefix[1].PrefixLen);
	 }
	else if(!strcmp(name,("AdvOnLinkFlag_1")))
	 {
	 	if(radvdCfgParam.interface.prefix[1].AdvOnLinkFlag)
			req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("AdvAutonomousFlag_1")))
	 {
	 	 if(radvdCfgParam.interface.prefix[1].AdvAutonomousFlag)
			req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("AdvValidLifetime_1")))
	 {
	 	req_format_write(wp,"%u",radvdCfgParam.interface.prefix[1].AdvValidLifetime);
	 }
	else if(!strcmp(name,("AdvPreferredLifetime_1")))
	 {
	 	req_format_write(wp,"%u",radvdCfgParam.interface.prefix[1].AdvPreferredLifetime);
	 }
	else if(!strcmp(name,("AdvRouterAddr_1")))
	 {
	 	 if(radvdCfgParam.interface.prefix[1].AdvRouterAddr)
			req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("if6to4_1")))
	 {
	 	req_format_write(wp,"%s",radvdCfgParam.interface.prefix[1].if6to4);
	 }
#ifdef CONFIG_IPV6_WAN_RA
//////////radvd_wan///////////////////////////////////////////////////////////////
	if(getWanRadvdInfo(&radvdCfgParam)<0)
	{
		req_format_write(wp,"Read Radvd Configuration Error");
		return -1;
	}
	  if(!strcmp(name,"enable_radvd_wan"))
	  {
		if(radvdCfgParam.enabled)
				req_format_write(wp,"checked");
	  }
	  else if(!strcmp(name,"radvdinterfacename_wan"))
		{
			req_format_write(wp,"%s",radvdCfgParam.interface.Name);
		}
	  else if(!strcmp(name,"MaxRtrAdvInterval_wan"))
		{
			req_format_write(wp,"%u",radvdCfgParam.interface.MaxRtrAdvInterval);
		}
	else  if(!strcmp(name,"MinRtrAdvInterval_wan"))
		{
			req_format_write(wp,"%u",radvdCfgParam.interface.MinRtrAdvInterval);
		}
	else  if(!strcmp(name,("MinDelayBetweenRAs_wan")))
		{
			req_format_write(wp,"%u",radvdCfgParam.interface.MinDelayBetweenRAs);
		}
	else  if(!strcmp(name,("AdvManagedFlag_wan")))
		{
		if(radvdCfgParam.interface.AdvManagedFlag)
				req_format_write(wp,"checked");
		}
	 else if(!strcmp(name,("AdvOtherConfigFlag_wan")))
		{
			if(radvdCfgParam.interface.AdvOtherConfigFlag)
				req_format_write(wp,"checked");
		}
	else  if(!strcmp(name,("AdvLinkMTU_wan")))
		{
			req_format_write(wp,"%d",radvdCfgParam.interface.AdvLinkMTU);
		}
	 else if(!strcmp(name,("AdvReachableTime_wan")))
		{
			req_format_write(wp,"%u",radvdCfgParam.interface.AdvReachableTime);
		}
	else  if(!strcmp(name,("AdvRetransTimer_wan")))
		{
			req_format_write(wp,"%u",radvdCfgParam.interface.AdvRetransTimer);
		}
	 else if(!strcmp(name,("AdvCurHopLimit_wan")))
		{
			req_format_write(wp,"%d",radvdCfgParam.interface.AdvCurHopLimit);
		}
	else  if(!strcmp(name,("AdvDefaultLifetime_wan")))
		{
			req_format_write(wp,"%d",radvdCfgParam.interface.AdvDefaultLifetime);
		}
	 else if(!strcmp(name,("AdvDefaultPreference_high_wan")))
		{
			if(!strcmp("high",radvdCfgParam.interface.AdvDefaultPreference))
			req_format_write(wp,"selected");	
		 }
	 else if(!strcmp(name,("AdvDefaultPreference_medium_wan")))
		{
			if(!strcmp("medium",radvdCfgParam.interface.AdvDefaultPreference))
			req_format_write(wp,"selected");	
		}
	 else if(!strcmp(name,("AdvDefaultPreference_low_wan")))
		{
			if(!strcmp("low",radvdCfgParam.interface.AdvDefaultPreference))
			req_format_write(wp,"selected");	
		}
	 else if(!strcmp(name,("AdvSourceLLAddress_wan")))
		{
				if(radvdCfgParam.interface.AdvSourceLLAddress)
				req_format_write(wp,"checked");
		}
	else  if(!strcmp(name,("UnicastOnly_wan")))
		{
					if(radvdCfgParam.interface.UnicastOnly)
				req_format_write(wp,"checked");
		}

	 /*prefix0*/
	else if(!strcmp(name,("Enabled_0_wan")))
	{
		if(radvdCfgParam.interface.prefix[0].enabled)
			req_format_write(wp,"checked");
	}
	else if(!strcmp(name,("radvdprefix0_1_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[0]);
	 }
	else if(!strcmp(name,("radvdprefix0_2_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[1]);
	 }
	else if(!strcmp(name,("radvdprefix0_3_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[2]);
	 }
	else if(!strcmp(name,("radvdprefix0_4_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[3]);
	 }
	else if(!strcmp(name,("radvdprefix0_5_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[4]);
	 }
	else if(!strcmp(name,("radvdprefix0_6_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[5]);
	 }
	else if(!strcmp(name,("radvdprefix0_7_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[6]);
	 }
	else if(!strcmp(name,("radvdprefix0_8_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[7]);
	 }
	else if(!strcmp(name,("radvdprefix0_len_wan")))
	 {
		req_format_write(wp,"%d",radvdCfgParam.interface.prefix[0].PrefixLen);
	 }


	else if(!strcmp(name,("AdvOnLinkFlag_0_wan")))
	 {
		   if(radvdCfgParam.interface.prefix[0].AdvOnLinkFlag)
				req_format_write(wp,"checked");
	 }
	
	else if(!strcmp(name,("AdvAutonomousFlag_0_wan")))
	 {
		 if(radvdCfgParam.interface.prefix[0].AdvAutonomousFlag)
				req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("AdvValidLifetime_0_wan")))
	 {
		req_format_write(wp,"%u",radvdCfgParam.interface.prefix[0].AdvValidLifetime);
	 }
	else if(!strcmp(name,("AdvPreferredLifetime_0_wan")))
	 {
		req_format_write(wp,"%u",radvdCfgParam.interface.prefix[0].AdvPreferredLifetime);
	 }
	else if(!strcmp(name,("AdvRouterAddr_0_wan")))
	 {
		if(radvdCfgParam.interface.prefix[0].AdvRouterAddr)
				req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("if6to4_0_wan")))
	 {
		req_format_write(wp,"%s",radvdCfgParam.interface.prefix[0].if6to4);
	 }
	  
	 /*prefix1*/
	else if(!strcmp(name,("Enabled_1_wan")))
	{
		if(radvdCfgParam.interface.prefix[1].enabled)
			req_format_write(wp,"checked");
	}
	  else	if(!strcmp(name,("radvdprefix1_1_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[0]);
	 }
	else if(!strcmp(name,("radvdprefix1_2_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[1]);
	 }
	else if(!strcmp(name,("radvdprefix1_3_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[2]);
	 }
	else if(!strcmp(name,("radvdprefix1_4_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[3]);
	 }
	else if(!strcmp(name,("radvdprefix1_5_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[4]);
	 }
	else if(!strcmp(name,("radvdprefix1_6_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[5]);
	 }
	else if(!strcmp(name,("radvdprefix1_7_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[6]);
	 }
	else if(!strcmp(name,("radvdprefix1_8_wan")))
	 {
		req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[7]);
	 }
	else if(!strcmp(name,("radvdprefix1_len_wan")))
	 {
		req_format_write(wp,"%d",radvdCfgParam.interface.prefix[1].PrefixLen);
	 }
	else if(!strcmp(name,("AdvOnLinkFlag_1_wan")))
	 {
		if(radvdCfgParam.interface.prefix[1].AdvOnLinkFlag)
			req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("AdvAutonomousFlag_1_wan")))
	 {
		 if(radvdCfgParam.interface.prefix[1].AdvAutonomousFlag)
			req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("AdvValidLifetime_1_wan")))
	 {
		req_format_write(wp,"%u",radvdCfgParam.interface.prefix[1].AdvValidLifetime);
	 }
	else if(!strcmp(name,("AdvPreferredLifetime_1_wan")))
	 {
		req_format_write(wp,"%u",radvdCfgParam.interface.prefix[1].AdvPreferredLifetime);
	 }
	else if(!strcmp(name,("AdvRouterAddr_1_wan")))
	 {
		 if(radvdCfgParam.interface.prefix[1].AdvRouterAddr)
			req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("if6to4_1_wan")))
	 {
		req_format_write(wp,"%s",radvdCfgParam.interface.prefix[1].if6to4);
	 }
#endif
////////////dnsmasq///////////////////////////////////////
// TODO: dns v6
#if 0	
	if(getDnsv6Info(&dnsv6CfgParam)<0)
	{
		req_format_write(wp,"Read Dnsmasq Configuration Error");
		return -1;
	}
	 if(!strcmp(name,("enable_dnsv6")))
        {
        	if(dnsv6CfgParam.enabled)
        		req_format_write(wp,"checked");
        }
        else if(!strcmp(name,("routername")))
        {
        	req_format_write(wp,"%s",dnsv6CfgParam.routerName);
        }
#endif		
///////////////DHCPv6 server//////////////////////////////////////
// TODO: dhcp v6
#if 0	
	if(getDhcpv6sInfo(&dhcp6sCfgParam)<0)
	{
		req_format_write(wp,"Read Dnsmasq Configuration Error");
		return -1;
	}	
	
	if(!strcmp(name,("enable_dhcpv6s")))
      {
      		if(dhcp6sCfgParam.enabled)
      			req_format_write(wp,"checked");
      }
	
      else if(!strcmp(name,("interfacenameds")))
      {
      		req_format_write(wp,"%s",dhcp6sCfgParam.interfaceNameds);
      }
	else if(!strcmp(name,("dnsaddr")))
      {
      		req_format_write(wp,"%s",dhcp6sCfgParam.DNSaddr6);
      }	
	else if(!strcmp(name,("addrPoolStart")))
      {
      		req_format_write(wp,"%s",dhcp6sCfgParam.addr6PoolS);
      }	
	else if(!strcmp(name,("addrPoolEnd")))
      {
      		req_format_write(wp,"%s",dhcp6sCfgParam.addr6PoolE);
      }	
#endif
	
	///////////////Tunnel//////////////////////////////////////
// TODO: tunnel6	
#if 0	
	if(!strcmp(name,("enable_tunnel6")))
      {
      		if(getTunnel6Info(&tunnelCfgParam)<0)
		{
			req_format_write(wp,"Read Tunnel Configuration Error");
			return -1;
		}	
      		if(tunnelCfgParam.enabled)
	      		req_format_write(wp,"checked");
      }	
#endif

	return 0;
}

// TODO: ipv6 wan
#if 0
uint32 getIPv6WanInfo(request *wp, int argc, char **argv)
{
	char	*name;
	addr6CfgParam_t	addr6_wan;
	addr6CfgParam_t	addr6_lan;
	addr6CfgParam_t addr6_gw;
	addr6CfgParam_t addr6_dns;
	addr6CfgParam_t addr6_prefix;
	dhcp6cCfgParam_t dhcp6cCfgParam;
	int val;

	name = argv[0];
	if (name == NULL) {
   		fprintf(stderr, "Insufficient args\n");
   		return -1;
   	}

	/*lan ipv6 address*/
	if(getLanAddv6Info(&addr6_lan)<0){
		fprintf(stderr, "Read lan addr6 mib Error\n");		
		return -1;
	}
	if(!strcmp(name,"lan_ipv6Addr_0")){
		req_format_write(wp,"%04x",addr6_lan.addrIPv6[0]);
	}
	else if(!strcmp(name,"lan_ipv6Addr_1")){
		req_format_write(wp,"%04x",addr6_lan.addrIPv6[1]);
	}
	else if(!strcmp(name,"lan_ipv6Addr_2")){
		req_format_write(wp,"%04x",addr6_lan.addrIPv6[2]);
	}
	else if(!strcmp(name,"lan_ipv6Addr_3")){
		req_format_write(wp,"%04x",addr6_lan.addrIPv6[3]);
	}
	else if(!strcmp(name,"lan_ipv6Addr_4")){
		req_format_write(wp,"%04x",addr6_lan.addrIPv6[4]);
	}
	else if(!strcmp(name,"lan_ipv6Addr_5")){
		req_format_write(wp,"%04x",addr6_lan.addrIPv6[5]);
	}
	else if(!strcmp(name,"lan_ipv6Addr_6")){
		req_format_write(wp,"%04x",addr6_lan.addrIPv6[6]);
	}
	else if(!strcmp(name,"lan_ipv6Addr_7")){
		req_format_write(wp,"%04x",addr6_lan.addrIPv6[7]);
	}
	else if(!strcmp(name,"lan_ipv6Addr_prefixLen")){
		req_format_write(wp,"%d",addr6_lan.prefix_len);
	}

	if(getDhcpv6cInfo(&dhcp6cCfgParam)<0)
	{
		fprintf(stderr, "Read dhcp6c mib Error\n");		
		//req_format_write(wp,"Read dhcp6c mib Error");
		return -1;
	}

	if(!strcmp(name,("enable_dhcpv6pd")))
    {
      	if(dhcp6cCfgParam.enabled)
      		req_format_write(wp,"true");
		else
			req_format_write(wp,"false");
    }
	else if(!strcmp(name,("interfacename_pd")))
    {
      		req_format_write(wp,"%s",dhcp6cCfgParam.dhcp6pd.ifName);
    }	
	else if(!strcmp(name,("sla_len")))
    {
      		req_format_write(wp,"%d",dhcp6cCfgParam.dhcp6pd.sla_len);
    }
	else if(!strcmp(name,("sla_id")))
    {
      		req_format_write(wp,"%d",dhcp6cCfgParam.dhcp6pd.sla_id);
    }

	if(!strcmp(name,"ipv6WanEnabled")){
		if(!apmib_get(MIB_IPV6_WAN_ENABLE,&val)){
			fprintf(stderr, "Read MIB_IPV6_WAN_ENABLE Error\n");	
			return -1;			
		}
		else{
			req_format_write(wp,"%d",val);
		}
	}
	
	if(!strcmp(name,"ipv6LinkType")){
		if(!apmib_get(MIB_IPV6_LINK_TYPE,&val)){
			fprintf(stderr, "Read MIB_IPV6_LINK_TYPE Error\n");
			return -1;			
		}
		else{
			req_format_write(wp,"%d",val);
		}
	}
	
	if(!strcmp(name,"ipv6Origin")){
		if(!apmib_get(MIB_IPV6_ORIGIN_TYPE,&val)){
			fprintf(stderr, "Read MIB_IPV6_ORIGIN_TYPE Error\n");
			return -1;			
		}
		else{
			req_format_write(wp,"%d",val);
		}
	}

	if(!strcmp(name,"wan_ipv6DnsAuto")){
		if(!apmib_get(MIB_IPV6_DNS_AUTO,&val)){
			fprintf(stderr, "Read MIB_IPV6_DNS_AUTO Error\n");
			return -1;			
		}
		else{
			req_format_write(wp,"%d",val);
		}
	}

	if(!strcmp(name,"mldproxyDisabled")){
		if(!apmib_get(MIB_MLD_PROXY_DISABLED,&val)){
			fprintf(stderr, "Read MIB_MLD_PROXY_DISABLED Error\n");
			return -1;			
		}
		else{
			req_format_write(wp,"%d",val);
		}
	}
	
	/*wan ipv6 address*/
	if(getWanAdd6Info(&addr6_wan)<0){
		fprintf(stderr, "Read wan addr6 mib Error\n");		
		return -1;
	}
	if(!strcmp(name,"wan_ipv6Addr_0")){
		req_format_write(wp,"%04x",addr6_wan.addrIPv6[0]);
	}
	else if(!strcmp(name,"wan_ipv6Addr_1")){
		req_format_write(wp,"%04x",addr6_wan.addrIPv6[1]);
	}
	else if(!strcmp(name,"wan_ipv6Addr_2")){
		req_format_write(wp,"%04x",addr6_wan.addrIPv6[2]);
	}
	else if(!strcmp(name,"wan_ipv6Addr_3")){
		req_format_write(wp,"%04x",addr6_wan.addrIPv6[3]);
	}
	else if(!strcmp(name,"wan_ipv6Addr_4")){
		req_format_write(wp,"%04x",addr6_wan.addrIPv6[4]);
	}
	else if(!strcmp(name,"wan_ipv6Addr_5")){
		req_format_write(wp,"%04x",addr6_wan.addrIPv6[5]);
	}
	else if(!strcmp(name,"wan_ipv6Addr_6")){
		req_format_write(wp,"%04x",addr6_wan.addrIPv6[6]);
	}
	else if(!strcmp(name,"wan_ipv6Addr_7")){
		req_format_write(wp,"%04x",addr6_wan.addrIPv6[7]);
	}
	else if(!strcmp(name,"wan_ipv6Addr_prefixLen")){
		req_format_write(wp,"%d",addr6_wan.prefix_len);
	}

	/*gateway*/
	if(getGatewayv6Info(&addr6_gw)<0){
		fprintf(stderr, "Read gw addr6 mib Error\n");
		return -1;
	}
	if(!strcmp(name,"wan_ipv6Gw_0")){
		req_format_write(wp,"%04x",addr6_gw.addrIPv6[0]);
	}
	else if(!strcmp(name,"wan_ipv6Gw_1")){
		req_format_write(wp,"%04x",addr6_gw.addrIPv6[1]);
	}
	else if(!strcmp(name,"wan_ipv6Gw_2")){
		req_format_write(wp,"%04x",addr6_gw.addrIPv6[2]);
	}
	else if(!strcmp(name,"wan_ipv6Gw_3")){
		req_format_write(wp,"%04x",addr6_gw.addrIPv6[3]);
	}
	else if(!strcmp(name,"wan_ipv6Gw_4")){
		req_format_write(wp,"%04x",addr6_gw.addrIPv6[4]);
	}
	else if(!strcmp(name,"wan_ipv6Gw_5")){
		req_format_write(wp,"%04x",addr6_gw.addrIPv6[5]);
	}
	else if(!strcmp(name,"wan_ipv6Gw_6")){
		req_format_write(wp,"%04x",addr6_gw.addrIPv6[6]);
	}
	else if(!strcmp(name,"wan_ipv6Gw_7")){
		req_format_write(wp,"%04x",addr6_gw.addrIPv6[7]);
	}
	else if(!strcmp(name,"wan_ipv6Gw_prefixLen")){
		req_format_write(wp,"%d",addr6_gw.prefix_len);
	}

	/*dns*/
	if(getDnsAddv6Info(&addr6_dns)<0){
		fprintf(stderr, "Read dns addr6 mib Error\n");
		return -1;
	}
	if(!strcmp(name,"wan_ipv6Dns1_0")){
		req_format_write(wp,"%04x",addr6_dns.addrIPv6[0]);
	}
	else if(!strcmp(name,"wan_ipv6Dns1_1")){
		req_format_write(wp,"%04x",addr6_dns.addrIPv6[1]);
	}
	else if(!strcmp(name,"wan_ipv6Dns1_2")){
		req_format_write(wp,"%04x",addr6_dns.addrIPv6[2]);
	}
	else if(!strcmp(name,"wan_ipv6Dns1_3")){
		req_format_write(wp,"%04x",addr6_dns.addrIPv6[3]);
	}
	else if(!strcmp(name,"wan_ipv6Dns1_4")){
		req_format_write(wp,"%04x",addr6_dns.addrIPv6[4]);
	}
	else if(!strcmp(name,"wan_ipv6Dns1_5")){
		req_format_write(wp,"%04x",addr6_dns.addrIPv6[5]);
	}
	else if(!strcmp(name,"wan_ipv6Dns1_6")){
		req_format_write(wp,"%04x",addr6_dns.addrIPv6[6]);
	}
	else if(!strcmp(name,"wan_ipv6Dns1_7")){
		req_format_write(wp,"%04x",addr6_dns.addrIPv6[7]);
	}
	else if(!strcmp(name,"wan_ipv6Dns1_prefixLen")){
		req_format_write(wp,"%d",addr6_dns.prefix_len);
	}

	/*prefix*/
	if(getLanv6PrefixInfo(&addr6_prefix)<0){
		fprintf(stderr, "Read Lan addr6 mib Error\n");
		return -1;
	}
	if(!strcmp(name,"child_prefixAddr_0")){
		req_format_write(wp,"%04x",addr6_prefix.addrIPv6[0]);
	}
	else if(!strcmp(name,"child_prefixAddr_1")){
		req_format_write(wp,"%04x",addr6_prefix.addrIPv6[1]);
	}
	else if(!strcmp(name,"child_prefixAddr_2")){
		req_format_write(wp,"%04x",addr6_prefix.addrIPv6[2]);
	}
	else if(!strcmp(name,"child_prefixAddr_3")){
		req_format_write(wp,"%04x",addr6_prefix.addrIPv6[3]);
	}
	else if(!strcmp(name,"child_prefixAddr_4")){
		req_format_write(wp,"%04x",addr6_prefix.addrIPv6[4]);
	}
	else if(!strcmp(name,"child_prefixAddr_5")){
		req_format_write(wp,"%04x",addr6_prefix.addrIPv6[5]);
	}
	else if(!strcmp(name,"child_prefixAddr_6")){
		req_format_write(wp,"%04x",addr6_prefix.addrIPv6[6]);
	}
	else if(!strcmp(name,"child_prefixAddr_7")){
		req_format_write(wp,"%04x",addr6_prefix.addrIPv6[7]);
	}
	else if(!strcmp(name,"prefix_len_childPrefixAddr")){
		req_format_write(wp,"%d",addr6_prefix.prefix_len);
	}
	
	return 0;
}
#endif

// TODO: dhcp v6
#if 0
void checkDhcp6pd();
void radvd_reconfig();
void checkDnsv6();

struct dhcp6_pd_t {		/* IA_PA */
	uint32 pltime;
	uint32 vltime;
	uint16 addr6[8];
	int plen;
	uint8 flag;
};

struct dhcp6_pd_t dhcp6_pd;
char dns_addr6[64];
void checkDhcp6pd()
{
	FILE *fp;
	uint32 pltime;
	uint32 vltime;
	char addr6[64];
	uint8	prefix[16];
	int plen;
	
	fp=fopen(DHCP6PD_CONF_FILE, "r");
	if(fp==NULL)
		return;
	fscanf(fp,"%s %d %u %u",addr6,&plen,&pltime,&vltime);
	fclose(fp);
	sscanf(addr6,"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
			&prefix[0],&prefix[1],&prefix[2],&prefix[3],
			&prefix[4],&prefix[5],&prefix[6],&prefix[7],
			&prefix[8],&prefix[9],&prefix[10],&prefix[11],
			&prefix[12],&prefix[13],&prefix[14],&prefix[15]);
	if(memcmp(prefix,dhcp6_pd.addr6,16)){
		memcpy(dhcp6_pd.addr6,prefix,16);
		dhcp6_pd.plen=plen;
		dhcp6_pd.pltime=pltime;
		dhcp6_pd.vltime=vltime;
		dhcp6_pd.flag=1;
		radvd_reconfig();		
	}
	return;
}
#endif

// TODO: dns v6
#if 0
void checkDnsv6()
{
		dhcp6sCfgParam_t dhcp6sCfgParam;
		char dns_server[64];
		int pid=-1;
		FILE *fp;
		char prefix[64];
		int flag=1;
		
		if ( !apmib_get(MIB_IPV6_DHCPV6S_PARAM,(void *)&dhcp6sCfgParam)){
			fprintf(stderr,"get MIB_IPV6_DHCPV6S_PARAM failed\n");
			return;  
		}

		fp=fopen(DNSV6_ADDR_FILE,"r");
		if(fp==NULL)
			return;
		memset(dns_server,0,64);
		fscanf(fp,"%s",dns_server);
		fclose(fp);
		
		/*same dns return*/
		if(!strcmp(dns_server,dns_addr6))
			return;
		
		fp=fopen(DHCP6S_CONF_FILE,"w+");
		if(fp==NULL)
			return;
		if(dhcp6sCfgParam.enabled){			
			strcpy(dns_addr6,dns_server);		
			fprintf(fp, "option domain-name-servers %s;\n", dns_server);
			fprintf(fp, "interface %s {\n", dhcp6sCfgParam.interfaceNameds);
			fprintf(fp, "  address-pool pool1 3600;\n");
			fprintf(fp, "};\n");
			fprintf(fp, "pool pool1 {\n");
			fprintf(fp, "  range %s to %s ;\n", dhcp6sCfgParam.addr6PoolS, dhcp6sCfgParam.addr6PoolE);
			fprintf(fp, "};\n");	
			flag=1;
		}
		else{
			if(dhcp6_pd.flag){
				strcpy(dns_addr6,dns_server);
				fprintf(fp, "option domain-name-servers %s;\n", dns_server);
				fprintf(fp, "interface br0 {\n");
				fprintf(fp, "  address-pool pool1 3600;\n");
				fprintf(fp, "};\n");
				fprintf(fp, "pool pool1 {\n");
				sprintf(dhcp6sCfgParam.addr6PoolS,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
				dhcp6_pd.addr6[0],dhcp6_pd.addr6[1],
				dhcp6_pd.addr6[2],dhcp6_pd.addr6[3],
				dhcp6_pd.addr6[4],dhcp6_pd.addr6[5],
				dhcp6_pd.addr6[6],dhcp6_pd.addr6[7]+1);
				sprintf(dhcp6sCfgParam.addr6PoolE,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
				dhcp6_pd.addr6[0],dhcp6_pd.addr6[1],
				dhcp6_pd.addr6[2],dhcp6_pd.addr6[3],
				dhcp6_pd.addr6[4],dhcp6_pd.addr6[5],
				dhcp6_pd.addr6[6],dhcp6_pd.addr6[7]+254);			
				fprintf(fp, "  range %s to %s ;\n", dhcp6sCfgParam.addr6PoolS, dhcp6sCfgParam.addr6PoolE);
				fprintf(fp, "};\n");
				flag=1;
			}
		}
		fclose(fp);
		
		/*start daemon*/
		if(flag){
			if(isFileExist(DHCP6S_PID_FILE)) {
				if ((fp = fopen(DHCP6S_PID_FILE, "r")) != NULL) {
					fscanf(fp, "%d\n", &pid);
					fclose(fp);
					kill(pid,1);	/*sighup radvd to reload config file*/
				}					
			}
			else{
				system("dhcp6s br0 2> /dev/null");
			}
		}
		return;
}
#endif

// TODO: dhcp v6
#if 0
void radvd_reconfig(void)
{
		radvdCfgParam_t radvdCfgParam;
		FILE *fp;
		char tmpBuf[256];
		unsigned short tmpNum[8];
		int pid;
		if ( !apmib_get(MIB_IPV6_RADVD_PARAM,(void *)&radvdCfgParam)){
			fprintf(stderr,"get MIB_IPV6_RADVD_PARAM failed\n");
			return;  
		}

		fp=fopen(RADVD_CONF_FILE,"w+");
		if(fp==NULL){
			fprintf(stderr, "Create %s file error!\n", RADVD_CONF_FILE);
			return;
		}
		
		if(radvdCfgParam.enabled){
			fprintf(fp, "interface %s\n", radvdCfgParam.interface.Name);
			fprintf(fp, "{\n");
			fprintf(fp, "AdvSendAdvert on;\n");			
			fprintf(fp, "MaxRtrAdvInterval %d;\n", radvdCfgParam.interface.MaxRtrAdvInterval);
			fprintf(fp, "MinRtrAdvInterval %d;\n", radvdCfgParam.interface.MinRtrAdvInterval);
			fprintf(fp, "MinDelayBetweenRAs %d;\n", radvdCfgParam.interface.MinDelayBetweenRAs);
			if(radvdCfgParam.interface.AdvManagedFlag > 0) {
				fprintf(fp, "AdvManagedFlag on;\n");					
			}
			if(radvdCfgParam.interface.AdvOtherConfigFlag > 0){
				fprintf(fp, "AdvOtherConfigFlag on;\n");				
			}
			fprintf(fp, "AdvLinkMTU %d;\n", radvdCfgParam.interface.AdvLinkMTU);
			fprintf(fp, "AdvReachableTime %u;\n", radvdCfgParam.interface.AdvReachableTime);
			fprintf(fp, "AdvRetransTimer %u;\n", radvdCfgParam.interface.AdvRetransTimer);
			fprintf(fp, "AdvCurHopLimit %d;\n", radvdCfgParam.interface.AdvCurHopLimit);
			fprintf(fp, "AdvDefaultLifetime %d;\n", radvdCfgParam.interface.AdvDefaultLifetime);			
			fprintf(fp, "AdvDefaultPreference %s;\n", radvdCfgParam.interface.AdvDefaultPreference);		
			if(radvdCfgParam.interface.AdvSourceLLAddress > 0) {
				fprintf(fp, "AdvSourceLLAddress on;\n");				
			}		
			if(radvdCfgParam.interface.UnicastOnly > 0){
				fprintf(fp, "UnicastOnly on;\n");			
			}

			if(radvdCfgParam.interface.prefix[0].enabled > 0){
				memcpy(tmpNum,radvdCfgParam.interface.prefix[0].Prefix, sizeof(radvdCfgParam.interface.prefix[0].Prefix));
				sprintf(tmpBuf, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", tmpNum[0], tmpNum[1], 
					tmpNum[2], tmpNum[3], tmpNum[4], tmpNum[5],tmpNum[6],tmpNum[7]);
				strcat(tmpBuf, "\0");
				fprintf(fp, "prefix %s/%d\n", tmpBuf, radvdCfgParam.interface.prefix[0].PrefixLen);				
				fprintf(fp, "{\n");				
				if(radvdCfgParam.interface.prefix[0].AdvOnLinkFlag > 0){
					fprintf(fp, "AdvOnLink on;\n");							
				}
				if(radvdCfgParam.interface.prefix[0].AdvAutonomousFlag > 0){
					fprintf(fp, "AdvAutonomous on;\n");					
				}
				fprintf(fp, "AdvValidLifetime %u;\n", radvdCfgParam.interface.prefix[0].AdvValidLifetime);
				fprintf(fp, "AdvPreferredLifetime %u;\n", radvdCfgParam.interface.prefix[0].AdvPreferredLifetime);
				
				if(radvdCfgParam.interface.prefix[0].AdvRouterAddr > 0){
					fprintf(fp, "AdvRouterAddr on;\n");							
				}
				if(radvdCfgParam.interface.prefix[0].if6to4[0]){
					fprintf(fp, "Base6to4Interface %s\n;", radvdCfgParam.interface.prefix[0].if6to4);									
				}
				fprintf(fp, "};\n");									
			}

			if(radvdCfgParam.interface.prefix[1].enabled > 0){
				memcpy(tmpNum,radvdCfgParam.interface.prefix[1].Prefix, sizeof(radvdCfgParam.interface.prefix[1].Prefix));
				sprintf(tmpBuf, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", tmpNum[0], tmpNum[1], 
					tmpNum[2], tmpNum[3], tmpNum[4], tmpNum[5],tmpNum[6],tmpNum[7]);
				strcat(tmpBuf, "\0");
				fprintf(fp, "prefix %s/%d\n", tmpBuf, radvdCfgParam.interface.prefix[1].PrefixLen);				
				fprintf(fp, "{\n");				
				if(radvdCfgParam.interface.prefix[1].AdvOnLinkFlag > 0){
					fprintf(fp, "AdvOnLink on;\n");							
				}
				if(radvdCfgParam.interface.prefix[1].AdvAutonomousFlag > 0){
					fprintf(fp, "AdvAutonomous on;\n");					
				}
				fprintf(fp, "AdvValidLifetime %u;\n", radvdCfgParam.interface.prefix[1].AdvValidLifetime);
				fprintf(fp, "AdvPreferredLifetime %u;\n", radvdCfgParam.interface.prefix[1].AdvPreferredLifetime);
				
				if(radvdCfgParam.interface.prefix[1].AdvRouterAddr > 0){
					fprintf(fp, "AdvRouterAddr on;\n");							
				}
				if(radvdCfgParam.interface.prefix[1].if6to4[0]){
					fprintf(fp, "Base6to4Interface %s\n;", radvdCfgParam.interface.prefix[1].if6to4);									
				}
				fprintf(fp, "};\n");									
			}			
		}
		else{
			/*create radvd's configure file and set parameters to default value*/
			fprintf(fp, "interface %s\n","br0");
			fprintf(fp, "{\n");
			fprintf(fp, "AdvSendAdvert on;\n");			
			fprintf(fp, "MaxRtrAdvInterval 600;\n");
			fprintf(fp, "MinRtrAdvInterval 198;\n");
			fprintf(fp, "MinDelayBetweenRAs 3;\n");
			fprintf(fp, "AdvLinkMTU 1500;\n");
			fprintf(fp, "AdvCurHopLimit 64;\n");
			fprintf(fp, "AdvDefaultLifetime 1800;\n");			
			fprintf(fp, "AdvDefaultPreference medium;\n");			
		}
		/*add prefix information*/
		sprintf(tmpBuf,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d",
				dhcp6_pd.addr6[0],dhcp6_pd.addr6[1],
				dhcp6_pd.addr6[2],dhcp6_pd.addr6[3],
				dhcp6_pd.addr6[4],dhcp6_pd.addr6[5],
				dhcp6_pd.addr6[6],dhcp6_pd.addr6[7],
				dhcp6_pd.plen);
		fprintf(fp, "prefix %s\n", tmpBuf);				
		fprintf(fp, "{\n");	
		fprintf(fp, "	AdvOnLink on;\n");	
		fprintf(fp, "	AdvAutonomous on;\n");	
		fprintf(fp, "	AdvValidLifetime %u;\n", dhcp6_pd.vltime);
		fprintf(fp, "	AdvPreferredLifetime %u;\n", dhcp6_pd.pltime);
		fprintf(fp, "	AdvRouterAddr on;\n");
		fprintf(fp, "};\n");
			
		fprintf(fp, "};\n");
		fclose(fp);
		
		if(isFileExist(RADVD_PID_FILE)){
			if ((fp = fopen(RADVD_PID_FILE, "r")) != NULL) {
				fscanf(fp, "%d\n", &pid);
				fclose(fp);
				kill(pid,1);	/*sighup radvd to reload config file*/
			}		
		} 
		else{			
			system("radvd -C /var/radvd.conf 2> /dev/null"); 						
		}
		
		return;
}
#endif

int getIPv6Status(int argc, char **argv)
{

	FILE *fp=NULL;
	struct in6_addr	addr6;
	struct in6_addr	src_addr;
	int src_prefix;
	struct in6_addr	dst_addr;
	int dst_prefix;
	struct in6_addr	gw_addr;
	struct in6_addr	any_addr;
	uint32 metrix;
	uint32 ref_cnt;
	uint32 usr_cnt;
	uint32 flag;
	char ifname[IFNAMESIZE];
	char devname[IFNAMESIZE];
	char src[64];
	char dst[64];
	char gw[64];
	int ret;
	char	*name;
	int val;
	int	if_index;
	int	prefix_len;
	int	if_scope;
	int	if_flag;
	
	name = argv[0];
	if (name == NULL) {
   		fprintf(stderr, "Insufficient args\n");
   		return -1;
   	}

	if(!strcmp(name,"ipv6LinkType")){
		if(!apmib_get(MIB_IPV6_LINK_TYPE,&val)){
			fprintf(stderr, "Read MIB_IPV6_LINK_TYPE Error\n");
			return -1;			
		}
		else{
			if(val==IPV6_LINKTYPE_IP)
				req_format_write(wp,"%s","IP link");
			else
				req_format_write(wp,"%s","PPP link");
		}
	}
	
	if(!strcmp(name,"ipv6Origin")){
		if(!apmib_get(MIB_IPV6_ORIGIN_TYPE,&val)){
			fprintf(stderr, "Read MIB_IPV6_ORIGIN_TYPE Error\n");
			return -1;			
		}
		else{
			if(val==IPV6_ORIGIN_AUTO)
				req_format_write(wp,"%s","SLAAC");
			else if(val==IPV6_ORIGIN_DHCP)
				req_format_write(wp,"%s","DHCPv6");
			else
				req_format_write(wp,"%s","STATIC");
		}
	}

	if(!apmib_get(MIB_IPV6_LINK_TYPE,&val)){
		fprintf(stderr, "Read MIB_IPV6_ORIGIN_TYPE Error\n");				
		return -1;			
	}
	
	if(val==IPV6_LINKTYPE_IP)
		sprintf(ifname,"eth1");
	else
		sprintf(ifname,"ppp0");
	
	if(!strcmp(name,"gw_addr6"))
	{		
		fp = fopen(IPV6_ROUTE_PROC,"r");
		if(fp!=NULL){			
			memset(&any_addr,0,sizeof(any_addr));						
			while((ret=fscanf(fp,"%s %x %s %x %s %x %x %x %x %s",
						dst,&dst_prefix,src,&src_prefix,gw,
						&metrix,&ref_cnt,&usr_cnt,&flag,devname))!=EOF){	
				/*return value must be parameter's number*/							
				if(ret!=10){								
					continue;
				}
				/*interface match?*/	
				if(strcmp(ifname,devname)==0){							
					ret=sscanf(dst,"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
								&dst_addr.s6_addr[ 0], &dst_addr.s6_addr[ 1], &dst_addr.s6_addr[ 2], &dst_addr.s6_addr[ 3],
								&dst_addr.s6_addr[ 4], &dst_addr.s6_addr[ 5], &dst_addr.s6_addr[ 6], &dst_addr.s6_addr[ 7],
								&dst_addr.s6_addr[ 8], &dst_addr.s6_addr[ 9], &dst_addr.s6_addr[10], &dst_addr.s6_addr[11],
								&dst_addr.s6_addr[12], &dst_addr.s6_addr[13], &dst_addr.s6_addr[14], &dst_addr.s6_addr[15]);

					/*default route?*/
					if(memcmp(&dst_addr,&any_addr,sizeof(struct in6_addr))==0){
						sscanf(gw,"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
							&gw_addr.s6_addr[ 0], &gw_addr.s6_addr[ 1], &gw_addr.s6_addr[ 2], &gw_addr.s6_addr[ 3],
							&gw_addr.s6_addr[ 4], &gw_addr.s6_addr[ 5], &gw_addr.s6_addr[ 6], &gw_addr.s6_addr[ 7],
							&gw_addr.s6_addr[ 8], &gw_addr.s6_addr[ 9], &gw_addr.s6_addr[10], &gw_addr.s6_addr[11],
							&gw_addr.s6_addr[12], &gw_addr.s6_addr[13], &gw_addr.s6_addr[14], &gw_addr.s6_addr[15]);
						if(gw_addr.s6_addr16[0]==0xFE80){										
							req_format_write(wp,"%s",gw);									
							break;
						}
					}								
				}												
			}
			fclose(fp);			
			return 0;
		}
		else{
			return -1;
		}
	}

	if(!strcmp(name,"wan_addr6_global"))
	{
		fp = fopen(IPV6_ADDR_PROC,"r");
		if(fp!=NULL){
			while((ret=fscanf(fp,"%s %x %x %x %x %s",src,&if_index,
					&prefix_len,&if_scope,&if_flag,devname))!=EOF){
				if(ret!=6)
					continue;
				
				/*interface match?*/	
				if(strcmp(ifname,devname)==0){									
					ret=sscanf(src,"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
								&addr6.s6_addr[ 0], &addr6.s6_addr[ 1], &addr6.s6_addr[ 2], &addr6.s6_addr[ 3],
								&addr6.s6_addr[ 4], &addr6.s6_addr[ 5], &addr6.s6_addr[ 6], &addr6.s6_addr[ 7],
								&addr6.s6_addr[ 8], &addr6.s6_addr[ 9], &addr6.s6_addr[10], &addr6.s6_addr[11],
								&addr6.s6_addr[12], &addr6.s6_addr[13], &addr6.s6_addr[14], &addr6.s6_addr[15]);
					if(addr6.s6_addr16[0]!=0xFE80){
						req_format_write(wp,"%s/%d",src,prefix_len);
						break;
					}
				}
			}
			fclose(fp);
			return 0;
		}
		else{
			return -1;
		}
	}
		
	if(!strcmp(name,"wan_addr6_ll"))
	{
		fp = fopen(IPV6_ADDR_PROC,"r");
		if(fp!=NULL){
			while((ret=fscanf(fp,"%s %x %x %x %x %s",src,&if_index,
				&prefix_len,&if_scope,&if_flag,devname))!=EOF){				
				if(ret!=6)
					continue;
					
				/*interface match?*/	
				if(strcmp(ifname,devname)==0){									
					ret=sscanf(src,"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
								&addr6.s6_addr[ 0], &addr6.s6_addr[ 1], &addr6.s6_addr[ 2], &addr6.s6_addr[ 3],
								&addr6.s6_addr[ 4], &addr6.s6_addr[ 5], &addr6.s6_addr[ 6], &addr6.s6_addr[ 7],
								&addr6.s6_addr[ 8], &addr6.s6_addr[ 9], &addr6.s6_addr[10], &addr6.s6_addr[11],
								&addr6.s6_addr[12], &addr6.s6_addr[13], &addr6.s6_addr[14], &addr6.s6_addr[15]
								);
					if(addr6.s6_addr16[0]==0xFE80){
						req_format_write(wp,"%s/%d",src,prefix_len);
						break;
					}
				}
			}
			fclose(fp);
			return 0;	
		}
		else{
			return -1;
		}
	}
	
	if(!strcmp(name,"lan_addr6_global"))
	{
		fp = fopen(IPV6_ADDR_PROC,"r");
		if(fp!=NULL){
			while((ret=fscanf(fp,"%s %x %x %x %x %s",src,&if_index,
				&prefix_len,&if_scope,&if_flag,devname))!=EOF){
				if(ret!=6)
					continue;
				/*interface match?*/	
				if(strcmp("br0",devname)==0){									
					ret=sscanf(src,"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
								&addr6.s6_addr[ 0], &addr6.s6_addr[ 1], &addr6.s6_addr[ 2], &addr6.s6_addr[ 3],
								&addr6.s6_addr[ 4], &addr6.s6_addr[ 5], &addr6.s6_addr[ 6], &addr6.s6_addr[ 7],
								&addr6.s6_addr[ 8], &addr6.s6_addr[ 9], &addr6.s6_addr[10], &addr6.s6_addr[11],
								&addr6.s6_addr[12], &addr6.s6_addr[13], &addr6.s6_addr[14], &addr6.s6_addr[15]);
					if(addr6.s6_addr16[0]!=0xFE80){
						req_format_write(wp,"%s/%d",src,prefix_len);
						break;
					}
				}
			}
			fclose(fp);
			return 0;	
		}
		else{
			return -1;
		}
	}

	if(!strcmp(name,"lan_addr6_ll"))
	{
		fp = fopen(IPV6_ADDR_PROC,"r");
		if(fp!=NULL){
			while((ret=fscanf(fp,"%s %x %x %x %x %s",src,&if_index,
					&prefix_len,&if_scope,&if_flag,devname))!=EOF){
				if(ret!=6)
					continue;
				
				/*interface match?*/	
				if(strcmp("br0",devname)==0){									
					ret=sscanf(src,"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
								&addr6.s6_addr[ 0], &addr6.s6_addr[ 1], &addr6.s6_addr[ 2], &addr6.s6_addr[ 3],
								&addr6.s6_addr[ 4], &addr6.s6_addr[ 5], &addr6.s6_addr[ 6], &addr6.s6_addr[ 7],
								&addr6.s6_addr[ 8], &addr6.s6_addr[ 9], &addr6.s6_addr[10], &addr6.s6_addr[11],
								&addr6.s6_addr[12], &addr6.s6_addr[13], &addr6.s6_addr[14], &addr6.s6_addr[15]
								);
					if(addr6.s6_addr16[0]==0xFE80){
						req_format_write(wp,"%s/%d",src,prefix_len);
						break;
					}
				}
			}
			fclose(fp);
			return 0;	
		}
		else{
			return -1;
		}
	}

	return 0;
}

int getIPv6BasicInfo(int argc, char **argv)
{
	char	*name;
	addrIPv6CfgParam_t addrIPv6CfgParam;
	
	//printf("get parameter=%s\n", argv[0]);
	name = argv[0];
	if (name == NULL) {
   		fprintf(stderr, "Insufficient args\n");
   		return -1;
   	}

	if(getAddr6Info(&addrIPv6CfgParam)<0)
	{
		req_format_write(wp,"Read Dnsmasq Configuration Error");
		return -1;
	}
	
	if(!strcmp(name,("addrIPv6_1_1")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][0]);
      }
	else if(!strcmp(name,("addrIPv6_1_2")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][1]);
      }
	else if(!strcmp(name,("addrIPv6_1_3")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][2]);
      }
	else if(!strcmp(name,("addrIPv6_1_4")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][3]);
      }
	else if(!strcmp(name,("addrIPv6_1_5")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][4]);
      }
	else if(!strcmp(name,("addrIPv6_1_6")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][5]);
      }
	else if(!strcmp(name,("addrIPv6_1_7")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][6]);
      }
	else if(!strcmp(name,("addrIPv6_1_8")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][7]);
      }
	else if(!strcmp(name,("prefix_len_1")))
      {
		req_format_write(wp,"%d",addrIPv6CfgParam.prefix_len[0]);
      }

	if(!strcmp(name,("addrIPv6_2_1")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][0]);
      }
	else if(!strcmp(name,("addrIPv6_2_2")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][1]);
      }
	else if(!strcmp(name,("addrIPv6_2_3")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][2]);
      }
	else if(!strcmp(name,("addrIPv6_2_4")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][3]);
      }
	else if(!strcmp(name,("addrIPv6_2_5")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][4]);
      }
	else if(!strcmp(name,("addrIPv6_2_6")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][5]);
      }
	else if(!strcmp(name,("addrIPv6_2_7")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][6]);
      }
	else if(!strcmp(name,("addrIPv6_2_8")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][7]);
      }
	else if(!strcmp(name,("prefix_len_2")))
      {
		req_format_write(wp,"%d",addrIPv6CfgParam.prefix_len[1]);
      }
	return 0;
}

#endif
#endif
