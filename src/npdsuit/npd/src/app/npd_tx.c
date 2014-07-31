/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
*npd_tx.c
*
*
*CREATOR:
*	chengjun@autelan.com
*
*DESCRIPTION:
*<some description about this file>
*      In this file The functions prepare for packet Tx from Adapter to 
*      The Virtual Network Interface  .
*
*DATE:
*	11/14/2007	
*04/26/2010              zhengzw@autelan.com          Unifying netif index formate with vlan and port-channel
*06/11/2010              zhengzw@autelan.com          L3 interface supported.
*06 /23/2010             zhengzw@autelan.com          Re-coding the packet tx flow. Make it easy to support new protocols.
*08/09/2010              zhengzw@autelan.com          Distribute packet to the slot whitch including the specified port.
*08/10/2010              zhengzw@autelan.com          LLDP packet supported.
*
*******************************************************************************/
#ifdef __cplusplus
extern "C" 
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd/protocol/intf_api.h"

static char *kapTxTaskName = "VirPktTx";
extern unsigned int arp_drop_enable;



unsigned char *npd_packet_alloc(unsigned int size)
{
	unsigned char *data = NULL;

	data = (unsigned char *)malloc(size + NPD_PKT_RESERVED_LEN);
    if(data == NULL)
    {
		return NULL;
    }
	memset(data, 0, NPD_PKT_RESERVED_LEN);
	return (data + NPD_PKT_RESERVED_LEN);
}

unsigned int npd_packet_free(unsigned char *data)
{
    free((data - NPD_PKT_RESERVED_LEN));
	return NPD_SUCCESS;
}


unsigned long	nam_packet_type_is_Mc
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;

	layer2 = (ether_header_t*)(packetBuff);

	if((layer2->dmac[0] & layer2->dmac[1] & layer2->dmac[2] & layer2->dmac[3] & layer2->dmac[4] & layer2->dmac[5]) 	\
		== 0xff) {
		return NPD_TRUE;
	}
	else if(0x01 & layer2->dmac[0]) {
		return NPD_TRUE;
	}
	else {
		return NPD_FALSE;
	}
}

unsigned long nam_lookup_param_by_vlanid
(
	unsigned int l3_intf,
	unsigned short vlanId,
	unsigned char* pktdata,
	 unsigned int *netif_index
	
)
{
	int ret;

	ret = npd_arp_snooping_lookup_arpinfo(l3_intf, vlanId,pktdata,netif_index);
	if(ARP_RETURN_CODE_SUCCESS == ret)
	{
		return NPD_TRUE;
	}

	return NPD_FALSE;
	
}

unsigned char * nam_get_packet_type_str
(
	enum NAM_PACKET_TYPE_ENT packetType
)
{
	if(packetType > NAM_PACKET_TYPE_OTHER){
		return (unsigned char *)"UNKNOWN";
	}
	
	return namPacketTypeStr[packetType];
}

#ifdef HAVE_CAPWAP_ENGINE 				
static unsigned short SeqNumber = 0;
static unsigned short cwFragmentID = 0;

unsigned short getSeqNum(void)
{
	unsigned short flag = SeqNumber;

	if (++SeqNumber == 4096)
		SeqNumber = 0;

	return flag;
}

unsigned short getCWSeqNum(void)
{
	unsigned short flag = cwFragmentID;

	if (++cwFragmentID == 65535)
		cwFragmentID = 0;
	return flag;
}

unsigned short npd_tx_get_checksum
(
	void *addr,
	unsigned int count
)
{
	/* Compute Internet Checksum for "count" bytes
	*         beginning at location "addr".
	*/
	register int sum = 0;
	unsigned short *source = (unsigned short *)addr;

	while (count > 1)  
	{
		/*  This is the inner loop */
		sum += *source++;
		count -= 2;
	}

	/*  Add left-over byte, if any */
	if (count > 0) 
	{
		/* Make sure that the left-over byte is added correctly both
		* with little and big endian hosts */
		unsigned short tmp = 0;
		*(unsigned char *) (&tmp) = * (unsigned char *) source;
		sum += tmp;
	}
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
	return ~sum;
}

int npd_tx_8023_to_80211
(
	char * pkt, int pktLen,
	char * dot11pkt, int * dot11pktLen,
	unsigned short  seqNum, 
	unsigned char *BSSID,
	unsigned int protect_type
)
{
	int offsetLen = 0;
	unsigned char IEEE8023_destMac[6];
	unsigned char IEEE8023_srcMac[6];
	unsigned char IEEE80211_addr1[6];
	unsigned char IEEE80211_addr2[6];
	unsigned char IEEE80211_addr3[6];
	
	unsigned char IEEE8023_type[2];
	struct ieee80211_frame *macHead;
	struct ieee80211_llc *llcHead;
	unsigned char  fcField[2];
	unsigned char *buff;
	unsigned int macHeadLen = 24;

	buff = pkt + 0;
	
	//TODO: every type of frames , the 802.11 mac header's length is different
	macHead = (struct ieee80211_frame*)(dot11pkt + 0);
	IEEE80211_RETRIEVE(IEEE8023_destMac, buff, IEEE8023_DEST_MAC_START, IEEE8023_MAC_LEN);
	IEEE80211_RETRIEVE(IEEE8023_srcMac, buff , IEEE8023_SRC_MAC_START, IEEE8023_MAC_LEN);
	IEEE80211_RETRIEVE(IEEE8023_type, buff, IEEE8023_TYPE_START, IEEE8023_TYPE_LEN);

	llcHead = (struct ieee80211_llc*)(dot11pkt + 24);
	//Fill frame control field.
	//Should add something about QoS

	/* WEP type */
	if (protect_type)
		{
			macHead->i_fc[0] = 0x08;  /*protocl version 0b;  type 00b; subtype 1000b, WEP set*/
			macHead->i_fc[1] = 0x02;  
		}
	else
		{
			macHead->i_fc[0] = 0x08;  /*protocl version 0b;  type 00b; subtype 1000b */
			macHead->i_fc[1] = 0x00;
		}

	macHead->i_dur[0] = 0;
	macHead->i_dur[1] = 0;
	
	memcpy(macHead->i_addr1, IEEE8023_destMac, MAC_ADDR_LEN);
	memcpy(macHead->i_addr3, IEEE8023_srcMac, MAC_ADDR_LEN);
	memcpy(macHead->i_addr2, BSSID, MAC_ADDR_LEN);
	macHead->i_seq[0] = (seqNum<<4)&0xf0;
	macHead->i_seq[1] = (seqNum>>4)&0xff;

	llcHead->llc_dsap = 0xaa;
	llcHead->llc_ssap = 0xaa;
	llcHead->llc_cmd = 0x03;
	llcHead->llc_org_code[0] = 0;
	llcHead->llc_org_code[1] = 0;
	llcHead->llc_org_code[2] = 0;
	llcHead->llc_ether_type[0] = IEEE8023_type[0];
	llcHead->llc_ether_type[1] = IEEE8023_type[1];

	memcpy(dot11pkt+sizeof(struct ieee80211_frame)+sizeof(struct ieee80211_llc), pkt+sizeof(ETHER_HEADER_T), pktLen-sizeof(ETHER_HEADER_T));
	*dot11pktLen = pktLen - sizeof(ETHER_HEADER_T) + sizeof(struct ieee80211_frame) + sizeof(struct ieee80211_llc);
	npd_syslog_dbg("8023==>802.11: orig len %d, convert len %d", pktLen, *dot11pktLen);
	
	return 0;
}

int npd_tx_assemble_capwap(unsigned char *pdata, unsigned int *len, unsigned int netif_index)
{
	unsigned int val = 0;
	unsigned char *buff = pdata;
	unsigned int radioId = npd_netif_local_radio_id_get(netif_index);
	unsigned int wlanId = npd_netif_wtpid_get(netif_index);
	CW_DATA_HEADER_T cwhead;
	CW_DATA_HEADER_T *valuesPtr = &cwhead;

	*len = 0;
	cwhead.bindingValuesPtr = NULL;
	cwhead.fragmentID = 0; //getCWSeqNum();  this seq is not used.
	cwhead.fragmentOffset = 0;
	cwhead.isFragment = 0;
	cwhead.keepAlive = 0;
	cwhead.last = 0;
	cwhead.payloadType =0;
	cwhead.type = 0;
	
	CWSetField32(val, 
		     CW_TRANSPORT_HEADER_VERSION_START,
		     CW_TRANSPORT_HEADER_VERSION_LEN,
		     CW_PROTOCOL_VERSION); //CAPWAP VERSION

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_TYPE_START,
		     CW_TRANSPORT_HEADER_TYPE_LEN,
		     0);
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_HLEN_START,
		     CW_TRANSPORT_HEADER_HLEN_LEN,
		     2);//Is it right?

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_RID_START,
		     CW_TRANSPORT_HEADER_RID_LEN,
		     radioId); // Radio local id
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_WBID_START,
		     CW_TRANSPORT_HEADER_WBID_LEN,
		     1); // Wireless Binding ID
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_T_START,
		     CW_TRANSPORT_HEADER_T_LEN,
		     1);

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_F_START,
		     CW_TRANSPORT_HEADER_F_LEN,
		     valuesPtr->isFragment); // is fragment

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_L_START,
		     CW_TRANSPORT_HEADER_L_LEN,
		     valuesPtr->last); // last fragment
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_W_START,
		     CW_TRANSPORT_HEADER_W_LEN,
		     0); //have no wireless option header
	

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_M_START,
		     CW_TRANSPORT_HEADER_M_LEN,
		     0); // no radio MAC address

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_K_START,
		     CW_TRANSPORT_HEADER_K_LEN,
		     0); // Keep alive flag

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_FLAGS_START,
		     CW_TRANSPORT_HEADER_FLAGS_LEN,
		     0); // required
	val = htonl(val);
	WSMProtocolStore32(buff, val);
	buff += 4;
	*len += 4;
	// end of first 32 bits
	
	val = 0;
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_FRAGMENT_ID_START,
		     CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN,
		     valuesPtr->fragmentID); // fragment ID
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START,
		     CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN,
		     valuesPtr->fragmentOffset); // fragment offset

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_RESERVED_START,
		     CW_TRANSPORT_HEADER_RESERVED_LEN,
		     0); // required
		     
	val = htonl(val);	     
	WSMProtocolStore32(buff, val);
	buff += 4;
	*len += 4;

#if 0	
	val = 0;
	CWSetField32(val, 0, 8, wlanId);
	val = htonl(val);
	WSMProtocolStore32(buff, val);
	buff += 4;
	*len += 4;
#endif	
	return 0;
}


