
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "man_str_parse.h"

int is_system_box = FALSE;
int is_system_cascade = FALSE;
int local_unitno = 1;



/*parse string to trunk netif index*/
unsigned int parse_str_to_bond_index(char* str)
{
    NPD_NETIF_INDEX_U trunk_ifindex;
	char *endptr = NULL;
	char c = 0;
	unsigned int tid = 0;
	if (NULL == str) return 0;
	trunk_ifindex.netif_index = 0;
	trunk_ifindex.trunk_if.type = NPD_NETIF_TRUNK_TYPE;
	c = str[0];
	if (c>='0'&&c<='9')
	{
		tid = (unsigned short)strtoul(str,&endptr,10);
		if(endptr == NULL)
		{
			return 0;
		}
		if('\0' != endptr[0])
		{
			return 0;
		}
		if ((tid < 1)||(tid >127))
        {
            return 0;
        }
		trunk_ifindex.trunk_if.trunkid = tid;
		trunk_ifindex.netif_index = npd_netif_trunk_index(trunk_ifindex.trunk_if.trunkid);
		return trunk_ifindex.netif_index;	
	}
	else
	{
		return 0; /*not Vlan ID. for Example ,enter '.',and so on ,some special char.*/
	}
}

/*parse string to trunk id*/
int parse_trunk_no(char* str,unsigned short* trunkId)
{
	char *endptr = NULL;
	char c = 0;
	if (NULL == str) return -1;
	c = str[0];
	if (c>'0'&&c<='9'){
		*trunkId= (unsigned short)strtoul(str,&endptr,10);
		if('\0' != endptr[0]){
			return -1;
		}
		return 0;	
	}
	else {
		return -1; /*not Vlan ID. for Example ,enter '.',and so on ,some special char.*/
	}
}

/*parse string to vlan netif index*/
unsigned int parse_str_to_vlan_index(char* str)
{
    char *endptr = NULL;
    unsigned short vid = 0;
    
    if(NULL == str)
      return 0;

    if(4 != strlen(str))
      return 0;
    
    vid = strtoul(str, &endptr, 10);
    if (vid < 1 || vid > 4094)
        return 0;
    
    if('\0' != endptr[0])
        return 0;
    
    return npd_netif_vlan_index(vid);
}

/*parse string to vlan id*/
int parse_vlan_no(char* str,unsigned short* vlanId) {

    *vlanId = atoi(str);
    if((*vlanId < 1) || (*vlanId > 4095))
    {
        *vlanId = -1;
        return -1;
    }
    return 0;
}

/*pare portlist string to port id array. the port list string format is
  1/1/1,1/1/2-6,1/1/10*/
#define STATE_S 1
#define STATE_SS 2
#define STATE_PS 3
#define STATE_PE 4
#define MAX_PORT_NUMBER_IN_PORTLIST 300

int port_index_compare(const void *index1, const void *index2)
{
    const unsigned long *eth_g_index1 = (const unsigned long *)index1;
    const unsigned long *eth_g_index2 = (const unsigned long *)index2;
    int ret;

    ret = (*eth_g_index1 > *eth_g_index2)-(*eth_g_index1 < *eth_g_index2);
    if(0 == ret)
        return index1 > index2;
    return ret;
}