unsigned int npd_wtp_encap_capwap
(
	unsigned int netif_index, 
	char *data, 
	unsigned int dataLen, 
	char *encap_data, 
	unsigned int *encap_dataLen, 
	unsigned int *eth_netif_index,
	unsigned int *vlan_id
)
{
	int ret = 0;
	int seq = 0; //getSeqNum();   This seq is not used.
	int protect_type = 1; //WEP bit is set.
	unsigned int offset = 0, len = 0, wtp_ipaddr, sourceIp, encap_format;	
	char *buff = NULL, mac[MAC_ADDR_LEN];
	char bssid[MAC_ADDR_LEN];
	unsigned short sPort, dPort;
	ETHER_HEADER_T *ethHdr = NULL;
	IP_HEADER_T *ipHdr = NULL;
	UDP_HEADER_T *udpHdr = NULL;

	
	//fill Eth header
	if( 0 != npd_capwap_get_bssid_by_netif( netif_index, &bssid)){
		npd_syslog_err("Fail to get bssid by netif index:0x%x\r\n", netif_index);
		return -1;
	}
	
	if( 0 != npd_capwap_get_info_by_bssid(bssid, &wtp_ipaddr, &sourceIp, &sPort, &dPort, &encap_format)){
		npd_syslog_err("Fail to get wtp ip, source ip source port dst port by bssid"MACSTR"\r\n", MAC2STR(bssid));
		return -1;
	}

	if( 0 != npd_route_get_egrintf_by_dip(wtp_ipaddr, &mac, eth_netif_index, vlan_id)){
		npd_syslog_err("Fail to get mac address by wtp ip address 0x%x, eth_netif_index:0x%x,vlan_id:%d\r\n", wtp_ipaddr, eth_netif_index, vlan_id);
		return -1;
	}
	
	//802.3==>802.11
	if( encap_format){
		offset = sizeof(ETHER_HEADER_T) + sizeof(IP_HEADER_T) + sizeof(UDP_HEADER_T) + 8; 
		buff = encap_data + offset;
		ret = npd_tx_8023_to_80211(data, dataLen, buff, &len, seq, bssid, protect_type);
		*encap_dataLen += len;
		len = 0;
	}
	
	//802.11==>CAPWAP
	offset = sizeof(ETHER_HEADER_T) + sizeof(IP_HEADER_T) + sizeof(UDP_HEADER_T);
	buff = encap_data + offset;
	npd_tx_assemble_capwap(buff, &len, netif_index);
	*encap_dataLen += len;
	len = 0;
	
	//fill udp header
	offset = sizeof(ETHER_HEADER_T) + sizeof(IP_HEADER_T);
	udpHdr = encap_data + offset;
	udpHdr->dest = htons(dPort);
	udpHdr->source = htons(sPort);
	udpHdr->len = *encap_dataLen + sizeof(UDP_HEADER_T);
	udpHdr->check = 0;
	*encap_dataLen += sizeof(UDP_HEADER_T);

	//fill psuedo IP header
	offset = sizeof(ETHER_HEADER_T);
	ipHdr = encap_data + offset;
	ipHdr->ipProtocol = 0x11;
	ipHdr->dip = htonl(wtp_ipaddr);
	ipHdr->sip = htonl(sourceIp);
	ipHdr->totalLen = udpHdr->len;
	//fill UDP checksum
	udpHdr->check = npd_tx_get_checksum(ipHdr, (udpHdr->len+sizeof(IP_HEADER_T)));
	
	ipHdr->version = htons(4);
	ipHdr->hdrLength = htons(5);
	ipHdr->totalLen = htons(*encap_dataLen) + sizeof(IP_HEADER_T);
	ipHdr->ttl = htons(64);
	ipHdr->ipProtocol = 0x11;
	ipHdr->dip = htonl(wtp_ipaddr);
	ipHdr->sip = htonl(sourceIp);	
	ipHdr->checkSum = npd_tx_get_checksum(ipHdr, sizeof(IP_HEADER_T));
	*encap_dataLen += sizeof(IP_HEADER_T);

	offset = 0;
	ethHdr = encap_data + offset;
	memcpy(ethHdr->dmac,mac, 6);
	npd_system_get_basemac(ethHdr->smac, 6);
	ethHdr->etherType = htons(0x0800);
	*encap_dataLen += sizeof(ETHER_HEADER_T);	

	return 0;							
}



int npd_tx_assemble_802_3_format_capwap_header(unsigned char *pdata, unsigned int *len, unsigned int netif_index,unsigned char *bssid)
{
	unsigned int val = 0;
	unsigned char *buff = pdata;
	unsigned int radioId = npd_netif_local_radio_id_get(netif_index);
	unsigned int wlanId = npd_netif_wtpid_get(netif_index);
	CW_DATA_HEADER_T cwhead;
	CW_DATA_HEADER_T *valuesPtr = &cwhead;

	*len = 0;
	cwhead.bindingValuesPtr = NULL;
	cwhead.fragmentID = 0; //getCWSeqNum();  this seq is not used.
	cwhead.fragmentOffset = 0;
	cwhead.isFragment = 0;
	cwhead.keepAlive = 0;
	cwhead.last = 0;
	cwhead.payloadType =0;
	cwhead.type = 0;
	
	CWSetField32(val, 
		     CW_TRANSPORT_HEADER_VERSION_START,
		     CW_TRANSPORT_HEADER_VERSION_LEN,
		     CW_PROTOCOL_VERSION); //CAPWAP VERSION

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_TYPE_START,
		     CW_TRANSPORT_HEADER_TYPE_LEN,
		     0);
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_HLEN_START,
		     CW_TRANSPORT_HEADER_HLEN_LEN,
		     4);//Is it right?

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_RID_START,
		     CW_TRANSPORT_HEADER_RID_LEN,
		     radioId); // Radio local id
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_WBID_START,
		     CW_TRANSPORT_HEADER_WBID_LEN,
		     1); // Wireless Binding ID
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_T_START,
		     CW_TRANSPORT_HEADER_T_LEN,
		     0);

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_F_START,
		     CW_TRANSPORT_HEADER_F_LEN,
		     valuesPtr->isFragment); // is fragment

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_L_START,
		     CW_TRANSPORT_HEADER_L_LEN,
		     valuesPtr->last); // last fragment
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_W_START,
		     CW_TRANSPORT_HEADER_W_LEN,
		     0); //have no wireless option header
	

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_M_START,
		     CW_TRANSPORT_HEADER_M_LEN,
		     1); // radio MAC address included

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_K_START,
		     CW_TRANSPORT_HEADER_K_LEN,
		     0); // Keep alive flag

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_FLAGS_START,
		     CW_TRANSPORT_HEADER_FLAGS_LEN,
		     0); // required
	val = htonl(val);
	WSMProtocolStore32(buff, val);
	buff += 4;
	*len += 4;
	// end of first 32 bits
	
	val = 0;
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_FRAGMENT_ID_START,
		     CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN,
		     valuesPtr->fragmentID); // fragment ID
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START,
		     CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN,
		     valuesPtr->fragmentOffset); // fragment offset

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_RESERVED_START,
		     CW_TRANSPORT_HEADER_RESERVED_LEN,
		     0); // required
		     
	val = htonl(val);	     
	WSMProtocolStore32(buff, val);
	buff += 4;
	*len += 4;

	/*Radio MAC Address Length*/
	val = 0;
	CWSetField32(val, 0, 8, MAC_ADDR_LEN);
	
	/*First three bytes Radio MAC Address*/
	CWSetField32(val, 8, 8, bssid[0]);
	CWSetField32(val, 16, 8, bssid[1]);
	CWSetField32(val, 24, 8, bssid[2]);
	val = htonl(val);
	WSMProtocolStore32(buff, val);
	buff += 4;
	*len += 4;

	/*Last three bytes Radio MAC Address*/
	val = 0;
	CWSetField32(val, 0, 8, bssid[3]);
	CWSetField32(val, 8, 8, bssid[4]);
	CWSetField32(val, 16, 8, bssid[5]);
	/*Padding byte*/
	CWSetField32(val, 
				CW_TRANSPORT_HEADER_RADIO_MAC_ADDR_PADDING_START,
				CW_TRANSPORT_HEADER_RADIO_MAC_ADDR_PADDING_LEN, 
				0);
	val = htonl(val);
	WSMProtocolStore32(buff, val);
	buff += 4;
	*len += 4;
	
	return 0;
}

#define NPD_802_3_FORMAT_CAPWAP_HEADER_LEN			16
#define NPD_802_11_FORMAT_CAPWAP_HEADER_LEN			8

unsigned int npd_wtp_encap_802_3_format_capwap
(
	unsigned int netif_index, 
	char *data, 
	unsigned int dataLen, 
	char *encap_data, 
	unsigned int *encap_dataLen, 
	unsigned int *eth_netif_index,
	unsigned int *vlan_id
)
{
	int ret = 0;
	int seq = 0; //getSeqNum();   This seq is not used.
	int protect_type = 1; //WEP bit is set.
	unsigned int offset = 0, len = 0, wtp_ipaddr, sourceIp, encap_format;	
	char *buff = NULL, mac[MAC_ADDR_LEN];
	char bssid[MAC_ADDR_LEN];
	unsigned short sPort, dPort;
	ETHER_HEADER_T *ethHdr = NULL;
	IP_HEADER_T *ipHdr = NULL;
	UDP_HEADER_T *udpHdr = NULL;

	
	//fill Eth header
	if( 0 != npd_capwap_get_bssid_by_netif( netif_index, &bssid)){
		npd_syslog_err("Fail to get bssid by netif index:0x%x\r\n", netif_index);
		return -1;
	}
	
	if( 0 != npd_capwap_get_info_by_bssid(bssid, &wtp_ipaddr, &sourceIp, &sPort, &dPort, &encap_format)){
		npd_syslog_err("Fail to get wtp ip, source ip source port dst port by bssid"MACSTR"\r\n", MAC2STR(bssid));
		return -1;
	}

	if( 0 != npd_route_get_egrintf_by_dip(wtp_ipaddr, &mac, eth_netif_index, vlan_id)){
		npd_syslog_err("Fail to get mac address by wtp ip address 0x%x, eth_netif_index:0x%x,vlan_id:%d\r\n", wtp_ipaddr, eth_netif_index, vlan_id);
		return -1;
	}	
	
	//copy data
	offset = sizeof(ETHER_HEADER_T) + sizeof(IP_HEADER_T) + sizeof(UDP_HEADER_T) + NPD_802_3_FORMAT_CAPWAP_HEADER_LEN; 
	if((dataLen + offset) >= NAM_SDMA_TX_PER_BUFFER)
	{
		npd_syslog_err("encap packet is larger than DMA TX BUFFER LEN\r\n");
		return -1;
	}
	
	buff = encap_data + offset;
	memcpy(buff, data, dataLen);
	*encap_dataLen = dataLen;
	len = 0;
	
	//802.11==>CAPWAP
	offset = sizeof(ETHER_HEADER_T) + sizeof(IP_HEADER_T) + sizeof(UDP_HEADER_T);
	buff = encap_data + offset;
	npd_tx_assemble_802_3_format_capwap_header(buff, &len, netif_index,bssid);
	*encap_dataLen += len;
	len = 0;
	
	//fill udp header
	offset = sizeof(ETHER_HEADER_T) + sizeof(IP_HEADER_T);
	udpHdr = encap_data + offset;
	udpHdr->dest = htons(dPort);
	udpHdr->source = htons(sPort);
	udpHdr->len = *encap_dataLen + sizeof(UDP_HEADER_T);
	udpHdr->check = 0;
	*encap_dataLen += sizeof(UDP_HEADER_T);

	//fill psuedo IP header
	offset = sizeof(ETHER_HEADER_T);
	ipHdr = encap_data + offset;
	ipHdr->ipProtocol = 0x11;
	ipHdr->dip = htonl(wtp_ipaddr);
	ipHdr->sip = htonl(sourceIp);
	ipHdr->totalLen = udpHdr->len;
	//fill UDP checksum
	udpHdr->check = npd_tx_get_checksum(ipHdr, (udpHdr->len+sizeof(IP_HEADER_T)));
	
	ipHdr->version = htons(4);
	ipHdr->hdrLength = htons(5);
	ipHdr->totalLen = htons(*encap_dataLen) + sizeof(IP_HEADER_T);
	ipHdr->ttl = htons(64);
	ipHdr->ipProtocol = 0x11;
	ipHdr->dip = htonl(wtp_ipaddr);
	ipHdr->sip = htonl(sourceIp);	
	ipHdr->checkSum = npd_tx_get_checksum(ipHdr, sizeof(IP_HEADER_T));
	*encap_dataLen += sizeof(IP_HEADER_T);

	offset = 0;
	ethHdr = encap_data + offset;
	memcpy(ethHdr->dmac,mac, 6);
	npd_system_get_basemac(ethHdr->smac, 6);
	ethHdr->etherType = htons(0x0800);
	*encap_dataLen += sizeof(ETHER_HEADER_T);	

	return 0;							
}

#endif //HAVE_CAPWAP_ENGINE

struct list_head tx_hook_handle_head =
{
    &tx_hook_handle_head,
    &tx_hook_handle_head
};

int npd_packet_tx_hook_register
(
int type, 
char *desc, 
int flags, 
int (*protocol_filter)(unsigned char  *packetBuff), 
int (*protocol_handler)(int packet_type,
                        unsigned char *packetBuffs, 
		                unsigned long buffLen, 
		                unsigned int netif_index,
		                unsigned int son_netif_index,
		                unsigned short vid,
		                unsigned char isTagged,
		                int flag
                        )
)
{
	return npd_packet_handle_register(&tx_hook_handle_head,
												type, desc, flags,			
												protocol_filter,
												protocol_handler);	
}

int npd_packet_tx_hook_sub_register
(
int father_type,
int type, 
char *desc, 
int flags, 
int (*protocol_filter)(unsigned char  *packetBuff), 
int (*protocol_handler)(int packet_type,
						unsigned char *packetBuffs, 
		                unsigned long buffLen, 
		                unsigned int netif_index,
		                unsigned int son_netif_index,
		                unsigned short vid,
		                unsigned char isTagged,
		                int flag
                        )
)
{
	return npd_packet_sub_handle_register(&tx_hook_handle_head,
										father_type, type,
										desc, flags,
										protocol_filter,
										protocol_handler);	
}

int npd_thread_packet_tx_list_handler(
    struct list_head *protocol_list, 
    unsigned char *packetBuffs, 
    unsigned long buffLen, 
    unsigned int netif_index,
    unsigned int l3intf,
    unsigned short vid,
    unsigned char isTagged)
{
	int ret = 0;
    struct list_head *current_list = NULL;
    struct list_head *pos = NULL;
    protocol_handle *protocol_handle_s = NULL;
	
    list_for_each(pos, protocol_list)
    {
        protocol_handle_s = list_entry(pos, protocol_handle, list);
        if(protocol_handle_s->protocol_filter)
        {
			if((*protocol_handle_s->protocol_filter)(packetBuffs) == NPD_TRUE)
			{
                npd_syslog_dbg(" %s ", protocol_handle_s->desc);
				current_list = &protocol_handle_s->sub_list;
				if(list_empty(current_list))
				{
					if(protocol_handle_s->protocol_handler)
					{
						ret = (*protocol_handle_s->protocol_handler)(protocol_handle_s->type,
																packetBuffs,
																buffLen,
																netif_index,
																l3intf,
																vid,
																isTagged, TRUE);
						return ret;
					}
				}
				else
				{
				    ret = npd_thread_packet_tx_list_handler(current_list, packetBuffs, buffLen, netif_index, l3intf, vid, isTagged);
					if(ret == NAM_PACKET_TX_DROP)
						return ret;
				}
			}
        }
    }
	return ret;
}

unsigned int nam_packet_tx_hook_process
(
	unsigned int netif_index,
	unsigned int l3intf,
	unsigned short vid,
	unsigned char isTagged,
	unsigned char *packetBuff, 
	unsigned int buffLen
)
{	
	return npd_thread_packet_tx_list_handler(&tx_hook_handle_head, packetBuff, buffLen, netif_index, l3intf, vid, isTagged);
}

int npd_packet_tx_hook_ipv4_udp_dhcp
(
	int packet_type,
	unsigned char *packetBuffs, 
	unsigned long buffLen, 
	unsigned int netif_index,
	unsigned int l3intf,
	unsigned short vid,
	unsigned char isTagged,
	int flag
)
{
	int ret = NAM_PACKET_TX_FWD;
#ifdef HAVE_DHCP_SNP
    ret = npd_dhcp_snp_packet_tx_hook(packet_type, 
								packetBuffs, buffLen, netif_index, isTagged, vid);
	if(DHCP_SNP_RETURN_CODE_PKT_DROP == ret)
	{
		return NAM_PACKET_TX_DROP;
	}
	else if(DHCP_SNP_RETURN_CODE_MAC_ILEGAL == ret)
	{
		return NAM_PACKET_TX_BC;
	}
	else
	{
		return NAM_PACKET_TX_FWD;
	}
#endif
	return ret;
}

int npd_packet_tx_hook_arp
(
	int packet_type,
	unsigned char *packetBuffs, 
	unsigned long buffLen, 
	unsigned int netif_index,
	unsigned int l3intf,
	unsigned short vid,
	unsigned char isTagged,
	int flag
)
{
	unsigned int ret = NAM_PACKET_TX_FWD;
	ether_header_t *layer2 = NULL;
	arp_header_t *layer3 = NULL;
	struct arp_snooping_item_s dbItem = {0};
	/* send packet */

	layer2 = (ether_header_t *)packetBuffs; 
    layer3 = (arp_header_t *)(packetBuffs + sizeof(ether_header_t));

    if(layer3->opCode != (unsigned short)htons(1))
    {
        return ret;
    }
    else
    {
        /*增加一个ARP Drop表项，规避IP包攻击*/
        unsigned int dip = *(unsigned int*)layer3->dip;
		
		if (!arp_drop_enable)
			return ret;
		
    	ret = npd_arp_snooping_find_item_byip(dip,&dbItem);
    	if(0 == ret )
    	{
    		return ret;
    	}
    
    	dbItem.ipAddr = dip;
    	dbItem.ifIndex  = 0;/*set netif index to zero in drop item*/
        dbItem.l3Index = l3intf;
    	dbItem.flag |= (ARPSNP_FLAG_DROP | ARPSNP_FLAG_HIT);
    	dbItem.isValid = TRUE;
    	npd_arp_snooping_learning(&dbItem);
    }

	return ret;     
}

int nam_packet_ipv4_from_myself(unsigned char * buff)
{
	ether_header_t  *layer2 = NULL;
	ip_header_t 	*layer3 = NULL;
    unsigned int sip = 0;
	int ifindex = 0, count = 0;

	layer2 = (ether_header_t*)(buff);
	layer3 = (ip_header_t *)(buff + sizeof(ether_header_t));
    sip = *(unsigned int*)layer3->sip;/*always use network order*/
	return npd_intf_addr_ifindex_get(&ifindex, &count, sip);
}

unsigned nam_packet_kap_tx_task
(
	void * dummy
)
{
	int ret = 0;
	unsigned long	i = 0;	/* iterator */
	unsigned char 	devNum = 0, portOrTrunkNum = 0;
	unsigned char		*buffArr[BUFF_LEN] = {NULL};
	unsigned long  bytesNum;
	unsigned long  pktNumber = 0;
	VIRT_PKT_INFO_STC  virPkt;
	unsigned int 		interfaceId = 0 ,l3Intf = 0;
	int tmpNum = 1;
	enum NAM_INTERFACE_TYPE_ENT virRePktType = NAM_INTERFACE_PORT_E;
	enum NAM_PACKET_TYPE_ENT packetType = NAM_PACKET_TYPE_OTHER;
	unsigned long isSend = NPD_FALSE,isMc = NPD_FALSE,isUc = NPD_FALSE,isFund = NPD_FALSE;

	unsigned char isTag = NPD_FALSE;
	/* tell my thread id */
	npd_init_tell_whoami("VirPktTx",0);

	devNum = 0;
	
	memset(&virPkt,0,sizeof(VIRT_PKT_INFO_STC));

	for(i=0;i<BUFF_LEN;i++)
	{
		
		buffArr[i] = npd_packet_alloc(NAM_SDMA_TX_PER_BUFFER);
	    memset(buffArr[i], 0, NAM_SDMA_TX_PER_BUFFER);
	}

	while(1)
	{
		if(ioctl(adptVirRxFd,KAPWAITING,&tmpNum))
		{
			sleep(1);
			continue;
		}
		else if ((pktNumber = ioctl(adptVirRxFd,KAPRETURNPKGNO,&tmpNum)) > 0)
		{
			if(BUFF_LEN < pktNumber)
			{
				npd_syslog_dbg("kap get packet count %d\r\n", pktNumber);				
				pktNumber = BUFF_LEN;
			}

			bytesNum = 0;
			for(i=0;i<pktNumber;i++)
			{
				virPkt.data_addr = (unsigned int)buffArr[i];
				virPkt.data_len = NAM_SDMA_TX_PER_BUFFER;
				npd_syslog_dbg("post-processor %d packet need send,", pktNumber);

				devNum = 0;
			
				bytesNum = read(adptVirRxFd,&virPkt,tmpNum);
				if(bytesNum <= 0)
				{
					npd_syslog_err("read tx buffer error %d\r\n",bytesNum);
					continue;
				}
				else if(bytesNum >0)
				{
            		if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
            		{
            			sleep(1);
            			continue;
            		}
					packetType = nam_packet_parse_txtype((unsigned char *)virPkt.data_addr);
				    if(packetType == NAM_PACKET_TYPE_IPMC_DATA_E)
				    {
				        if(!nam_packet_ipv4_from_myself((unsigned char *)virPkt.data_addr))
						{
						    continue;
						}
				    }
					l3Intf = virPkt.l3_index;
					virRePktType =  virPkt.dev_type;
					switch(virRePktType)
					{
						case NAM_INTERFACE_PORT_E:
							npd_syslog_dbg("outif PORT %#x,",virPkt.l2_index);
							interfaceId = virPkt.l2_index;
							isTag = NPD_FALSE; /*port in 4095 must be untag*/
							isSend = NPD_TRUE;
							isUc = NPD_TRUE;
							break;
							
						case NAM_INTERFACE_TRUNK_E:
							interfaceId = virPkt.l2_index;
							portOrTrunkNum = npd_netif_trunk_get_tid(interfaceId);
							npd_syslog_dbg("unicast %s packet send to trunk(%d,0x%x) in vlan %d\r\n", \
     										nam_get_packet_type_str(packetType),portOrTrunkNum, interfaceId,virPkt.vId);
							isTag = NPD_FALSE; /*port in 4095 must be untag*/
							isSend = NPD_TRUE;
							isUc = NPD_TRUE;
							break;
							
						case NAM_INTERFACE_VIDX_E:
						case NAM_INTERFACE_VID_E:
							npd_syslog_dbg("outif(%d) VLAN (%d),", l3Intf, virPkt.vId);							
							isMc = nam_packet_type_is_Mc((unsigned char*)virPkt.data_addr);							
                            if(!isMc)
							{	
								/*unicast packets*/
								/*此处应该是switch port通用的，不用再区分端口类型*/
								{
    								isFund = nam_lookup_param_by_vlanid(
    															l3Intf, 
    															virPkt.vId,
    															(unsigned char*)virPkt.data_addr,
    															&interfaceId);
    								if(NPD_TRUE == isFund)
    								{
										if(npd_vlan_check_contain_port(virPkt.vId, interfaceId, &isTag) == NPD_TRUE)
										{
    										isUc = NPD_TRUE;
    										isSend = NPD_TRUE;
											npd_syslog_dbg("unicast %s packet send to switch port(0x%x) in vlan %d %s\r\n",	\
    													nam_get_packet_type_str(packetType),interfaceId,virPkt.vId,isTag ? "TAG" : "UNTAG");
										}
										else
										{
    										isUc = NPD_FALSE;
    										isSend = NPD_FALSE;
											npd_syslog_dbg("unicast %s packet drop as switch port(0x%x) not in vlan %d\r\n",	\
        												nam_get_packet_type_str(packetType),interfaceId,virPkt.vId);
										}
									}
    								else
    								{
    									isMc = NPD_TRUE;
    									isSend = NPD_TRUE;
   										npd_syslog_dbg("Out netif is not found.Broadcast %s packet flood in vlan %d\r\n",
        															nam_get_packet_type_str(packetType),virPkt.vId);
    								}
								}
							}
							else 
							{
								isSend = NPD_TRUE;
								isMc = NPD_TRUE;
								npd_syslog_dbg("multicast packet flood in vlan %d\r\n",virPkt.vId);
							}
							break;
						default:
							npd_syslog_dbg("outif %d vlan %d l3if %d l2index %#x %s\r\n",
    											virRePktType,virPkt.vId,l3Intf,virPkt.l2_index, isSend ? "send":"drop");
							break;
					}
                    {
                        unsigned int l3index;
                
                        npd_intf_get_global_l3index(l3Intf, &l3index);
       					ret = nam_packet_tx_hook_process(interfaceId, l3index, virPkt.vId,isTag, (unsigned char *)virPkt.data_addr, virPkt.data_len);	
					}
					if( NAM_PACKET_TX_DROP == ret )
					{
						isSend = NPD_FALSE;isUc = NPD_FALSE;isFund = NPD_FALSE;isMc = NPD_FALSE;
						continue;
					}
					
					if( NAM_PACKET_TX_BC == ret )
					{
						isSend = NPD_TRUE;isUc = NPD_FALSE;isFund = NPD_FALSE;isMc = NPD_TRUE;
					}
					
					/*only send unicast packets*/
					if((NPD_TRUE == isSend) && (NPD_TRUE == isUc))
					{
                        nam_packet_tx_unicast_by_netif(packetType, interfaceId, virPkt.vId, isTag, (unsigned char *)virPkt.data_addr, virPkt.data_len);
						/*go back init status*/
						isSend = NPD_FALSE;isUc = NPD_FALSE;isFund = NPD_FALSE;isMc = NPD_FALSE;
					}/*only send broadcast packets*/
					else if((NPD_TRUE == isSend) && (NPD_TRUE == isMc))
					{
                        nam_packet_tx_broadcast_global(packetType, virPkt.vId, (unsigned char *)virPkt.data_addr, virPkt.data_len);
						isSend = NPD_FALSE;isUc = NPD_FALSE;isFund = NPD_FALSE;isMc = NPD_FALSE;
					}	
					else if(NPD_FALSE == isSend) {
						/*drop packets by zhubo@autelan.com 2008.7.22*/
						isSend = NPD_FALSE;isUc = NPD_FALSE;isFund = NPD_FALSE;isMc = NPD_FALSE;
					}
				}/* end else if(...) */
				bytesNum = 0;
			} /* end for(...) */
		} /* end if(...) */
	} /* end while(..) */	
	return 0;
}		