unsigned long * parse_portlist( char * portlist )
{
    unsigned long state = STATE_S;
    unsigned long slot = 0;
    unsigned long chassis = 0;
    unsigned long sub_slot = 0;
    char digit_temp[ 12 ];
    unsigned long ulInterfaceList[ MAX_PORT_NUMBER_IN_PORTLIST ];
    unsigned long ulPortS = 0;
    unsigned long ulPortE = 0;
    char cToken;
    unsigned long iflist_i = 0;
    unsigned long list_i = 0;
    unsigned long temp_i = 0;
    unsigned long ulIfindex;
    unsigned long ulListLen = 0;
    char * list;
    unsigned long *ret_list;
    unsigned long type;
    char *endptr = NULL;
    int count = 0;

    memset(ulInterfaceList, 0, MAX_PORT_NUMBER_IN_PORTLIST * 4 );
    ulListLen = strlen( portlist );
    list = malloc( ulListLen + 2);
    if ( list == NULL )
    {
        return NULL;
    }
    strncpy( list, portlist, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];

    while ( cToken != 0 )
    {
        switch ( state )
        {
            case STATE_S:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( cToken == '/' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    chassis = (unsigned long)strtoul(digit_temp,&endptr,10);
                    temp_i = 0;
                    
                    state = STATE_SS;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_SS:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( cToken == '/' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    slot = (unsigned long)strtoul(digit_temp,&endptr,10);
                    temp_i = 0;
                    
                    state = STATE_PS;
                } 
                /*we support slot/port expression, so give a decide wether it is*/
                else if ( cToken == ',' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    chassis = 0;
                    slot = chassis;

                    ulIfindex = generate_eth_index(chassis, slot, 0, ulPortS, 1);
                    if ( 0 != ulIfindex )
                    {
                        ulInterfaceList[ iflist_i ] = ulIfindex;
                        iflist_i++;
                    }
                    if ( iflist_i >= MAX_PORT_NUMBER_IN_PORTLIST )
                    {
                        goto error;
                    }
                    temp_i = 0;
                    state = STATE_S;
                }
                else if ( cToken == '-' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    chassis = 0;
                    slot = chassis;
                    
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    temp_i = 0;
                    state = STATE_PE;
                }
                else
                {
                    goto error;
                }
                break;
                
            case STATE_PS:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if (cToken == '/')
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    sub_slot = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    /*for box system, we use chassis no as slot to generate eth index*/
                    temp_i = 0;
                    state = STATE_PS;                    
                }
                else if ( cToken == ',' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    if((chassis != local_unitno)
                        && !DCLI_SYS_ISCASCADE)
                        return 0;
                    /*for box system, we use chassis no as slot to generate eth index*/
                    ulIfindex = generate_eth_index(chassis, slot, sub_slot, ulPortS, 1);
                    if ( 0 != ulIfindex )
                    {
                        ulInterfaceList[ iflist_i ] = ulIfindex;
                        iflist_i++;
                    }
                    if ( iflist_i >= MAX_PORT_NUMBER_IN_PORTLIST )
                    {
                        goto error;
                    }
                    temp_i = 0;
                    state = STATE_S;
                }
                else if ( cToken == '-' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    temp_i = 0;
                    state = STATE_PE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_PE:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( cToken == ',' )
                {
                    unsigned long i;
                    unsigned long i_s, i_e;
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortE = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    if ( ulPortE > ulPortS )
                    {
                        i_s = ulPortS;
                        i_e = ulPortE;
                    }
                    else
                    {
                        i_s = ulPortE;
                        i_e = ulPortS;
                    }
                    if((chassis != local_unitno)
                        && !DCLI_SYS_ISCASCADE)
                        return 0;
                    /*for box system, we use chassis no as slot to generate eth index*/
                    for ( i = i_s;i <= i_e;i++ )
                    {
                        ulIfindex = generate_eth_index(chassis, slot, sub_slot, i, 1);
                        if ( 0 != ulIfindex )
                        {
                            ulInterfaceList[ iflist_i ] = ulIfindex;
                            iflist_i++;
                        }
                        if ( iflist_i >= MAX_PORT_NUMBER_IN_PORTLIST )
                        {
                            goto error;
                        }
                    }
                    temp_i = 0;
                    state = STATE_S;
                }
                else
                {
                    goto error;
                }
                break;
            default:
                goto error;
        }
        list_i++;
        cToken = list[ list_i ];
    }
    free( list );
    if ( iflist_i == 0 )
    {
        return NULL;
    }
    else
    {
        int i;
        int offset = 0;
        qsort(ulInterfaceList, iflist_i, sizeof(unsigned long), port_index_compare);
        ret_list = malloc( ( iflist_i + 1 ) * 4);
        if ( list == NULL )
        {
            goto error;
        }
        memset( ret_list, 0, ( iflist_i + 1 ) * 4 );
        ret_list[0] = ulInterfaceList[0];
        for(i = 1; i < iflist_i; i++)
        {
            if(ulInterfaceList[i] == ulInterfaceList[i-1])
            {
                offset++;
                continue;
            }
            ret_list[i-offset] = ulInterfaceList[i];
        }
        return ( unsigned long * ) ret_list;
    }
error:
    if(list)
        free( list );
    return NULL;
}

#define STATE_VS 0
#define STATE_VE 1
/*pare vlanlist string to vlan id array. the vlan list string format is
  1,2-6,10*/
int vlan_index_compare(const void *index1, const void *index2)
{
    const unsigned long *vid1 = (const unsigned long *)index1;
    const unsigned long *vid2 = (const unsigned long *)index2;
    int ret;

    ret = (*vid1 > *vid2)-(*vid1 < *vid2);
    if(0 == ret)
        return index1 > index2;
    return ret;
}
unsigned long * parse_vlanlist( char * vlanlist )
{
    unsigned long state = STATE_VS;
    char digit_temp[ 12 ];
    unsigned long vlan_list[ MAX_VLAN_IN_LIST ];
    unsigned long vlan_s = 0;
    unsigned long vlan_e = 0;
    char cToken;
    unsigned long iflist_i = 0;
    unsigned long list_i = 0;
    unsigned long temp_i = 0;
    unsigned long vid;
    unsigned long ulListLen = 0;
    char * list;
    char *endptr = NULL;

    memset(vlan_list, 0, MAX_VLAN_IN_LIST * 4 );
    ulListLen = strlen( vlanlist );
    list = malloc( ulListLen + 2);
    if ( list == NULL )
    {
        return NULL;
    }
    strncpy( list, vlanlist, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];

    while ( cToken != 0 )
    {
        switch ( state )
        {
               
            case STATE_VS:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 5 )
                    {
                        goto error;
                    }
                }
                else if ( cToken == ',' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    vlan_s = ( unsigned long )strtoul(digit_temp,&endptr,10);

                    vid = vlan_s;
					if ( iflist_i >= MAX_VLAN_IN_LIST )
                    {
                        goto error;
                    }
                    if (( 0 < vlan_s ) && (vlan_s < 4095))
                    {
                        vlan_list[ iflist_i ] = vid;
                        iflist_i++;
                    }
                    else
                        goto error;
                    temp_i = 0;
                    state = STATE_VS;
                }
                else if ( cToken == '-' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    vlan_s = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    temp_i = 0;
                    state = STATE_VE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_VE:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 5 )
                    {
                        goto error;
                    }
                }
                else if ( cToken == ',' )
                {
                    unsigned long i;
                    unsigned long i_s, i_e;
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    vlan_e = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    if ( vlan_e > vlan_s )
                    {
                        i_s = vlan_s;
                        i_e = vlan_e;
                    }
                    else
                    {
                        i_s = vlan_s;
                        i_e = vlan_e;
                    }
                    for ( i = i_s;i <= i_e;i++ )
                    {
                        vid = i;
						if ( iflist_i >= MAX_VLAN_IN_LIST )
                        {
                            goto error;
                        }
                        if (( 1 < vid)&&(vid < 4096))
                        {
                            vlan_list[ iflist_i ] = vid;
                            iflist_i++;
							if (vid == 4095)
							{
								goto error;
							}
                        } 
                    }
                    temp_i = 0;
                    state = STATE_VS;
                }
                else
                {
                    goto error;
                }
                break;
            default:
                goto error;
        }
        list_i++;
        cToken = list[ list_i ];
    }
    free( list );
    if ( iflist_i == 0 )
    {
        return NULL;
    }
    else
    {
        int i;
        int offset = 0;
        unsigned long *ret_list;
        
        qsort(vlan_list, iflist_i, sizeof(unsigned long), port_index_compare);
        ret_list = malloc( ( iflist_i + 1 ) * 4);
        if ( ret_list == NULL )
        {
            return NULL;
        }
        memset( ret_list, 0, ( iflist_i + 1 ) * 4 );
        ret_list[0] = vlan_list[0];
        for(i = 1; i < iflist_i; i++)
        {
            if(vlan_list[i] == vlan_list[i-1])
            {
                offset++;
                continue;
            }
            ret_list[i-offset] = vlan_list[i];
        }
        
        return ( unsigned long * ) ret_list;
    }
error:
    free( list );
    return NULL;
}

/*pare vlanlist string to vlan range id array. the vlan list string format is
  1,2-6,10*/