unsigned long nam_packet_tx_arp_solicit
(
unsigned int netif_index,
unsigned short vid,
unsigned char *smac,
unsigned char *dmac,
unsigned int sip,
unsigned int dip
)
{
	unsigned char bc_mac[MAC_ADDR_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	unsigned char isBC = FALSE;
	unsigned char isTagged = 0;
	unsigned char	*data_buff = NULL;
	ether_header_t *layer2 = NULL;
	arp_header_t *layer3 = NULL;

	npd_syslog_dbg("Arp solicit at netif 0x%x\r\n", netif_index);

	npd_vlan_check_contain_port(vid, netif_index, &isTagged);
	
	if( 0 == memcmp(dmac,bc_mac,MAC_ADDR_LEN) )
	{
		isBC = TRUE;
	}
	data_buff = npd_packet_alloc(NAM_ARP_SOLICIT_PKTSIZE);
	if(NULL == data_buff)
	{
		npd_syslog_dbg("malloc dma err when send solicit arp\r\n");
		return COMMON_RETURN_CODE_NO_RESOURCE;
	}
	memset(data_buff,0,NAM_ARP_SOLICIT_PKTSIZE);

	/*
	 * Build up ARP solicit packet
	 */
	/* layer 2 */
	layer2 = (ether_header_t *)data_buff;
	memcpy(layer2->dmac,dmac,ETH_ALEN);
	memcpy(layer2->smac,smac,ETH_ALEN);
	layer2->etherType = htons(0x0806);

	/* layer 3 */
	layer3 = (arp_header_t *)(layer2 + 1);
	layer3->hwType 		= htons(0x1); /* ethernet hardware */
	layer3->protType	= htons(0x0800); /* IP */
	layer3->hwSize 		= 0x6;
	layer3->protSize 	= 0x4;
	layer3->opCode 		= htons(0x1); /* request */
	memcpy(layer3->smac,smac,ETH_ALEN);
	memset(layer3->dmac,0,ETH_ALEN);
	/*make sure sip and dip in network endian*/
	memcpy(layer3->sip, &sip, sizeof(sip));
	memcpy(layer3->dip, &dip, sizeof(dip));

	if( TRUE == isBC )
	{
		nam_packet_tx_broadcast_global(NAM_PACKET_TYPE_ARP_E, vid, data_buff, NAM_ARP_SOLICIT_PKTSIZE);
	}
	else {
		nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_ARP_E, netif_index, vid,  isTagged, data_buff, NAM_ARP_SOLICIT_PKTSIZE);
	}
    npd_packet_free(data_buff);
	return ARP_RETURN_CODE_SUCCESS;
}

unsigned long nam_packet_tx_arp_reply
(
    unsigned int netif_index,
    unsigned short vid,
    unsigned char isTagged,
    unsigned char *smac,
    unsigned char *dmac,
    unsigned int sip,
    unsigned int dip
)
{
	unsigned long result = ARP_RETURN_CODE_SUCCESS;
	unsigned char	*data_buff = NULL;
	ether_header_t *layer2 = NULL;
	arp_header_t *layer3 = NULL;

	int ret = -1;

	npd_syslog_dbg("netif 0x%x arp reply send\r\n", netif_index);
	
	data_buff = npd_packet_alloc(NAM_ARP_SOLICIT_PKTSIZE);
	if(NULL == data_buff)
	{
		npd_syslog_dbg("malloc dma err when send solicit arp\r\n");
		return COMMON_RETURN_CODE_NO_RESOURCE;
	}
	memset(data_buff,0,NAM_ARP_SOLICIT_PKTSIZE);

	/*
	 * Build up ARP solicit packet
	 */
	/* layer 2 */
	layer2 = (ether_header_t *)data_buff;
	memcpy(layer2->dmac,dmac,ETH_ALEN);
	memcpy(layer2->smac,smac,ETH_ALEN);
	layer2->etherType = htons(0x0806);

	/* layer 3 */
	layer3 = (arp_header_t *)(layer2 + 1);
	layer3->hwType 		= htons(0x1); /* ethernet hardware */
	layer3->protType	= htons(0x0800); /* IP */
	layer3->hwSize 		= 0x6;
	layer3->protSize 	= 0x4;
	layer3->opCode 		= htons(0x2); /* reply */
	memcpy(layer3->smac,smac,ETH_ALEN);
	memcpy(layer3->dmac,dmac,ETH_ALEN);
	/*make sure sip and dip in network endian*/
	memcpy(layer3->sip, &sip, sizeof(sip));
	memcpy(layer3->dip, &dip, sizeof(dip));
	
	ret = nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_ARP_E, 
		                                 netif_index, vid, isTagged,
		                                 data_buff, NAM_ARP_SOLICIT_PKTSIZE);
	if(0 == ret)
	{
		result = ARP_RETURN_CODE_SUCCESS;
	}
	else
	{
		result = ARP_RETURN_CODE_ERROR;
	}
	npd_packet_free(data_buff);
	return result;
}

void nam_packet_tx_session_dma_free(NAM_PACKET_TX_SESSION *nam_packet_tx_session)
{
	int i = 0, j = 0;
	for(i = 0; i < nam_asic_get_instance_num(); i++)
	{
	    for(j = 0; j < TX_BUFFER_NUM; j++)
	    {
	    /*
			if(nam_packet_tx_session->dma_buff[i*TX_BUFFER_NUM + j] != NULL)
			{
				nam_dma_cache_free(i, nam_packet_tx_session->dma_buff[i*TX_BUFFER_NUM + j]);
			}
		*/
			nam_packet_tx_session->len[i*TX_BUFFER_NUM + j] = 0;
	    }
	}
}

int nam_packet_tx_broadcast_global
(
int packet_type,
unsigned int vid,
unsigned char* data,
unsigned int dataLen
)
{
	packet_sync_ctrl *packet_ctrl = NULL;
	int ret = 0;
	int dest_slot = 0;
	#ifdef _NAM_TX_MULTITHREAD_
	int service_type = NAM_PACKET_TX_SERVICE+packet_type;
	#else
	int service_type = NAM_PACKET_TX_SERVICE;
	#endif 

#ifdef HAVE_CAPWAP_ENGINE	
    unsigned char brc_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif	

	npd_syslog_dbg( "func:%s, line:%d broadcast packet mac address:"MACSTR"\r\n", __func__, __LINE__, MAC2STR(data));
	
    if(packet_type >= NAM_PACKET_TYPE_OTHER || packet_type < 0)
    {
		return -1;
    }
	for(dest_slot = 0; dest_slot < CHASSIS_SLOT_COUNT; dest_slot++)
	{
    	if(dest_slot != SYS_LOCAL_MODULE_SLOT_INDEX)/*不是本板端口*/
    	{
    		packet_ctrl = (packet_sync_ctrl *)(data - NPD_PKT_RESERVED_LEN);
    		packet_ctrl->packet_type = packet_type;
    		packet_ctrl->netif_index = 0;/*broadcast packet*/
    		packet_ctrl->vid = vid;
    		packet_ctrl->istagged = 0;
    		packet_ctrl->packet_len = dataLen;
    		packet_ctrl->flag = 0;
    		ret = tipc_client_async_send(service_type, dest_slot + 1, (char *)(data - NPD_PKT_RESERVED_LEN), (NPD_PKT_RESERVED_LEN + dataLen));
    	}
	}
    ret = nam_packet_tx_broadcast(vid, data, dataLen);
    
#ifdef HAVE_CAPWAP_ENGINE
    if(0 == memcmp(brc_mac, data, MAC_ADDR_LEN))
	{
		struct vlan_s *vlanEntry;
		struct switch_port_db_s switchport;
		unsigned int portnum;

		
		npd_syslog_dbg( "func:%s, line:%d wireless station broadcast packet send by software\r\n", __func__, __LINE__);
		vlanEntry = npd_find_vlan_by_vid(vid);
		if(vlanEntry != NULL)
		{
			NPD_PBMP_ITER(vlanEntry->untag_ports, portnum)
			{
				if(0 == dbtable_array_get(switch_ports, portnum, &switchport))
				{
					if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(switchport.global_port_ifindex))
					{
						npd_syslog_dbg( "func:%s, line:%d wireless station broadcast packet send to specify vap \r\n", __func__, __LINE__);
						nam_packet_tx_unicast_by_netif(packet_type, switchport.global_port_ifindex,vid, FALSE, data, dataLen);
					}
				}
			}			
			free(vlanEntry);
		}
	}
#endif //HAVE_CAPWAP_ENGINE	

	
	return ret;
}

int nam_packet_tx_broadcast_wireless_station
(
int packet_type,
unsigned int vid,
unsigned char* data,
unsigned int dataLen
)
{
	packet_sync_ctrl *packet_ctrl = NULL;
	int ret = 0;
	int dest_slot = 0;
	#ifdef _NAM_TX_MULTITHREAD_
	int service_type = NAM_PACKET_TX_SERVICE+packet_type;
	#else
	int service_type = NAM_PACKET_TX_SERVICE;
	#endif
	
    if(packet_type >= NAM_PACKET_TYPE_OTHER || packet_type < 0)
    {
		return -1;
    }
	for(dest_slot = 0; dest_slot < CHASSIS_SLOT_COUNT; dest_slot++)
	{
    	if(dest_slot != SYS_LOCAL_MODULE_SLOT_INDEX)/*不是本板端口*/
    	{
    		packet_ctrl = (packet_sync_ctrl *)(data - NPD_PKT_RESERVED_LEN);
    		packet_ctrl->packet_type = packet_type;
    		packet_ctrl->netif_index = 0;/*broadcast packet*/
    		packet_ctrl->vid = vid;
    		packet_ctrl->istagged = 0;
    		packet_ctrl->packet_len = dataLen;
    		packet_ctrl->flag = 0;
    		ret = tipc_client_async_send(service_type, dest_slot + 1, (char *)(data - NPD_PKT_RESERVED_LEN), (NPD_PKT_RESERVED_LEN + dataLen));
    	}
	}
#ifdef HAVE_CAPWAP_ENGINE
	{
		struct vlan_s *vlanEntry;
		struct switch_port_db_s switchport;
		unsigned int portnum;
		vlanEntry = npd_find_vlan_by_vid(vid);
		if(vlanEntry != NULL)
		{
			NPD_PBMP_ITER(vlanEntry->untag_ports, portnum)
			{
				if(0 == dbtable_array_get(switch_ports, portnum, &switchport))
				{
					if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(switchport.global_port_ifindex))
					{
						nam_packet_tx_unicast_by_netif(packet_type, switchport.global_port_ifindex,vid, FALSE, data, dataLen);
					}
				}
			}			
			free(vlanEntry);
		}
	}
#endif //HAVE_CAPWAP_ENGINE	
	
	return ret;
}

int npd_packet_tx_broadcast_exclude_netif
(
int packet_type,
unsigned int vid,
unsigned char* data,
unsigned int dataLen,
unsigned int netif_index
)
{
	packet_sync_ctrl *packet_ctrl = NULL;
	int ret = 0;
	int dest_slot = 0;
	#ifdef _NAM_TX_MULTITHREAD_
	int service_type = NAM_PACKET_TX_SERVICE+packet_type;
	#else
	int service_type = NAM_PACKET_TX_SERVICE;
	#endif
	
    if(packet_type >= NAM_PACKET_TYPE_OTHER || packet_type < 0)
    {
		return -1;
    }
	for(dest_slot = 0; dest_slot < CHASSIS_SLOT_COUNT; dest_slot++)
	{
    	if(dest_slot != SYS_LOCAL_MODULE_SLOT_INDEX)/*不是本板端口*/
    	{
    		packet_ctrl = (packet_sync_ctrl *)(data - NPD_PKT_RESERVED_LEN);
    		packet_ctrl->packet_type = packet_type;
    		packet_ctrl->netif_index = netif_index;/*broadcast packet*/
    		packet_ctrl->vid = vid;
    		packet_ctrl->istagged = 0;
    		packet_ctrl->packet_len = dataLen;
    		packet_ctrl->flag = BROADCAST_EXCLUDE_NETIF;
    		ret = tipc_client_async_send(service_type, dest_slot + 1, (char *)(data - NPD_PKT_RESERVED_LEN), (NPD_PKT_RESERVED_LEN + dataLen));
    	}
        else
        {
		    ret = nam_packet_tx_broadcast_exclude_netif(vid, data, dataLen, netif_index);
#if 0 //HAVE_CAPWAP_ENGINE
			{
				struct vlan_s *vlanEntry;
				struct switch_port_db_s switchport;
				unsigned int portnum;
				vlanEntry = npd_find_vlan_by_vid(vid);
				if(vlanEntry != NULL)
				{
					NPD_PBMP_ITER(vlanEntry->untag_ports, portnum)
					{
						if(0 == dbtable_array_get(switch_ports, portnum, &switchport))
						{
							if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(switchport.global_port_ifindex))
							{
								nam_packet_tx_unicast_by_netif(packet_type, switchport.global_port_ifindex,vid, FALSE, data, dataLen);
							}
						}
					}
				}
				
			    free(vlanEntry);
			}
#endif //HAVE_CAPWAP_ENGINE			
        }
	}
	return ret;
}

int nam_packet_tx_unicast_by_slot
(
int packet_type,
unsigned int slot_id,
unsigned int vid,
unsigned int isTagged,
unsigned char* data,
unsigned int dataLen
)
{
	int i = 0;
	unsigned int tx_len = 0;
	unsigned char *tx_buff = NULL;
	int ret = 0;
	unsigned int peerSlot;
	unsigned char devNum, portNum;
	
    if(packet_type >= NAM_PACKET_TYPE_OTHER || packet_type < 0)
    {
		return -1;
    }
	

	if (packet_type == NAM_PACKET_TYPE_IPv4_E)
	{
		ether_header_t  *layer2 = NULL;
		ip_header_t 	*layer3 = NULL;
		unsigned int sip = 0;
		unsigned int l3index[16];
		unsigned int net_g_index = 0;
		unsigned int intfCount = 16;
		unsigned short src_vid = 0;
		int src_bind_slot = 0;
		unsigned int gateway[MAX_IP_COUNT];

		layer2 = (ether_header_t*)(data);
		layer3 = (ip_header_t *)((unsigned char*)data + sizeof(ether_header_t));
		sip = *(unsigned int *)(layer3->sip);

		/* find the sip vlan by src ip*/
		if (NPD_FALSE ==  npd_intf_addr_ifindex_get_bynet(l3index, &intfCount, sip))
		{
			npd_syslog_pkt_send("Can't not find interface by ip %x\r\n", sip);
			return -1;
		}

		if (NPD_FALSE == npd_intf_netif_get_by_ifindex(l3index[0], &net_g_index))
		{
			npd_syslog_pkt_send("Can't not find interface by ifindex %x\r\n", l3index[0]);
			return -1;
		}
		
		if(NPD_NETIF_VLAN_TYPE != npd_netif_type_get(net_g_index))
		{
			return -1;
		}

		/* check the ip is the interface's ip */
		memset(gateway, 0, MAX_IP_COUNT);
		if (NPD_FALSE == npd_intf_addr_ip_get(l3index[0],gateway,NULL))
		{
			npd_syslog_pkt_send("no l3 index in this interface,ifIndex %#0x \n",l3index[0]);
			return -1;			
		}
		if (COMMON_SUCCESS != npd_arp_snooping_ip_gateway_check(sip, gateway))
		{
			npd_syslog_pkt_send("no l3 index in this interface,ifIndex %#0x \n",l3index[0]);
			return -1;						
		}
		
	}	

	
	for(i = 0; i < PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE); i++)
    {  		
		peerSlot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, i);
		if( -1 == peerSlot )
			continue;
		
		if(peerSlot == CHASSIS_SLOT_NO2INDEX(slot_id))
		{
			devNum = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, i);
			portNum= PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, i);

			if((unsigned char)-1 == devNum || (unsigned char)-1 == portNum)
				continue;
			else 
				break;
		}			
    }
	if((unsigned char)-1 == devNum || (unsigned char)-1 == portNum)
		return -1;
    
	tx_len = dataLen;
	tx_buff = nam_dma_cache_malloc(devNum, dataLen);
	if(tx_buff == NULL)
	{
	    npd_syslog_err("dma cache malloc failed for packet TX service.\r\n");
	    npd_syslog_err("Packet type %s, buffer length: %d.\r\n", nam_get_packet_type_str(packet_type), tx_len);
		return -1;
	}
	memcpy(tx_buff, data, tx_len);
	ret = nam_packet_tx_test_unicast(packet_type, devNum, portNum, vid, isTagged, tx_buff, tx_len);
	nam_dma_cache_free(devNum, tx_buff);
	return ret;
}


int nam_packet_tx_unicast_by_netif
(
int packet_type,
unsigned int netif_index,
unsigned int vid,
unsigned int isTagged,
unsigned char* data,
unsigned int dataLen
)
{
	unsigned int eth_netif_index = 0;
	unsigned int tx_len = 0;
	unsigned char *tx_dma_buff = NULL;
	packet_sync_ctrl *packet_ctrl = NULL;
	unsigned int rc = 0;
	int ret = 0;
	unsigned int netif_type = 0;
	unsigned char devNum, portNum;
	int dest_slot = 0;
	#ifdef _NAM_TX_MULTITHREAD_
	int service_type = NAM_PACKET_TX_SERVICE+packet_type;
	#else
	int service_type = NAM_PACKET_TX_SERVICE;
	#endif

#ifdef HAVE_CAPWAP_ENGINE	
	char bssid[MAC_ADDR_LEN];
	char encap_data[NAM_SDMA_TX_PER_BUFFER];
	int encap_dataLen = 0;
	unsigned char brdcst_mac[MAC_ADDR_LEN] ={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
#endif	//HAVE_CAPWAP_ENGINE	
	
    if(packet_type >= NAM_PACKET_TYPE_OTHER || packet_type < 0)
    {
		return -1;
    }
	
    eth_netif_index = netif_index;
	netif_type = npd_netif_type_get(netif_index);
	if(netif_type == NPD_NETIF_TRUNK_TYPE)
	{
		rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &eth_netif_index);
		if(rc == NPD_FAIL)
		{
			return -1;
		}
	}
	else if(netif_type == NPD_NETIF_VLAN_TYPE)
	{
		
	}
#ifdef HAVE_CAPWAP_ENGINE		
	else if(netif_type == NPD_NETIF_WIRELESS_TYPE)
	{	
		if(memcmp(brdcst_mac, data, MAC_ADDR_LEN) == 0){
			memset(encap_data, 0, NAM_SDMA_TX_PER_BUFFER);
			if( dataLen >= NAM_SDMA_TX_PER_BUFFER )
			{
				npd_syslog_err("packet is larger than DMA TX BUFFER LEN\r\n");
				return -1;
			}
			if(0 != npd_wtp_encap_802_3_format_capwap(netif_index, data, dataLen, encap_data + NPD_PKT_RESERVED_LEN, &encap_dataLen, &eth_netif_index, &vid))
			{
				npd_syslog_err("Fail to encapusulate capwap packet\r\n");
				return -1;
			}
			memcpy(encap_data, data - NPD_PKT_RESERVED_LEN, NPD_PKT_RESERVED_LEN);
			npd_syslog_err("capwap packet len %d send from netif index: 0x%x vid:%d\r\n", encap_dataLen, eth_netif_index, vid);
			data = encap_data + NPD_PKT_RESERVED_LEN;
			dataLen = encap_dataLen;
		}
		else{
		    eth_netif_index = netif_index;
		}
	}
#endif   //HAVE_CAPWAP_ENGINE

	if(netif_type != NPD_NETIF_WIRELESS_TYPE){
		/*对于无交换芯片的单板，通过判断槽位号确定是否本板端口不合理，所以改成获取dev port*/
	    rc = npd_get_devport_by_global_index(eth_netif_index, &devNum, &portNum);
		if (rc != 0)
		{
	    	dest_slot = npd_netif_eth_get_slot(eth_netif_index);
	    	if(dest_slot != SYS_LOCAL_MODULE_SLOT_INDEX)/*不是本板端口*/
	    	{
	    		npd_syslog_dbg("The target port is not on local board. Packet will be sent to slot %d.\r\n", dest_slot + 1);
	    
	    		packet_ctrl = (packet_sync_ctrl *)(data - NPD_PKT_RESERVED_LEN);
	    		packet_ctrl->packet_type = packet_type;
	    		packet_ctrl->netif_index = eth_netif_index;
	    		packet_ctrl->vid = vid;
	    		packet_ctrl->istagged = isTagged;
	    		packet_ctrl->packet_len = dataLen;
	        	packet_ctrl->flag = 0;
	    		ret = tipc_client_async_send(service_type, dest_slot + 1, (char *)(data - NPD_PKT_RESERVED_LEN), (NPD_PKT_RESERVED_LEN + dataLen));
	    		return ret;
	    	}
			return 0;
		}
	}
	
	tx_len = dataLen;
	tx_dma_buff = nam_dma_cache_malloc(devNum, dataLen);
	if(tx_dma_buff == NULL)
	{
	    npd_syslog_err("dma cache malloc failed for packet TX service.\r\n");
	    npd_syslog_err("Packet type %s, buffer length: %d.\r\n", nam_get_packet_type_str(packet_type), tx_len);
		return -1;
	}
	ret = nam_packet_tx_unicast_send(eth_netif_index, vid, isTagged, data, tx_len, tx_dma_buff);
	nam_dma_cache_free(devNum, tx_dma_buff);
	return ret;
}