int parse_vlanlist_range( char * vlanlist,  vid_range_t *range)
{
    unsigned long state = STATE_VS;
    char digit_temp[ 12 ];
    unsigned long vlan_list[ MAX_VLAN_IN_LIST ];
    unsigned long vlan_s = 0;
    unsigned long vlan_e = 0;
    char cToken;
    unsigned long iflist_i = 0;
    unsigned long list_i = 0;
    unsigned long temp_i = 0;
    unsigned long vid;
    unsigned long ulListLen = 0;
    char * list;
    char *endptr = NULL;

    ulListLen = strlen( vlanlist );
    list = malloc( ulListLen + 2);
    if ( list == NULL )
    {
        return -1;
    }
    strncpy( list, vlanlist, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];

    while ( cToken != 0 )
    {
        switch ( state )
        {
               
            case STATE_VS:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    vlan_s = ( unsigned long )strtoul(digit_temp,&endptr,10);

                    vid = vlan_s;
                    if (( 0 < vlan_s ) && (vlan_s < 4095))
                    {
                        range[iflist_i].start = vid;
                        range[iflist_i].end = vid;
                        iflist_i++;
                    }
                    else
                        goto error;
                    if ( iflist_i >= MAX_VLAN_IN_LIST-1 )
                    {
                        goto error;
                    }
                    temp_i = 0;
                    state = STATE_VS;
                }
                else if ( cToken == '-' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    vlan_s = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    temp_i = 0;
                    state = STATE_VE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_VE:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                    unsigned long i;
                    unsigned long i_s, i_e;
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    vlan_e = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    if ( vlan_e > vlan_s )
                    {
                        i_s = vlan_s;
                        i_e = vlan_e;
                    }
                    else
                    {
                        i_s = vlan_s;
                        i_e = vlan_e;
                    }
                    range[iflist_i].start = i_s;
                    range[iflist_i].end = i_e;
                    iflist_i++;
                    if ( iflist_i >= MAX_VLAN_IN_LIST-1 )
                    {
                        goto error;
                    }
                    temp_i = 0;
                    state = STATE_VS;
                }
                else
                {
                    goto error;
                }
                break;
            default:
                goto error;
        }
        list_i++;
        cToken = list[ list_i ];
    }
    range[iflist_i].start = 0;
    range[iflist_i].end = 0;
    free( list );
    if ( iflist_i == 0 )
    {
        return -1;
    }
    else
        return 0;
error:
    free( list );
    return -1;
}


/*same as parse_vlan_no*/
int parse_subvlan_no(char* str,unsigned short* subvlanId) {
	char *endptr = NULL;

	if (NULL == str) return -1;
	*subvlanId= strtoul(str,&endptr,10);
	return 0;	
}

int parse_vlan_string(char* input){
	int i;
	 char c;
	 if(NULL == input) {
		 return -1;
	 }
	 c=input[0];
	 
	if ((c>='A'&&c<='Z')||(c>='a'&&c<='z')||('_'==c)){
	 	return 1;
		}
	return -1;
	
	 }
char *guc_LAGResvName[] = {
		"lag",
		"port-channel",
		 NULL
	};
int trunk_name_legal_check(char* str,unsigned int len)
{
	int i = 0;
	int ret = -1;
	char c = 0;
	if((NULL == str)||(len==0)){
		return ret;
	}
	if(len >= ALIAS_NAME_SIZE){
		ret = ALIAS_NAME_LEN_ERROR;
		return ret;
	}

	c = str[0];
	if(	(c=='_')||
		(c<='z'&&c>='a')||
		(c<='Z'&&c>='A')
	  ){
		ret =0;
	}
	else {
		return ALIAS_NAME_HEAD_ERROR;
	}
	for (i=1;i<=len-1;i++){
		c = str[i];
		if( (c>='0' && c<='9')||
			(c<='z'&&c>='a')||
		    (c<='Z'&&c>='A')||
		    (c=='_') || (c=='-')){
			continue;
		}
		else {
			ret =ALIAS_NAME_BODY_ERROR;
			break;
		}
	}
    
    i = 0;
	while( guc_LAGResvName[i])
	{
		if( 0 == strncmp(str, guc_LAGResvName[i], strlen(guc_LAGResvName[i])))
		{
			ret = ALIAS_NAME_RESD_ERROR;
			break;
		}
		i++;
	}
    
	return ret;
}

char *guc_VlanResvName[] = {
		"vlan", 
		"ethernet",
		"VLAN",
		"ETHERNET",
		NULL
	};


int vlan_name_legal_check(char* str,unsigned int len)
{
	int i;
	int ret = -1;
	char c = 0;
	if((NULL == str)||(len==0)){
		return ret;
	}
	if(len >= ALIAS_NAME_SIZE){
		ret = ALIAS_NAME_LEN_ERROR;
		return ret;
	}

	c = str[0];
	if(	(c=='_')||
		(c<='z'&&c>='a')||
		(c<='Z'&&c>='A')
	  ){
		ret = 0;
	}
	else {
		return ALIAS_NAME_HEAD_ERROR;
	}
	for (i=1;i<=len-1;i++){
		c = str[i];
		if( (c>='0' && c<='9')||
			(c<='z'&&c>='a')||
		    (c<='Z'&&c>='A')||
		    (c=='_')
		    ){
			continue;
		}
		else {
			ret = ALIAS_NAME_BODY_ERROR;
			break;
		}
	}
    i = 0;
	while( guc_VlanResvName[i])
	{
		if( 0 == strncmp(str, guc_VlanResvName[i], strlen(guc_VlanResvName[i])))
		{
			ret = ALIAS_NAME_RESD_ERROR;
			break;
		}
		i++;
	}
	
	return ret;
}

typedef union _eth_parse_
{
	struct 
	{
		unsigned long chassis_no;
		unsigned long slot_no;
		unsigned long module_no;
		unsigned long port_no;
		unsigned long subport_no;
	}eth_parse_unit;
	unsigned long ul_parse[5];
}eth_parse;