int nam_packet_tx_unicast_by_netif_witch_dma_buffer
(
int packet_type,
unsigned int netif_index,
unsigned int vid,
unsigned int isTagged,
unsigned char* data,
unsigned int dataLen
)
{
	unsigned int eth_netif_index = 0;
	packet_sync_ctrl *packet_ctrl = NULL;
	unsigned int rc = 0;
	int ret = 0;
	unsigned int netif_type = 0;
	unsigned char devNum, portNum;
	int dest_slot = 0;
	#ifdef _NAM_TX_MULTITHREAD_
	int service_type = NAM_PACKET_TX_SERVICE+packet_type;
	#else
	int service_type = NAM_PACKET_TX_SERVICE;
	#endif
    if(packet_type >= NAM_PACKET_TYPE_OTHER || packet_type < 0)
    {
		npd_syslog_err("Packet with unknown type.\r\n");
		return -1;
    }
	
    eth_netif_index = netif_index;
	netif_type = npd_netif_type_get(netif_index);
	if(netif_type == NPD_NETIF_TRUNK_TYPE)
	{
		rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &eth_netif_index);
		if(rc == NPD_FALSE)
		{
			return -1;
		}
	}
	else if(netif_type == NPD_NETIF_VLAN_TYPE)
	{
		
	}
	
    rc = npd_get_devport_by_global_index(eth_netif_index, &devNum, &portNum);
	if (rc != 0)
	{
    	dest_slot = npd_netif_eth_get_slot(eth_netif_index);
    	if(dest_slot != SYS_LOCAL_MODULE_SLOT_INDEX)/*不是本板端口*/
    	{
    		packet_ctrl = (packet_sync_ctrl *)(data - NPD_PKT_RESERVED_LEN);
    		packet_ctrl->packet_type = packet_type;
    		packet_ctrl->netif_index = eth_netif_index;
    		packet_ctrl->vid = vid;
    		packet_ctrl->istagged = isTagged;
    		packet_ctrl->packet_len = dataLen;
        	packet_ctrl->flag = 0;
    		ret = tipc_client_async_send(service_type, dest_slot+1, (char *)(data - NPD_PKT_RESERVED_LEN), (NPD_PKT_RESERVED_LEN + dataLen));
    		return ret;
    	}
		return 0;
	}
	
	ret = nam_packet_tx_unicast_send(eth_netif_index, vid, isTagged, data, dataLen, data);
	return ret;
}

/*
不同协议的报文发送控制结构有待统一
由于涉及上层应用，暂时不做修改
*/
int nam_packet_tx_dldp(int sock, char *buff, int len, void *private_data)
{

	unsigned int rc = 0;
	unsigned int tx_len = 0;
	unsigned char *data_buff = NULL;
	unsigned char istagged = 0;
	unsigned short vid = 0;
	unsigned int netif_index = 0;
	unsigned int e_netif_index = 0;
	packet_sync_ctrl *dldpPktBuff = NULL;	
	NAM_PACKET_TX_SESSION *nam_packet_tx_session = (NAM_PACKET_TX_SESSION *)private_data;
	if(len < 0)
	{
		if(nam_packet_tx_session)
		{
		    nam_packet_tx_session_dma_free(nam_packet_tx_session);
		}
		return -1;
	}
	if(nam_packet_tx_session == NULL)
	{
		return 0;
	}
	
    dldpPktBuff = (packet_sync_ctrl *)buff;
	data_buff = (unsigned char*)(buff + NPD_PKT_RESERVED_LEN);
    netif_index = dldpPktBuff->netif_index;
	tx_len = dldpPktBuff->nlh.nlmsg_len - NPD_PKT_RESERVED_LEN;
	vid = dldpPktBuff->vid;
	istagged = dldpPktBuff->istagged;
	e_netif_index = netif_index;
	
	if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
	{
		rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &e_netif_index);
		if(rc == NPD_FALSE)
		{
			return -1;
		}
	}
	
    nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_DLDP_E, e_netif_index, 
				                           vid, istagged, 
				                           data_buff, tx_len);
	return 0;
}

int nam_packet_tx_eap(int sock, char *buff, int len, void *private_data)
{

	unsigned int rc = 0;
	unsigned char *data_buff = NULL;
	unsigned char istagged = 0;
	unsigned short vid = 0;
	unsigned int tx_len = 0;
	unsigned int netif_index = 0;
	unsigned int e_netif_index = 0;
	packet_sync_ctrl *asdPktBuff = NULL;	
	NAM_PACKET_TX_SESSION *nam_packet_tx_session = (NAM_PACKET_TX_SESSION *)private_data;
	npd_syslog_dbg("EAP packet TX \r\n");
	if(len < 0)
	{
		if(nam_packet_tx_session)
		{
		    nam_packet_tx_session_dma_free(nam_packet_tx_session);
		}
		return -1;
	}
	if(nam_packet_tx_session == NULL)
	{
		return 0;
	}
	asdPktBuff = (packet_sync_ctrl *)buff;
	data_buff = (unsigned char*)(buff + NPD_PKT_RESERVED_LEN);
	netif_index = asdPktBuff->netif_index;
	vid = asdPktBuff->vid;
	tx_len = len - NPD_PKT_RESERVED_LEN;
	e_netif_index = netif_index;
	npd_syslog_dbg("EAP packet TX ifindex 0x%x, vlan id %d\r\n", netif_index, vid);
	if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
	{
		rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &e_netif_index);
		if(rc == NPD_FALSE)
		{
			return -1;
		}
    	rc = npd_vlan_check_contain_trunk(vid, npd_netif_trunk_get_tid(netif_index), &istagged);
    	if (rc == NPD_FALSE)
    	{
    		if(0 != npd_vlan_port_pvid_get(netif_index, &vid))
    		{
    			npd_syslog_err("global index 0x%x is not in VLAN %d!\r\n",
    								e_netif_index, asdPktBuff->vid);
    			return -1;
    		}
    	}
	}
	else if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
	{
    	rc = npd_vlan_check_contain_port(vid, netif_index, &istagged);
    	if (rc == NPD_FALSE)
    	{
    		if(0 != npd_vlan_port_pvid_get(netif_index, &vid))
    		{
    			npd_syslog_err("global index 0x%x is not in VLAN %d!\r\n",
    								netif_index, asdPktBuff->vid);
    			return -1;
    		}
    	}
	}
	nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_EAP_E, e_netif_index, 
				                           vid, istagged, 
				                           data_buff, tx_len);
	return 0;
}

int nam_packet_tx_lldp(int sock, char *buff, int len, void *private_data)
{
	unsigned int rc = 0;
	unsigned char *data_buff = NULL;
	unsigned char istagged = 0;
	unsigned short vid = 0;
	unsigned int tx_len = 0;
	unsigned int netif_index = 0;
	unsigned int e_netif_index = 0;
	packet_sync_ctrl *lldpPktBuff = NULL;	
	NAM_PACKET_TX_SESSION *nam_packet_tx_session = (NAM_PACKET_TX_SESSION *)private_data;
	npd_syslog_dbg("lldp packet TX ...\r\n");
	if(len < 0)
	{
		if(nam_packet_tx_session)
		{
		    nam_packet_tx_session_dma_free(nam_packet_tx_session);
		}
		npd_syslog_err("packet len error\r\n");
		return -1;
	}
	if(nam_packet_tx_session == NULL)
	{
		npd_syslog_err("TX session NULL\r\n");
		return 0;
	}
	lldpPktBuff = (packet_sync_ctrl *)buff;
	data_buff = (unsigned char*)(buff + NPD_PKT_RESERVED_LEN);
	netif_index = lldpPktBuff->netif_index;
	vid = lldpPktBuff->vid;
	tx_len = len - NPD_PKT_RESERVED_LEN;
	e_netif_index = netif_index;
	if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
	{
		rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &e_netif_index);
		if(rc == NPD_FALSE)
		{
    		npd_syslog_err("No master port of port-channel %d!\r\n",
    								npd_netif_trunk_get_tid(netif_index));
			return -1;
		}
    	rc = npd_vlan_check_contain_trunk(vid, npd_netif_trunk_get_tid(netif_index), &istagged);
    	if (rc == NPD_FALSE)
    	{
    		npd_syslog_err("Port-channel (netif index 0x%x) is not in VLAN %d!\r\n",
    								e_netif_index, vid);
    		return -1;
    	}
	}
	else if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
	{
    	rc = npd_vlan_check_contain_port(vid, netif_index, &istagged);
    	if (rc == NPD_FALSE)
    	{
			rc = npd_vlan_port_pvid_get(netif_index, &vid);
			if(rc != 0)
			{
				npd_syslog_err("Ethernet port (netif index 0x%x) is not in VLAN %d!\r\n",
    								netif_index, vid);
    		    return -1;
			}
    	}
	}
	else
	{
		npd_syslog_err("global index 0x%x is unknown netif type!\r\n",
    								netif_index);
		return -1;
	}
	npd_system_get_basemac(data_buff + 6, 6);
    nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_LLDP_E, e_netif_index, 
				                           vid, istagged, 
				                           data_buff, tx_len);
	return 0;
}
#ifdef HAVE_UDLD
#include "udld/udld_main.h"
#include "udld/udld_packet.h"
int nam_packet_tx_udld(int sock, char *buff, int len, void *private_data)
{
	unsigned int rc = 0;
	unsigned char *data_buff = NULL;
	unsigned char istagged = 0;
	unsigned short vid = 0;
	unsigned int tx_len = 0;
	unsigned int netif_index = 0;
	unsigned int e_netif_index = 0;
	udld_skb *udldPktBuff = NULL;	
	NAM_PACKET_TX_SESSION *nam_packet_tx_session = (NAM_PACKET_TX_SESSION *)private_data;
	npd_syslog_dbg("UDLD packet TX ...\r\n");
	if(len < 0)
	{
		if(nam_packet_tx_session)
		{
		    nam_packet_tx_session_dma_free(nam_packet_tx_session);
		}
		npd_syslog_err("packet len error\r\n");
		return -1;
	}
	if(nam_packet_tx_session == NULL)
	{
		npd_syslog_err("TX session NULL\r\n");
		return 0;
	}
	udldPktBuff = (udld_skb *)buff;
	data_buff = (unsigned char*)(udldPktBuff->buf);
	netif_index = udldPktBuff->netif_index;
	vid = udldPktBuff->vid;
	tx_len = len - NPD_PKT_RESERVED_LEN;
	e_netif_index = netif_index;
	if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
	{
		rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &e_netif_index);
		if(rc == NPD_FALSE)
		{
    		npd_syslog_err("No master port of port-channel %d!\r\n",
    								npd_netif_trunk_get_tid(netif_index));
			return -1;
		}
    	rc = npd_vlan_check_contain_trunk(vid, npd_netif_trunk_get_tid(netif_index), &istagged);
    	if (rc == NPD_FALSE)
    	{
    		npd_syslog_err("Port-channel (netif index 0x%x) is not in VLAN %d!\r\n",
    								e_netif_index, vid);
    		return -1;
    	}
	}
	else if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
	{
    	rc = npd_vlan_check_contain_port(vid, netif_index, &istagged);
    	if (rc == NPD_FALSE)
    	{
			rc = npd_vlan_port_pvid_get(netif_index, &vid);
			if(rc != 0)
			{
				npd_syslog_err("Ethernet port (netif index 0x%x) is not in VLAN %d!\r\n",
    								netif_index, vid);
    		    return -1;
			}
    	}
	}
	else
	{
		npd_syslog_err("global index 0x%x is unknown netif type!\r\n",
    								netif_index);
		return -1;
	}
	npd_system_get_basemac(data_buff + 6, 6);
    nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_UDLD_E, e_netif_index, 
				                           vid, istagged, 
				                           data_buff, tx_len);
	return 0;
}
#endif
#ifdef HAVE_SMART_LINK
int nam_packet_tx_stlk(int sock, char *buff, int len, void *private_data)
{
    unsigned int rc = 0;
    unsigned char* data = NULL;
    unsigned char istagged = 0;
    unsigned short vid = 0;
    unsigned int tx_len = 0;
    unsigned int netif_index = 0;
    unsigned int e_netif_index = 0;
    packet_sync_ctrl* stlkPktBuff = NULL;	
    NAM_PACKET_TX_SESSION* nam_packet_tx_session = (NAM_PACKET_TX_SESSION *)private_data;
    npd_syslog_dbg("STLK packet TX \r\n");
    
    if (len < 0)
    {
        if (nam_packet_tx_session)
        {
            nam_packet_tx_session_dma_free(nam_packet_tx_session);
        }
        return -1;
    }
    
    if (NULL == nam_packet_tx_session)
    {
        return 0;
    }
    stlkPktBuff = (packet_sync_ctrl *)buff;
    data = (unsigned char*)(buff + NPD_PKT_RESERVED_LEN);
    netif_index = stlkPktBuff->netif_index;
    vid = stlkPktBuff->vid;
    istagged = (char)stlkPktBuff->istagged;
    tx_len = len - NPD_PKT_RESERVED_LEN;
    e_netif_index = netif_index;
    npd_syslog_dbg("STLK packet TX ifindex 0x%x, vlan id %d\r\n", netif_index, vid);
    if (npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
    {
        rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &e_netif_index);
        if (rc == NPD_FALSE)
        {
            return -1;
        }
        rc = npd_vlan_check_contain_trunk(vid, npd_netif_trunk_get_tid(netif_index), &istagged);
        if (rc == NPD_FALSE)
        {
            if (0 != npd_vlan_port_pvid_get(netif_index, &vid))
            {
                npd_syslog_err("global index 0x%x is not in VLAN %d!\r\n", e_netif_index, stlkPktBuff->vid);
                return -1;
            }
        }
    }
    else if (npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
    {
        rc = npd_vlan_check_contain_port(vid, netif_index, &istagged);
        if (rc == NPD_FALSE)
        {
            if (0 != npd_vlan_port_pvid_get(netif_index, &vid))
            {
                npd_syslog_err("global index 0x%x is not in VLAN %d!\r\n", netif_index, stlkPktBuff->vid);
                return -1;
            }
        }
    }
    npd_syslog_dbg("netif = %x, vid = %d, istag = %d\n", e_netif_index, vid, istagged & 0xff);
    nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_STLK_E, e_netif_index, vid, istagged, data, tx_len);

    return 0;
}
#endif

int nam_packet_tx_rstp(int sock, char *buff, int len, void *private_data)
{
	unsigned int rc = 0;
	unsigned char *data_buff = NULL;
	unsigned char istagged = 0;
	unsigned short vid = 0;
	unsigned int tx_len = 0;
	unsigned int netif_index = 0;
	unsigned int e_netif_index = 0;
	packet_txrx_ctrl *bpduPktBuff = NULL;	
	NAM_PACKET_TX_SESSION *nam_packet_tx_session = (NAM_PACKET_TX_SESSION *)private_data;
	npd_syslog_dbg("BPDU packet TX \r\n");
	if(len < 0)
	{
		if(nam_packet_tx_session)
		{
		    nam_packet_tx_session_dma_free(nam_packet_tx_session);
		}
		return -1;
	}
	if(nam_packet_tx_session == NULL)
	{
		return 0;
	}
	bpduPktBuff = (packet_txrx_ctrl *)buff;
	netif_index = bpduPktBuff->sync_ctrl.netif_index;
	tx_len = bpduPktBuff->sync_ctrl.packet_len;
	data_buff = (unsigned char*)(bpduPktBuff + 1);
	vid = 0;
	istagged = 0;
	e_netif_index = netif_index;
	if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
	{
		rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &e_netif_index);
		if(rc == NPD_FALSE)
		{
			return -1;
		}
	}
    nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_BPDU_E, e_netif_index, 
				                           vid, istagged, 
				                           data_buff, tx_len);
	return 0;
}

int nam_packet_tx_ip_igmp(int sock, char *buff, int len, void *private_data)
{

	unsigned int rc = 0;
	unsigned char *data_buff = NULL;
	unsigned char istagged = 0;
	unsigned short vid = 0;
	unsigned int tx_len = 0;
	unsigned int netif_index = 0;
	unsigned int e_netif_index = 0;
	packet_sync_ctrl *igmpPktBuff = NULL;	
	NAM_PACKET_TX_SESSION *nam_packet_tx_session = (NAM_PACKET_TX_SESSION *)private_data;
	npd_syslog_dbg("IP IGMP packet TX \r\n");
	if(len < 0)
	{
		if(nam_packet_tx_session)
		{
		    nam_packet_tx_session_dma_free(nam_packet_tx_session);
		}
		return -1;
	}
	if(nam_packet_tx_session == NULL)
	{
		return 0;
	}
	igmpPktBuff = (packet_sync_ctrl *)buff;
	netif_index = igmpPktBuff->netif_index;
	tx_len = igmpPktBuff->nlh.nlmsg_len - NPD_PKT_RESERVED_LEN;
	data_buff = (unsigned char*)(buff + NPD_PKT_RESERVED_LEN);
	vid = igmpPktBuff->vid;
	e_netif_index = netif_index;
	if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
	{
		rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &e_netif_index);
		if(rc == NPD_FALSE)
		{
			npd_syslog_err("port channel %d has no master port\r\n", npd_netif_trunk_get_tid(netif_index));
			return -1;
		}
    	rc = npd_vlan_check_contain_trunk(igmpPktBuff->vid, npd_netif_trunk_get_tid(netif_index), &istagged);
    	if (rc != NPD_TRUE)
    	{
    		npd_syslog_err("global index 0x%x is not in VLAN %d!\r\n", netif_index, igmpPktBuff->vid);
    		return -1;
    	}
	}
	else if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
	{
    	rc = npd_vlan_check_contain_port(igmpPktBuff->vid, netif_index, &istagged);
    	if (rc != NPD_TRUE)
    	{
    		npd_syslog_err("global index 0x%x is not in VLAN %d!\r\n", netif_index, igmpPktBuff->vid);
    		return -1;
    	}
	}
	
    nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_IP_IGMP_E, e_netif_index, 
				                           vid, istagged, 
				                           data_buff, tx_len);
	return 0;
}

#ifdef HAVE_MLD_SNP
int nam_packet_tx_ipv6_mld(int sock, char *buff, int len, void *private_data)
{

	unsigned int rc = 0;
	unsigned char *data_buff = NULL;
	unsigned char istagged = 0;
	unsigned short vid = 0;
	unsigned int tx_len = 0;
	unsigned int netif_index = 0;
	unsigned int e_netif_index = 0;
	packet_sync_ctrl *mldPktBuff = NULL;	
	NAM_PACKET_TX_SESSION *nam_packet_tx_session = (NAM_PACKET_TX_SESSION *)private_data;
	npd_syslog_dbg("IP MLD packet TX \r\n");
	if(len < 0)
	{
		if(nam_packet_tx_session)
		{
		    nam_packet_tx_session_dma_free(nam_packet_tx_session);
		}
		return -1;
	}
	if(nam_packet_tx_session == NULL)
	{
		return 0;
	}
	mldPktBuff = (packet_sync_ctrl *)buff;
	netif_index = mldPktBuff->netif_index;
	tx_len = mldPktBuff->nlh.nlmsg_len - NPD_PKT_RESERVED_LEN;
	data_buff = (unsigned char*)(buff + NPD_PKT_RESERVED_LEN);
	vid = mldPktBuff->vid;
	e_netif_index = netif_index;
	if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
	{
		rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &e_netif_index);
		if(rc == NPD_FALSE)
		{
			return -1;
		}
    	rc = npd_vlan_check_contain_trunk(mldPktBuff->vid, npd_netif_trunk_get_tid(netif_index), &istagged);
    	if (rc != 0)
    	{
    		npd_syslog_err("global index 0x%x is not in VLAN %d!\r\n",
    								netif_index, mldPktBuff->vid);
    		return -1;
    	}
	}
	else if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
	{
    	rc = npd_vlan_check_contain_port(mldPktBuff->vid, netif_index, &istagged);
    	if (rc != NPD_TRUE)
    	{
    		npd_syslog_err("global index 0x%x is not in VLAN %d!\r\n",
    								netif_index, mldPktBuff->vid);
    		return -1;
    	}
	}
	
    nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_MLD_E, e_netif_index, 
				                           vid, istagged, 
				                           data_buff, tx_len);
	return 0;
}
#endif        

int nam_packet_tx_tipc_accept(int ctrl_sock, int sock, void **data)
{
	NAM_PACKET_TX_SESSION *nam_packet_tx_session = NULL;
	
	nam_packet_tx_session = malloc(sizeof(NAM_PACKET_TX_SESSION));
	if(nam_packet_tx_session == NULL)
	{
		printf("%s %d: Malloc memory for NAM_PACKET_TX_SESSION failed.\r\n", __func__, __LINE__);
		return -1;
	}
	if(*data)
	{
		memcpy(nam_packet_tx_session, *data, sizeof(NAM_PACKET_TX_SESSION));
	}
	nam_packet_tx_session->sock = sock;
	nam_packet_tx_session->local = 0;
	*data = nam_packet_tx_session;
	return 0;
}