int parse_chassis_slot_module_port_subport_no(char *str,unsigned char *chassis, unsigned char *slotno, unsigned char *moduleno, unsigned char *portno, unsigned char *subportno)
{
	int counter = 0;
	int have_subport = 0;
	char *str_temp = NULL;
	char *endptr = NULL;
	unsigned long ul_temp[5] = {0};

	if (NULL == str) return -1;
	*chassis = 0;
	*slotno = 0;
	*moduleno = 0;
	*portno = 0;
	*subportno = 0;
	str_temp = str;
	while(1)
	{
		ul_temp[counter] = strtoul(str_temp,&endptr,10);
		if(endptr)
		{
			counter++;
			if(SLOT_PORT_SPLIT_SLASH == endptr[0])
			{
				if(have_subport)
				{
					return -1;
				}
				if(counter > 4)
				{
					return -1;
				}
				str_temp = endptr+1;
				continue;
			}
			else if('.' == endptr[0])
			{
				have_subport++;
				if(have_subport > 1)
				{
					return -1;
				}
				str_temp = endptr+1;
				continue;
			}
			else if('\0' == endptr[0])
			{
				switch(counter)
				{
					case 1:
						return -1;
					case 2:
						if(have_subport)
						{
						    return -1;
						}
						else
						{
							*slotno = (char)ul_temp[0];
							*portno = (char)ul_temp[1];
							return 0;
						}
					case 3:
						if(have_subport)
						{
							*slotno = (char)ul_temp[0];
							*portno = (char)ul_temp[1];
							*subportno = (char)ul_temp[2];
						    return 0;
						}
						else
						{
							*slotno = (char)ul_temp[0];
							*moduleno = (char)ul_temp[1];
							*portno = (char)ul_temp[2];
							return 0;
						}
					case 4:
						if(have_subport)
						{
							*slotno = (char)ul_temp[0];
							*moduleno = (char)ul_temp[1];
							*portno = (char)ul_temp[2];
							*subportno = (char)ul_temp[3];
						    return 0;
						}
						else
						{
							*chassis = (char)ul_temp[0];
							*slotno = (char)ul_temp[1];
							*moduleno = (char)ul_temp[2];
							*portno = (char)ul_temp[3];
							return 0;
						}
					case 5:
						if(have_subport)
						{
							*chassis = (char)ul_temp[0];
							*slotno = (char)ul_temp[1];
							*moduleno = (char)ul_temp[2];
							*portno = (char)ul_temp[3];
							*subportno = (char)ul_temp[4];
						    return 0;
						}
						else
						{
							return -1;
						}
					default:
						return -1;
				}
			}
			else
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}
	
	return -1;	
}

unsigned int parse_str_to_eth_index(char *str)
{
	int chassis = local_unitno, slotno = 0, moduleno = 0, portno = 0, subportno = 1;
	int counter = 0;
	int have_subport = 0;
	char *str_temp = NULL;
	char *endptr = NULL;
	unsigned long ul_temp[5] = {0};

	if (NULL == str) return 0;
	str_temp = str;
	while(1)
	{
		ul_temp[counter] = strtoul(str_temp,&endptr,10);
		if(endptr)
		{
			counter++;
			if(SLOT_PORT_SPLIT_SLASH == endptr[0])
			{
				if(have_subport)
				{
					return 0;
				}
				if(counter > 4)
				{
					return 0;
				}
				str_temp = endptr+1;
				continue;
			}
			else if('.' == endptr[0])
			{
				have_subport++;
				if(have_subport > 1)
				{
					return 0;
				}
				str_temp = endptr+1;
				continue;
			}
			else if('\0' == endptr[0])
			{
				switch(counter)
				{
					case 1:
						return 0;
					case 2:
						if(have_subport)
						{
						    return 0;
						}
						else
						{
							slotno = (char)ul_temp[0];
							portno = (char)ul_temp[1];
							return generate_eth_index(chassis, slotno, moduleno, portno, subportno);
						}
					case 3:
						if(have_subport)
						{
							slotno = (char)ul_temp[0];
							portno = (char)ul_temp[1];
							subportno = (char)ul_temp[2];
							return generate_eth_index(chassis, slotno, moduleno, portno, subportno);
						}
						else
						{
							chassis = (char)ul_temp[0];
                            if((chassis != local_unitno)
                                && !DCLI_SYS_ISCASCADE)
                                return 0;
							slotno = (char)ul_temp[1];

                            /*for box system, we use chassis no as slot to generate eth index*/
							portno = (char)ul_temp[2];
							return generate_eth_index(chassis, slotno, moduleno, portno, subportno);
						}
					case 4:
						if(have_subport)
						{
							chassis = (char)ul_temp[0];
                            if((chassis != local_unitno)
                                && !DCLI_SYS_ISCASCADE)
                                return 0;
							slotno = (char)ul_temp[1];
							portno = (char)ul_temp[2];
							subportno = (char)ul_temp[3];
							return generate_eth_index(chassis, slotno, moduleno, portno, subportno);
						}
						else
						{
							chassis = (char)ul_temp[0];
                            if((chassis != local_unitno)
                                && !DCLI_SYS_ISCASCADE)
                                return 0;
                            
							slotno = (char)ul_temp[1];
							moduleno = (char)ul_temp[2];
							portno = (char)ul_temp[3];
							return generate_eth_index(chassis, slotno, moduleno, portno, subportno);
						}
					case 5:
						if(have_subport)
						{
							chassis = (char)ul_temp[0];
                            if((chassis != local_unitno)
                                && !DCLI_SYS_ISCASCADE)
                                return 0;
							slotno = (char)ul_temp[1];
							moduleno = (char)ul_temp[2];
							portno = (char)ul_temp[3];
							subportno = (char)ul_temp[4];
							return generate_eth_index(chassis, slotno, moduleno, portno, subportno);
						}
						else
						{
							return 0;
						}
					default:
						return 0;
				}
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	
	return 0;	
}

unsigned int parse_intf_name_to_eth_index(char *str)
{
	int chassis = 0, slotno = 0, moduleno = 0, portno = 0, subportno = 0;
	int counter = 0;
	int have_subport = 0;
	char *str_temp = NULL;
	char *endptr = NULL;
	unsigned long ul_temp[5] = {0};

	if (NULL == str) return 0;
	str_temp = str;

	if(0 == strncmp(str,"eth",3))
	{
		str_temp = str+3;
		return parse_str_to_eth_index(str_temp);
		
	}
	
	return 0;	
}

int parse_slotport_no(char *str,unsigned char *slotno,unsigned char *portno) {
	char *endptr = NULL;
	char *endptr2 = NULL;

	if (NULL == str) return -1;
	*portno = strtoul(str,&endptr,10);
	if (endptr) {
		if ((SLOT_PORT_SPLIT_DASH == endptr[0])||(SLOT_PORT_SPLIT_SLASH == endptr[0])) {
            *slotno = *portno;
			*portno = strtoul((char *)&(endptr[1]),&endptr2,10);
			if('\0' == endptr2[0]) {
				return 0;
			}
			else {
				return -1;
			}
		}
		if ('\0' == endptr[0]) {
			*slotno = 0;
			return 0;
		}
	}
	return -1;	
}

int parse_slotport_tag_no(char *str,unsigned char *slotno,unsigned char *portno,unsigned int * tag1,unsigned int *tag2)
{
	char *endptr = NULL;
	char *endptr2 = NULL;
	char *endptr3 = NULL;
	char *endptr4 = NULL;
	char * tmpstr = str;
	
	if(NULL == str){
		return -1;
	}
	if((NULL == slotno)||(NULL == portno)||(NULL == tag1)){
		return -1;
	}
	*slotno = 0;
	*portno = 0;
	*tag1 = 0;
	if(NULL == tag2){
		*tag2 = 0;
	}
	if(0 == strncmp(str,"eth",3)){
		tmpstr = str+3;
	}
	if (NULL == tmpstr) {return -1;}
	if(((tmpstr[0] == '0')&&(tmpstr[1] != SLOT_PORT_SPLIT_DASH))|| \
		(tmpstr[0] > '9')||(tmpstr[0] < '0')){
         return -1;
	}
	*portno = (char)strtoul(tmpstr,&endptr,10);
	if (endptr) {
		if ((SLOT_PORT_SPLIT_DASH == endptr[0])&& \
			(('0' < endptr[1])&&('9' >= endptr[1]))){
            *slotno = *portno;
			*portno = (char)strtoul((char *)&(endptr[1]),&endptr2,10);
			/*}
			for eth1, eth1.2, eth1.2.3
			else endptr2 = endptr;
			{
			*/
			if(endptr2){	
				if('\0' == endptr2[0]){
					*tag1 = 0;
					return 0;
				}
				else if(('.' == endptr2[0])&&\
					(('0' < endptr2[1])&&('9' >= endptr2[1]))){
					*tag1 = strtoul((char *)&(endptr2[1]),&endptr3,10);
					if((NULL == endptr3)||('\0' == endptr3[0])){
						if(tag2) *tag2 = 0;
						return 0;
					}
					if(!tag2) return -1;
					if((endptr3 != NULL)&&(endptr3[0] == '.')){
						if(('0' >= endptr3[1])||('9' < endptr3[1])){
							return -1;
						}
						if(tag2){
							*tag2 = strtoul((char *)&(endptr3[1]),&endptr4,10);
							if((endptr4 != NULL)&&(endptr4[0] != '\0')){
								return -1;
							}
						}
						return 0;
					}
					return -1;
					
				}
				else{
					*tag1 = 0;
					return -1;
				}
			}
			
			*tag1 = 0;
			if(tag2) *tag2 = 0;
			return 0;
		}
	}
	*slotno = 0;
	*tag1 = 0;
	if(tag2) *tag2 = 0;
	/* return 0;*/
	return -1;	
}

typedef struct _interface_parse_name_
{
    char *name;
    unsigned int (*parse_handler)(char *str);
} interface_parse_name;

interface_parse_name intf_parse_name[] =
{
    {"eth", &parse_str_to_eth_index},
    {"lag", &parse_str_to_bond_index},
    {"vlan", &parse_str_to_vlan_index},
    {NULL, NULL}
};
/*
	char bond_name[MAXLEN_BOND_CMD] = {0};
*/



unsigned int parse_intf_name_to_netif_index(unsigned char *str)
{
    int i = 0;
    int name_pre_len = 0;

    for (i = 0; ; i++)
    {
        if (intf_parse_name[i].name != NULL && intf_parse_name[i].parse_handler != NULL)
        {
            name_pre_len = strlen(intf_parse_name[i].name);

            if (strncasecmp(str, intf_parse_name[i].name, name_pre_len) == 0)
            {
                return (*intf_parse_name[i].parse_handler)(str+name_pre_len);
            }
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

void dcli_netif_name_convert_to_interface_name(char *netif_name, char *ifname)
{
	int i = 0;
	if(netif_name && ifname)
	{
	    if(strncmp(netif_name,"loopback0",strlen("loopback0")) == 0)
	    {
	    	strcpy(ifname,"lo");
            return;
		}
		
	    while(netif_name[i])
	    {
			if(netif_name[i] == '/')
			{
				ifname[i] = '_';
			}
			else if(netif_name[i] == '\r' || netif_name[i] == '\n')
			{
				ifname[i] = '\0';
			}
			else
			{
				ifname[i] = netif_name[i];
			}
			i++;
	    }
		ifname[i] = '\0';
	}
}


void dcli_interface_name_convert_to_netif_name(char *ifname, char *netif_name)
{
	int i = 0;
	if(netif_name && ifname)
	{
	    if(strncmp(ifname, "lo", strlen("lo")) == 0)
	    {
	    	strcpy(netif_name,"loopback0 ");
            return;
		}
		
	    while(ifname[i])
	    {
			if(ifname[i] == '_')
			{
				netif_name[i] = '/';
			}
			else if(ifname[i] == '\r' || ifname[i] == '\n')
			{
				netif_name[i] = '\0';
			}
			else
			{
				netif_name[i] = ifname[i];
			}
			i++;
	    }
		netif_name[i] = '\0';
	}
}

int parse_slotno_localport(char* str,unsigned int *slotno,unsigned int *portno) 
{
	char *endptr = NULL;
	char c = 0;
	if (NULL == str) return -1;
	c = str[0];
	if (c>'0' && c<='9'){
		*slotno= strtoul(str,&endptr,10);
		if('/' != endptr[0]){
            *slotno = -1;
            *portno = -1;
		    return -1;
		}
		else {
             *portno = strtoul(&endptr[1],&endptr,10);
             if('\0' != endptr[0])
			 	return 1;
        }
		return 0;	
	}
	else {
		return -1; /*not Vlan ID. for Example ,enter '.',and so on ,some special char.*/
	}
}

/*
*MAC addr parse from string
*/
long GetMacAddr( char * string, char * pucMacAddr )
{
    char * p = NULL, *q = NULL, *pcTmp = NULL;
    char cTmp[ 3 ];
    unsigned long i = 0, j = 0, k = 0;

    /* 检查字符串长度是否正常 */
    if ( 14 != strlen( string ) )
    {
        return -1;
    }

    p = string;
    for ( i = 0; i < 3; i++ )
    {
        if ( i != 2 )
        {
            /* 查看有无'.' */
            q = strchr( p, '.' );
            if ( NULL == p )
            {
                return -1;
            }
        }
        else
        {
            q = string + strlen( string );
        }

        /* 一个H不是4个字符 */
        if ( 4 != q - p )
        {
            return -1;
        }
        /* 检查是否是16进制的数字 */
        for ( j = 0; j < 4; j++ )
        {
            if ( !( ( *( p + j ) >= '0' && *( p + j ) <= '9' ) || ( *( p + j ) >= 'a' && *( p + j ) <= 'f' )
                    || ( *( p + j ) >= 'A' && *( p + j ) <= 'F' ) ) )
            {
                return -1;
            }
        }

        cTmp[ 0 ] = *p;
        cTmp[ 1 ] = *( p + 1 );
        cTmp[ 2 ] = '\0';
        pucMacAddr[ k ] = ( char ) strtoul( cTmp, &pcTmp, 16 );
        k++;

        cTmp[ 0 ] = *( p + 2 );
        cTmp[ 1 ] = *( p + 3 );
        cTmp[ 2 ] = '\0';
        pucMacAddr[ k ] = ( char ) strtoul( cTmp, &pcTmp, 16 );
        k++;

        p = q + 1;
    }

    /* 判断是否全部为0 */
    if ( 0x0 == pucMacAddr[ 0 ] && 0x0 == pucMacAddr[ 1 ] && 0x0 == pucMacAddr[ 2 ]
            && 0x0 == pucMacAddr[ 3 ] && 0x0 == pucMacAddr[ 4 ] && 0x0 == pucMacAddr[ 5 ] )
    {
        return -1;
    }

    /* 判断是否全部为ff */
    if ( (char)0xff == pucMacAddr[ 0 ] && (char)0xff == pucMacAddr[ 1 ] && (char)0xff == pucMacAddr[ 2 ]
            && (char)0xff == pucMacAddr[ 3 ] && (char)0xff == pucMacAddr[ 4 ] && (char)0xff == pucMacAddr[ 5 ] )
    {
        return -1;
    }

    /* 判断是否为多播 */
    if ( 0 != ( pucMacAddr[ 0 ] & 0x01 ) )
    {
        return -1;
    }

    return 0;

}

/*****************************************************************
   Function Name	: GetMacStr
   Processing		: 根据Mac 得到<H.H.H>
   Input Parameters	: pucMacAddr--Mac地址
   Output Parameters : string--H.H.H的字符串
   Return Values	: SSL_E_NONE成功，SSL_ERROR错误
******************************************************************
   History
   Data            Author      Modification 
******************************************************************/
long GetMacStr( char * pucMacAddr , char * string)
{
	if ( pucMacAddr == NULL || pucMacAddr == NULL )
	{
		assert( 0 );
		return -1;
	}
	
	sprintf( string , "%2.2X%2.2X.%2.2X%2.2X.%2.2X%2.2X" ,
				  *( pucMacAddr ) , *( pucMacAddr + 1 ) , *( pucMacAddr + 2 ) ,
				  *( pucMacAddr + 3 ) , *( pucMacAddr + 4 ) , *( pucMacAddr + 5 ) );

	return 0;
}


int is_muti_brc_mac(ETHERADDR *mac)
{
  if(mac->arEther[0] & 0x1)
  	return TRUE;
  else
    return FALSE;
}

int is_zero_mac(ETHERADDR *mac)
{
    return ((mac->arEther[0] == 0)    
        && (mac->arEther[1] == 0)
        && (mac->arEther[2] == 0)
        && (mac->arEther[3] == 0)
        && (mac->arEther[4] == 0)
        && (mac->arEther[5] == 0)
        );
}

int is_bcast_mac(ETHERADDR *mac)
{
    return ((mac->arEther[0] == 0xff)    
        && (mac->arEther[1] == 0xff)
        && (mac->arEther[2] == 0xff)
        && (mac->arEther[3] == 0xff)
        && (mac->arEther[4] == 0xff)
        && (mac->arEther[5] == 0xff)
        );
}

int is_mcast_mac(ETHERADDR *mac)
{
    return ((mac->arEther[0] == 0x01)    
        && (mac->arEther[1] == 0x00)
        && (mac->arEther[2] == 0x5e)
        );
}


int mac_format_check
(
	char* str,
	int len
) 
{
	int i = 0;
	unsigned int result = 0;
	char c = 0;
	
	if( 17 != len){
	   return -1;
	}
	for(;i<len;i++) {
		c = str[i];
		if((2 == i)||(5 == i)||(8 == i)||(11 == i)||(14 == i)){
			if((':'!=c)&&('-'!=c))
				return -1;
		}
		else if((c>='0'&&c<='9')||
			(c>='A'&&c<='F')||
			(c>='a'&&c<='f'))
			continue;
		else {
			result = -1;
			return result;
		}
    }
	if((str[2] != str[5])||(str[2] != str[8])||(str[2] != str[11])||(str[2] != str[14])||
		(str[5] != str[8])||(str[5] != str[11])||(str[5] != str[14])||
		(str[8] != str[11])||(str[8] != str[14])){
		
        result = -1;
		return result;
	}
	return result;
}


/*here str space must greater than 17*/
int mac_to_str(ETHERADDR *mac, char *str)
{
    sprintf(str, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
       mac->arEther[0], mac->arEther[1],mac->arEther[2],
       mac->arEther[3],mac->arEther[4],mac->arEther[5]);
    return 0;
}

/*
* ip addr string to 
*/
long mlib_ipv4_prefix_match ( char * str )
{
    char * sp;
    long dots = 0;
    char buf[ 4 ];

    if ( str == NULL )
        return -1;
    for ( ;; )
    {
        memset ( ( char * ) buf, 0, sizeof ( buf ) );
        sp = str;
        while ( *str != '\0' && *str != '/' )
        {
            if ( *str == '.' )
            {
                if ( dots == 3 )
                    return -1;

                if ( *( str + 1 ) == '.' || *( str + 1 ) == '/' )
                    return -1;

                if ( *( str + 1 ) == '\0' )
                    return -1;
                dots++;
                break;
            }

            if ( !isdigit ( ( long ) * str ) )
                return -1;

            str++;
        }

        if ( str - sp > 3 )
            return -1;

        strncpy ( buf, sp, ( unsigned long ) ( str - sp ) );
        if ( atoi ( buf ) > 255 )
            return -1;

        if ( dots == 3 )
        {
            if ( *str == '/' )
            {
                if ( *( str + 1 ) == '\0' )
                    return -1;
                str++;
                break;
            }
            else if ( *str == '\0' )
                return 0;
        }

        if ( *str == '\0' )
            return 0;
        str++;
    }

    sp = str;
    while ( *str != '\0' )
    {
        if ( !isdigit ( ( long ) * str ) )
            return -1;

        str++;
    }

    if ( atoi ( sp ) > 32 )
        return -1;

    return 0;
}

long CheckMaskAddr( unsigned long ulMask )
{
    unsigned long ulTmp = ntohl( ulMask );
    long lMov = 30 ; /* 移位操作的初始位数 */
    unsigned long ulTmpMask = 3; /* 用来获得2位的掩码 */

    while ( lMov != -1 )
    {
        if ( ( ( ulTmp & ( ulTmpMask << lMov ) ) >> lMov ) == 1 )
            return -1;
        lMov = ( long ) ( lMov - 1 );
    }
    return 0;
}

long IsSubnetBroadcast( unsigned long ulIpAddr, unsigned long ulMask )
{
    unsigned long ulTmpValue = 0;

    if ( CheckMaskAddr( ulMask ) != 0 )
        return -1;

    /* 说明是合法的掩码 */
    if ( ( ulMask == 0xFFFFFFFF ) || ( ulMask == 0xFFFFFFFE ) )
        return -1;
    ulTmpValue = ( ntohl( ulMask ) )  & ( ntohl( ulIpAddr ) );
    if ( ulTmpValue == ~( ntohl( ulMask ) ) )
        return 0;
    else
        return -1;
}

/*将A.B.C.D/M 解析成 ip ,mask */
/*例如:1.2.3.4/24,最后返回内存(网络字节序),0x01,0x02,0x03,0x04/0xff,0xff,0xff,0x00*/
long IpListToIp( const char * IpList, long * ulIpAddr, long * ulMask )
{
    char bak[ 20 ];
    int len, pos;
    char *strMask = NULL;
    char *strIpaddr = NULL;
    char *Mask = NULL;
    int ipaddr, mask, masklen;

    len = strlen( IpList );
    if ( len > 18 )
    {
        return -1;
    }
    strncpy( bak, IpList, len );

    strMask = ( char * ) strchr( bak, '/' );
    if ( !strMask )
    {
        assert( 0 );
        return -1;
    }
    pos = strMask - bak;
    if ( pos < 0 )
    {
        return -1;
    }
    strIpaddr = ( char * ) malloc( pos + 1);
    if ( !strIpaddr )
    {
        assert( 0 );
        return -1;
    }

    memset( strIpaddr, 0, pos + 1 );
    if ( ( len - pos ) <= 0 )
    {
        if ( strIpaddr )
            free( strIpaddr );
        return -1;
    }
    Mask = ( char * ) malloc( len - pos);
    if ( !Mask )
    {
        assert( 0 );
        if ( strIpaddr )
            free( strIpaddr );
        return -1;
    }

    memset( Mask, 0, ( len - pos ) );

    if ( strMask )
    {
        memcpy( strIpaddr, bak, pos );
        memcpy( Mask, strMask + 1, len - pos - 1 );
    }
    ipaddr = lib_get_ip_from_string( strIpaddr );

    if ( strIpaddr )
    {
        free( strIpaddr );
        strIpaddr = NULL;
    }

    if ( ipaddr == -1 )
    {
        if ( Mask )
            free( Mask );
        Mask = NULL;
        return -1;
    }

    if ( strlen( Mask ) >= 7 )
    {
        masklen = lib_get_masklen_from_string( Mask );
        if ( masklen <= 0 )
        {
            if ( Mask )
            {
                free( Mask );
                Mask = NULL;
            }
            return -1;
        }
    }
    else
    {
        masklen = atoi( Mask );
        if ( masklen <= 0 )
        {
            if ( Mask )
            {
                free( Mask );
                Mask = NULL;
            }
            return -1;
        }
    }

    if ( Mask )
    {
        free( Mask );
        Mask = NULL;
    }

	lib_get_mask_from_masklen(masklen, &mask);

    *ulIpAddr = ipaddr;
    *ulMask = mask;
    return 0;
}


int parse_param_no(char* str,unsigned int* param)
{
    char* endptr = NULL;

    if (NULL == str)
        return -1;

    *param = strtoul(str,&endptr,10);
    return 0;
}

#define DCLI_IPMASK_STRING_MAXLEN	(strlen("255.255.255.255/32"))
#define DCLI_IPMASK_STRING_MINLEN	(strlen("0.0.0.0/0"))
#define DCLI_IP_STRING_MAXLEN	(strlen("255.255.255.255"))
#define DCLI_IP_STRING_MINLEN   (strlen("0.0.0.0"))


int
inet_atoi (const char *cp, unsigned int  *inaddr)
{
    struct in_addr in;
    int ret;
    
    ret = inet_pton(AF_INET, cp, &in);

    *inaddr = in.s_addr;
    return ret;
    
}

unsigned long dcli_ip2ulong(char *str)
{
    struct in_addr in;
    int ret;
    
    ret = inet_pton(AF_INET, str, &in);
    if(1 != ret)
        return -1;
    return in.s_addr;
}

int parse_ip_check(char * str)
	{
	
		char *sep=".";
		char *token = NULL;
		unsigned long ip_long[4] = {0}; 
		int i = 0;
		int pointCount=0;
		char ipAddr[30]={0};
		if(str==NULL||strlen(str)>DCLI_IP_STRING_MAXLEN || \
			strlen(str) < DCLI_IP_STRING_MINLEN ){
			return 1;
		}
		if((str[0] > '9')||(str[0] < '1')){
			return 1;
		}
		for(i=0;i<strlen(str);i++){
			ipAddr[i]=str[i];
			if('.' == str[i]){
                pointCount++;
				if((i == strlen(str)-1)||('0' > str[i+1])||(str[i+1] > '9')){
					return 1;
				}
			}
			if((str[i]>'9'||str[i]<'0')&&str[i]!='.'&&str[i]!='\0'){
				return 1;
			}
		}
		if(3 != pointCount){
            return 1;
		}
		token=strtok(ipAddr,sep);
		if((NULL == token)||("" == token)||(strlen(token) < 1)||\
			((strlen(token) > 1) && ('0' == token[0]))){
			return 1;
		}
		if(NULL != token){
		    ip_long[0] = strtoul(token,NULL,10);
		}
		else {
			return 1;
		}
		i=1;
		while((token!=NULL)&&(i<4))
		{
			token=strtok(NULL,sep);
			if((NULL == token)||("" == token)||(strlen(token) < 1)|| \
				((strlen(token) > 1) && ('0' == token[0]))){
				return 1;
			}
			if(NULL != token){
			    ip_long[i] = strtoul(token,NULL,10);
			}
			else {
				return 1;
			}
			i++;
		}
		for(i=0;i<4;i++){
            if(ip_long[i]>255){
                return 1;
			}
		}
		return 0;
		
}

int	ip_address_format2ulong(char ** buf,unsigned long *ipAddress,unsigned int *mask)
{
    int ret;

    ret = IpListToIp(*buf, (long*)ipAddress, (long*)mask);

    if(-1 == ret)
        return 1;

    return 0;
}
/*
 *parse_ip_check
 */

/**********************************************************************************
*  ip_long2str
*
*  DESCRIPTION:
*	 convert ip (ABCD) format to (A.B.C.D)
*
*  INPUT:
*	 ipAddress - string (ABCD)
*  
*  OUTPUT:
*	  null
*
*  RETURN:
*	  buff - ip (A.B.C.D)
*	  
**********************************************************************************/
unsigned int ip_long2str(unsigned long ipAddress,unsigned char **buff)
{

    unsigned long	 cnt = 0;
    unsigned char *tmpPtr = *buff;
    unsigned long	 ipAddr = ntohl(ipAddress);

    cnt = sprintf((char*)tmpPtr,"%ld.%ld.%ld.%ld",(ipAddr>>24) & 0xFF, \
    	 (ipAddr>>16) & 0xFF,(ipAddr>>8) & 0xFF,ipAddr & 0xFF);

    return cnt;
}

int parse_ipaddr_withmask(
    char * str,
    unsigned long *ip_addr,
    unsigned int *mask
    )
{
    return IpListToIp(str, (long*)ip_addr, (long*)mask);
}

int parse_ip_addr(
    char * str,
    unsigned long *ip_addr
    )
{
    *ip_addr = lib_get_ip_from_string(str);
    if(*ip_addr == -1)
        return -1;
    return 0;
}

int parse_int(char* str, unsigned int* ID)
{
	char *endptr = NULL;
    
	if (str[0] >= '0' && str[0] <= '9')
	{
		*ID = strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}


int parse_char_ID(char* str,unsigned char* ID)
{
	char *endptr = NULL;
	char c;
	unsigned int ret = 0;
	c = str[0];
	if (c>='1'&&c<='9'){
		ret = strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
		{
			if(ret > 255){
				*ID = 0;
			}else{
				*ID = ret;
			}
			return 0;
		}
		else
			return -1;
	}
	else
		return -1;
	
}

int parse_short_ID(char* str,unsigned short* ID){
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
			return 0;
		else
			return -1;
	}
	else
		return -1;
	
}

int parse_int_ID(char* str,unsigned int* ID)
{
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='1'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
			return 0;
		else
			return -1;
	}
	else
		return -1;	
}

int web_checkPoint(char* ptr)
{
	int ret = 0;
	while (*ptr != '\0') 
	{
		if (((*ptr) < '0') || ((*ptr) > '9'))
		{
			ret = 1;
	 		break;
		}
		ptr++;
	}
	
	return ret;
}

int web_str2ulong(char* str, unsigned int* Value)
{
	char *endptr = NULL;
	char c = 0;
	int ret = 0;
	
	if (NULL == str) 
	{
		return FALSE;
	}

	ret = web_checkPoint(str);
	if (ret == 1)
	{
		return FALSE;
	}

	c = str[0];
	if ((strlen(str) > 1) && ('0' == c))
	{
		return FALSE;
	}
	
	*Value= strtoul(str, &endptr, 10);
	if ('\0' != endptr[0])
	{
		return FALSE;
	}
	
	return TRUE;	
}

int  man_web_interface_list_get(char * ifname[], int * ifnum)
{
	FILE * ft;
    int ni = 0;
    int ifname_len = 0;
    int ifname_eth0_len = strlen("eth0-1");
	char syscommand[256];
    char temp[32];
    
    memset(syscommand, 0, sizeof(syscommand));
	memset(temp, 0, sizeof(temp));
    
	sprintf(syscommand,"ip addr | awk 'BEGIN{FS=\":\";RS=\"(\^|\\n)[0-9]+:[ ]\"}{print $1}' | awk 'NR==2,NR==0{print}' ");
	ft = popen(syscommand, "r");

	if (NULL != ft)
	{
		while (NULL != (fgets(temp, 18, ft)))
		{
            ifname_len = strlen(temp);
			if (((ifname_len == ifname_eth0_len) && (0 == strncmp(temp, "eth", strlen("eth"))))
                || (0 == strncmp(temp, "mng", strlen("mng")))
				|| (0 == strncmp(temp, "ctrl", strlen("ctrl")))
				|| (0 == strncmp(temp, "lo", strlen("lo")))
				|| (0 == strncmp(temp, "pimreg", strlen("pimreg")))
				|| (0 == strncmp(temp, "sit", strlen("sit")))
				|| (0 == strncmp(temp, "ip6tnl", strlen("ip6tnl"))))
			{
				memset(temp, 0, sizeof(temp));
				continue;
			}

            dcli_interface_name_convert_to_netif_name(temp, ifname[ni]);
    		ni++;
			
			memset(temp, 0, sizeof(temp));
		}
        pclose(ft);
	}
    
	*ifnum = ni;
    
	return 0;
}


int man_exec_shell(char *command, unsigned char *filter, unsigned char *retbuf)
{
	FILE *fd;
	int ret = -1;
	char  temp[128];
	memset(temp,0,128); 
	
	if(NULL == (fd = popen(command,"r")))
	{		
		return ret;
	}	

	while((fgets(temp,128,fd)) != NULL)
	{
		if(filter && (strstr(temp,filter) == NULL))
			continue;
		
		strcat(retbuf,temp);			
		ret = 0;		
	}
	fclose(fd);
	return ret;
}

#ifdef HAVE_WCPSS

int read_ac_info(char *FILENAME,char *buff)
{
    int len,fd;
    fd = open(FILENAME,O_RDONLY);

    if (fd < 0)
    {
        return 1;
    }

    len = read(fd,buff,256);

    if (len < 0)
    {
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}

int get_dir_wild_file_count(char *dir, char *wildfile)
{
    DIR *dp = NULL;
    struct dirent *dirp;
    int wildfilecount = 0;
    dp = opendir(dir);

    if (dp == NULL)
    {
        return wildfilecount;
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        //printf("dirname = %s count = %d\n",dirp->d_name,wildfilecount);
        if ((memcmp(dirp->d_name,wildfile,strlen(wildfile))) ==  0)
        {
            wildfilecount++;
            //printf("count = %d\n",wildfilecount);
        }
    }

    //printf("last count = %d\n",wildfilecount);
    closedir(dp);
    return wildfilecount;
}

#endif

#ifdef __cplusplus
}
#endif