int nam_packet_tx_tipc_handler(int sock, char *buff, int len, void *private_data)
{

	int i = 0;
	unsigned int rc = 0;
	unsigned int tx_len = 0;
	unsigned char *tx_buff = NULL;
	unsigned char istagged = 0;
	unsigned short vid = 0;
	unsigned int netif_index = 0;
	unsigned int e_netif_index = 0;
	packet_sync_ctrl *packet_sync = NULL;
	unsigned char devNum, portNum;
	NAM_PACKET_TX_SESSION *nam_packet_tx_session = (NAM_PACKET_TX_SESSION *)private_data;
	if(len < 0)
	{
		if(nam_packet_tx_session)
		{
		    nam_packet_tx_session_dma_free(nam_packet_tx_session);
		}
		return -1;
	}
	if(nam_packet_tx_session == NULL)
	{
		return -1;
	}
	if(nam_packet_tx_session->local == 0)/*from tipc socket*/
	{
		packet_sync = (packet_sync_ctrl *)(buff);
		tx_len = packet_sync->packet_len;
		vid = packet_sync->vid;
		istagged = packet_sync->istagged;
		netif_index = packet_sync->netif_index;
	}
	else
	{
	    return -1;
	}
	if(netif_index == 0 && vid != 0)/*broadcast*/
	{
		return nam_packet_tx_broadcast(vid, (unsigned char *)(buff + NPD_PKT_RESERVED_LEN), tx_len);
	}
	if(packet_sync->flag == BROADCAST_EXCLUDE_NETIF && netif_index != 0 && vid != 0)
	{
		return nam_packet_tx_broadcast_exclude_netif(vid, (unsigned char *)(buff + NPD_PKT_RESERVED_LEN), tx_len, netif_index);
	}
	/*unicast*/
	e_netif_index = netif_index;
	if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
	{
		rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &e_netif_index);
		if(rc == NPD_FALSE)
		{
			return -1;
		}
	}
    rc = npd_get_devport_by_global_index(e_netif_index, &devNum, &portNum);
	if (rc != 0)
	{
		npd_syslog_err("global index 0x%x get port (%d/%d) error, result %d!\r\n",
								e_netif_index, devNum, portNum, rc);
		return -1;
	}
	for(i = 0; i < TX_BUFFER_NUM; i++)
	{
		if(nam_packet_tx_session->len[TX_BUFFER_NUM*devNum + i] == 0 && nam_packet_tx_session->dma_buff[TX_BUFFER_NUM*devNum + i])
		{
			nam_packet_tx_session->len[TX_BUFFER_NUM*devNum + i] = tx_len;
			tx_buff = nam_packet_tx_session->dma_buff[TX_BUFFER_NUM*devNum + i];
			nam_packet_tx_unicast_send(e_netif_index, 
				                           vid, istagged, 
				                           (unsigned char *)(buff + NPD_PKT_RESERVED_LEN), tx_len, tx_buff);
			nam_packet_tx_session->len[TX_BUFFER_NUM*devNum + i] = 0;
			return 0;
		}
	}
	npd_syslog_dbg("%s %d, DMA buffer is full.\r\n", __func__, __LINE__);
	return 0;
}
extern int local_socket[NAM_PACKET_TYPE_OTHER];
int nam_packet_tx_socket_init(int type, int service_priority, int (*tx_packet_handler)(int sock, char *buff, int len, void *private_data), int buff_len)
{
	#ifdef _NAM_TX_MULTITHREAD_
	int tx_service_type = NAM_PACKET_TX_SERVICE+type;
	#else
	int tx_service_type = NAM_PACKET_TX_SERVICE;
	static int tx_client_inited = 0;
	#endif
	int ret = -1;
	int sock = -1;
	int i = 0, j = 0;
	NAM_PACKET_TX_SESSION *nam_packet_tx_session = NULL;
    if(type >= NAM_PACKET_TYPE_OTHER || type < 0)
    {
		return -1;
    }
    sock = local_socket[type];
	nam_packet_tx_session = malloc(sizeof(NAM_PACKET_TX_SESSION));
	if(nam_packet_tx_session == NULL)
	{
		return -1;
	}
	nam_packet_tx_session->sock = sock;
	nam_packet_tx_session->local = 1;
	for(i = 0; i < nam_asic_get_instance_num(); i++)
	{
	    for(j = 0; j < TX_BUFFER_NUM; j++)
	    {
			nam_packet_tx_session->dma_buff[i*TX_BUFFER_NUM + j] = nam_dma_cache_malloc(i, buff_len);
			if(nam_packet_tx_session->dma_buff[i*TX_BUFFER_NUM + j] == NULL)
			{
				printf("dma cache malloc failed for packet TX service.\r\n");
				printf("Packet type %s, buffer length: %d.\r\n", nam_get_packet_type_str(type), buff_len);
				return -1;
			}
			nam_packet_tx_session->len[i*TX_BUFFER_NUM + j] = 0;
	    }
	}
	if(tx_packet_handler)
	{
    	if(sock == 0)
    	{
    		printf("%s %d: socket for packet %s is not created.\r\n", __func__, __LINE__, nam_get_packet_type_str(type));
    		return -1;
    	}
        ret = osal_thread_register_fd(sock, tx_service_type, OSAL_THREAD_READ, tx_packet_handler, nam_packet_tx_session, 1);
    	if(ret != 0)
    	{
    		printf("%s %d: Register socket %d to thread master failed.\r\n", __func__, __LINE__, sock);
    		nam_packet_tx_session_dma_free(nam_packet_tx_session);
    		free(nam_packet_tx_session);
    		return -1;
    	}
	}
	#ifndef _NAM_TX_MULTITHREAD_
	if(tx_client_inited == 0)
	{
	#endif
    	ret = tipc_client_init(tx_service_type, NULL);
    	if(ret != 0)
    	{
    		printf("%s %d: Create TIPC client for PACKET TX service failed.\r\n", __func__, __LINE__);
    		/*不要释放资源了，
    		1)可能上面注册的thread已经在使用该资源
    		2)释放也没有什么意义*/
    		return -1;
    	}
    	ret = tipc_server_init(tx_service_type, SYS_LOCAL_MODULE_SLOT_INDEX + 1, nam_packet_tx_tipc_handler, nam_packet_tx_tipc_accept, nam_packet_tx_session);
    	if(ret != 0)
    	{
    		printf("%s %d: Create TIPC socket as server failed.\r\n", __func__, __LINE__);
    		/*不要释放资源了，
    		1)可能上面注册的thread已经在使用该资源
    		2)释放也没有什么意义*/
    		return -1;
    	}
		osal_thread_read_buffer_length_set(tx_service_type, NAM_TX_MAX_PACKET_LEN);
    	ret = osal_thread_create((char *)nam_get_packet_type_str(type), (void (*)(void *))osal_thread_master_run, service_priority, 0x80000, (void *)tx_service_type);
    	if(ret != 0)
    	{
    		printf("%s %d: Create thread for thread master failed.\r\n", __func__, __LINE__);
    		/*不要释放资源了，
    		1)可能上面注册的thread已经在使用该资源
    		2)释放也没有什么意义*/
    		return -1;
    	}
	#ifndef _NAM_TX_MULTITHREAD_
	    tx_client_inited = 1;
	}
	#endif
	return 0;

}
#ifdef HAVE_ERPP
#include "erpp/erpp_packet.h"
int nam_packet_tx_erpp(int sock, char *buff, int len, void *private_data)
{

	unsigned int rc = 0;
	unsigned char *data_buff = NULL;
	unsigned char istagged = 0;
	unsigned short vid = 0;
	unsigned int tx_len = 0;
	unsigned int netif_index = 0;
	unsigned int e_netif_index = 0;
	union erpp_packet_sync_ctrl_u *erppPktBuff = NULL;	
	npd_syslog_dbg("erpp packet TX \r\n");

	erppPktBuff = (union erpp_packet_sync_ctrl_u *)buff;
	data_buff = (unsigned char*)(buff + NPD_PKT_RESERVED_LEN);
	netif_index = erppPktBuff->sync_ctrl.netif_index;
	vid = erppPktBuff->sync_ctrl.vid;
	tx_len = len - NPD_PKT_RESERVED_LEN;
	e_netif_index = netif_index;
	npd_syslog_dbg("erpp packet TX ifindex 0x%x, vlan id %d\r\n", netif_index, vid);
	if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
	{
		rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &e_netif_index);
		if(rc == NPD_FALSE)
		{
			return -1;
		}
    	rc = npd_vlan_check_contain_trunk(vid, npd_netif_trunk_get_tid(netif_index), &istagged);
    	if (rc == NPD_FALSE)
    	{
    		if(0 != npd_vlan_port_pvid_get(netif_index, &vid))
    		{
    			npd_syslog_err("global index 0x%x is not in VLAN %d!\r\n",
    								e_netif_index, erppPktBuff->sync_ctrl.vid);
    			return -1;
    		}
    	}
	}
	else if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
	{
    	rc = npd_vlan_check_contain_port(vid, netif_index, &istagged);
    	if (rc == NPD_FALSE)
    	{
    		if(0 != npd_vlan_port_pvid_get(netif_index, &vid))
    		{
    			npd_syslog_err("global index 0x%x is not in VLAN %d!\r\n",
    								netif_index, erppPktBuff->sync_ctrl.vid);
    			return -1;
    		}
    	}
	}
	nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_ERPP_E, e_netif_index, 
				                           vid, istagged, 
				                           data_buff, tx_len);
	return 0;
}
#endif
unsigned long nam_packet_tx_adapter_init()
{
	unsigned long 		rc = NPD_SUCCESS;
	
    nam_thread_create(
					kapTxTaskName,                 /* Task Name             	*/
                	nam_packet_kap_tx_task,		/* Starting Point 				*/
                	NULL,          							/* there is no arguments 	*/
                	TRUE,
                	FALSE);
	
	npd_syslog_pkt_send("kap tx task %s created\r\n",kapTxTaskName);

	nam_packet_tx_socket_init(NAM_PACKET_TYPE_BPDU_E, 50, nam_packet_tx_rstp, NAM_TX_MAX_PACKET_LEN);
	nam_packet_tx_socket_init(NAM_PACKET_TYPE_DLDP_E, 51, nam_packet_tx_dldp, NAM_TX_MAX_PACKET_LEN);
	nam_packet_tx_socket_init(NAM_PACKET_TYPE_EAP_E, 52, nam_packet_tx_eap, NAM_TX_MAX_PACKET_LEN);
	nam_packet_tx_socket_init(NAM_PACKET_TYPE_LLDP_E, 52, nam_packet_tx_lldp, NAM_TX_MAX_PACKET_LEN);
#ifdef HAVE_UDLD
	nam_packet_tx_socket_init(NAM_PACKET_TYPE_UDLD_E, 53, nam_packet_tx_udld, NAM_TX_MAX_PACKET_LEN);
#endif
#ifdef HAVE_SMART_LINK
    nam_packet_tx_socket_init(NAM_PACKET_TYPE_STLK_E, 53, nam_packet_tx_stlk, NAM_TX_MAX_PACKET_LEN);
#endif
	nam_packet_tx_socket_init(NAM_PACKET_TYPE_IP_IGMP_E, 53, nam_packet_tx_ip_igmp, NAM_TX_MAX_PACKET_LEN);
#ifdef HAVE_MLD_SNP
	nam_packet_tx_socket_init(NAM_PACKET_TYPE_MLD_E, 53, nam_packet_tx_ipv6_mld, NAM_TX_MAX_PACKET_LEN);
#endif
#ifdef HAVE_ERPP
	nam_packet_tx_socket_init(NAM_PACKET_TYPE_ERPP_E, 55, nam_packet_tx_erpp, NAM_TX_MAX_PACKET_LEN);
#endif
	nam_packet_tx_socket_init(NAM_PACKET_TYPE_ARP_E, 54, NULL, NAM_TX_MAX_PACKET_LEN);
	nam_packet_tx_socket_init(NAM_PACKET_TYPE_IPv4_E, 55, NULL, NAM_TX_MAX_PACKET_LEN);
	nam_packet_tx_socket_init(NAM_PACKET_TYPE_IP_ICMP_E, 55, NULL, NAM_TX_MAX_PACKET_LEN);

    npd_packet_tx_hook_register(NAM_PACKET_TYPE_ARP_E, 
									  "ARP", 0, 
									  npd_packet_type_is_ARP, npd_packet_tx_hook_arp);

    npd_packet_tx_hook_register(NAM_PACKET_TYPE_IPv4_E, 
									  "IPv4", 0, 
									  npd_packet_type_is_IPv4, NULL);
	npd_packet_tx_hook_sub_register(NAM_PACKET_TYPE_IPv4_E, NAM_PACKET_TYPE_IP_UDP_DHCP_E, 
									 "DHCP", 0, 
									 npd_packet_type_is_Dhcp, npd_packet_tx_hook_ipv4_udp_dhcp);
	return rc;
}

#ifdef __cplusplus
}
#endif

