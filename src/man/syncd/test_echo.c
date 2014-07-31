/* UDP echo server program */

#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/file.h>
#include<sys/msg.h>
#include<sys/ipc.h>
#include<time.h>
#include<linux/if_ether.h> 
#include<netinet/ip.h>
#include<netinet/udp.h>
#include<net/if.h> 
#include<sys/ioctl.h> 
#include<netpacket/packet.h>
#include "board/ts_product_feature.h"
#include"test_echo.h"

static void
die (const char *failure)
{
  fprintf (stderr, "Test failed: %s\n", failure);
  exit (1);
}

int state_file_read(char *filename)
{
	int state_fd = -1;
	char buf[4] = {0};
	int ret = -1;
    int value = 0;

    state_fd = open(filename,O_RDONLY);
    if(state_fd < 0) 
    {
        return -1;
    }

    ret = read(state_fd,buf,2);
    if(ret > 0)
    {
        buf[ret] = 0;
        value = atoi(buf);
        close(state_fd);
        state_fd = -1;
        return value;
    }
    else
    {
        close(state_fd);
        state_fd = -1;
        return -1;
    }
	return ret;
}

int property_file_read(char *filename,char *property)
{
	int property_fd = -1;
	int ret = -1;
    property_fd = open(filename,O_RDONLY);
    if(property_fd < 0) 
    {
        return -1;
    }
    ret = read(property_fd,property,4);
    close(property_fd);
    property_fd = -1;
    if(ret > 0)
    {       
        return 0;
    }
    else
    {
        return -1;
    }
}

int app_slot_work_mode_get()
{
    return state_file_read(CHASSIS_MAN_MASTER_STATE_FILE);
}
int app_slot_num_get()
{
    return state_file_read(CHASSIS_MAN_SLOT_NUM_FILE);
}
int app_enterprise_name_get(char *enterprise)
{
    return property_file_read(CHASSIS_MAN_ENTERPRISE_NAME_FILE,enterprise);
}
int app_product_name_get(char *pname)
{
    return property_file_read(CHASSIS_MAN_PRODUCT_NAME_FILE,pname);
}

int app_board_type_get()
{
	int ret;
	ret = state_file_read(CHASSIS_MAN_BOARD_TYPE_FILE);
	if(-1 == ret)
		return 0;
	else
		return ret;
}

int app_act_master_running()
{
    return state_file_read(CHASSIS_MAN_ACTMASTER_STATE_FILE);
}

int  app_local_slot_get()
{
    return state_file_read(CHASSIS_MAN_SLOT_NO_FILE);
}

int app_actmaster_slot_get()
{
    return state_file_read(CHASSIS_MAN_ACTMASTER_SLOT_FILE);
}

int app_sbymaster_slot_get()
{
    return state_file_read(CHASSIS_MAN_SBYMASTER_SLOT_FILE);
}
int app_ctrl_num_get()
{
	return state_file_read(CHASSIS_MAN_CTRL_NUM_FILE);
}
int app_ctrl_switch_get()
{
	return state_file_read(CHASSIS_MAN_CTRL_SWITCH_FILE);
}
int app_ctrl_pre_slot_num_get()
{
	return state_file_read(CHASSIS_MAN_PRE_SLOT_NUM_FILE);
}
char test_buffer1[64] = 
{
    0xff,0xff,0xff,0xff,0xff,0xff,
    0x00,0x00,0x00,0x00,0x00,0x11,
    0x99,0x99,0x00,0x01,0x08,0x00,
    0x06,0x04,0x00,0x01,0x00,0x00,
    0x00,0x00,0x00,0xe0,0x10,0xc0,
    0xa8,0x03,0xea,0x00,0x00,0x00,
    0x00,0x00,0x00,0xc0,0xa8,0x00,
    0x01,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00
};
char test_buffer2[64] = 
{
    0xff,0xff,0xff,0xff,0xff,0xff,
    0x00,0x00,0x00,0x00,0x00,0x22,
    0x99,0x99,0x00,0x01,0x08,0x00,
    0x06,0x04,0x00,0x01,0x00,0x00,
    0x11,0x11,0x11,0x11,0x11,0x11,
    0xa8,0x03,0xea,0x00,0x00,0x00,
    0x00,0x00,0x00,0xc0,0xa8,0x00,
    0x01,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00
};
/*
程序为工装设计，主控板会通过switch给工装广播UDP报文
工装板会收到(总槽位数-2(两块主控板))个报文
当slave_num=-1,时表示正常环境测试
否则认为是在工装环境下测试
mode 表示测试模式，单纯业务板测试时将ctrl口环回发送测试
主被通信ctrl口测试时需要主被相互发送报文测试
*/
#define RAW_PROTOCOL 0x9999 

int safeguard_client(int mode, int pre_slot_num) 
{
	int snd_seq = 0;
	int ret = 0;
	int sock_rcv0 = 0;
	int sock_rcv1 = 0;
	int slot_num;
	slot_num = app_slot_num_get();
	struct ifreq ifr0;
	struct sockaddr_ll	dest0;

	if((sock_rcv0 = socket(PF_PACKET, SOCK_RAW, htons(RAW_PROTOCOL))) == -1)
		die("socket recv");
	bzero(&dest0,sizeof(dest0));
	
	strcpy( ifr0.ifr_name, "eth0" );  

	if ( ioctl(sock_rcv0, SIOCGIFFLAGS, &ifr0) < 0 )  
	{  
		die("get flag error");
	}  
	if(ioctl(sock_rcv0, SIOCGIFINDEX, &ifr0) < 0 )
	{
		die("get index error");
	}
	dest0.sll_family = AF_PACKET;
	dest0.sll_ifindex = ifr0.ifr_ifindex;
	dest0.sll_protocol = htons(RAW_PROTOCOL);
	if(bind(sock_rcv0,(struct sockaddr*)&dest0,sizeof(dest0)) == -1)
	{
		die("bind");
	}

    	
	for(snd_seq = 1; snd_seq <= 100; snd_seq++)
	{
		test_buffer1[14]= snd_seq;
		ret = sendto(sock_rcv0, (char *)test_buffer1, 64, 0,(struct sockaddr *)&dest0, sizeof(dest0));
		//fprintf(stdout,"client eth0:send seq_id = %d\n", test_buffer1[14]);
		if(ret < 0)
		{
			die("Internal ctrl port send error");
		}
		usleep(10000);
	}
 	
	return 0;
}

int safeguard_server(int mode, int pre_slot_num)
{
	int ret = 0;
	int sock_rcv0 = 0;
	int sock_rcv1 = 0;
	int count = 0;
	char buf[2*32767] = {0}; 
	int seq_id = 0;
	int rcv_flag[102] = {0};
	int lost_msg = 0;
	int rcv_msg = 0;
	int slot_num = 0;
	int i;
		
	slot_num = app_slot_num_get();

	FILE *fp;
	char path[32];
	sprintf(path, "/var/run/ctrl_test.bin");
	fp = fopen(path, "w+");
	if(fp == NULL)
	{
		die("Unable to open file ctrl_test.bin\n");
	}
	
	struct ifreq ifr0;
	struct sockaddr_ll	dest0;
	struct packet_mreq	mr0;
	
	if((sock_rcv0 = socket(PF_PACKET, SOCK_DGRAM, htons(RAW_PROTOCOL))) == -1)
		die("socket recv");
	
	bzero(&dest0,sizeof(dest0));

	strcpy( ifr0.ifr_name, "eth0" );
	
	if ( ioctl(sock_rcv0, SIOCGIFFLAGS, &ifr0) < 0 )  
	{  
		die("get flag error");
	}  
	if(ioctl(sock_rcv0, SIOCGIFINDEX, &ifr0) < 0)
	{
		die("get index error");
	}

	dest0.sll_family = AF_PACKET;
	dest0.sll_ifindex = ifr0.ifr_ifindex;
	dest0.sll_protocol = htons(RAW_PROTOCOL);
	if(bind(sock_rcv0,(struct sockaddr*)&dest0,sizeof(dest0)) == -1)
	{
		die("bind");
	}
	
	memset(&mr0, 0, sizeof(mr0));
	mr0.mr_ifindex = ifr0.ifr_ifindex;
	mr0.mr_type    = PACKET_MR_PROMISC;
	if (setsockopt(sock_rcv0, SOL_PACKET, PACKET_ADD_MEMBERSHIP,&mr0, sizeof(mr0)) < 0) 
	{
		die("setsockopt REUSEADDR");  
	}
    
	while(1)
	{
		int len;
		int numfds;
		fd_set readfds;
		struct timeval    tv;
		int rc;
		tv.tv_usec = 0;
		tv.tv_sec = 20;
		
		memset(buf, 0, sizeof(buf));
		numfds = sock_rcv0+1;
		
		FD_ZERO(&readfds);
		FD_SET(sock_rcv0, &readfds);
		
		rcv_msg = 0;
		for(i = 1; i <= seq_id; i++)
		{
			rcv_msg += rcv_flag[i];
		}
		lost_msg = (slot_num -1)*100 - rcv_msg;
		
		rc = select(numfds, &readfds, NULL, NULL, &tv);
		if(rc < 0)
		{
			die("Ctrl interface select error");
		}
		if(!rc)
		{
			fprintf(stdout,"rcv_msg = %d\n", rcv_msg);
			fprintf(stdout,"lost_msg = %d\n",lost_msg);
			fprintf (stdout, "master board, ctrl port test failed.\r\n");
			fwrite(&lost_msg, sizeof(lost_msg), 1, fp); 
			fclose(fp);
			die("Ctrl interface not recv correct number of msg");
		}
		if(rc > 0)
		{
			if( FD_ISSET(sock_rcv0, &readfds))
			{
				//fprintf(stdout,"buf[0]= %d\n",buf[0]);
				len = recvfrom(sock_rcv0, (char *)buf, sizeof(buf), 0 ,NULL, NULL);
				count++;
				if((mode == 2)&&(app_act_master_running()))
					seq_id = buf[14];
				else
				    seq_id = buf[0];/*seq_id is in buf[0]*/
				rcv_flag[seq_id]++;
				//fprintf(stdout,"ctrl0: seqid = %d, rcv_flag[%d] = %d\n",seq_id,seq_id,rcv_flag[seq_id] );
				if(count == 1)
				{
					//fprintf(stdout,"first seq_id = %d\n", seq_id);
					if((mode == 2)&&(app_act_master_running()))
						rcv_flag[0] = buf[14];
					else
						rcv_flag[0] = buf[0];
				}
				/*when master board receive a packet send a broadcast packect back*/
				if((mode == 2)&&(!app_act_master_running()))
				{
					test_buffer2[14]= seq_id;
					ret = sendto(sock_rcv0, (char *)test_buffer2, 64, 0,(struct sockaddr *)&dest0, sizeof(dest0));
					//fprintf(stdout,"server ctrl0:send seq_id = %d\n", test_buffer2[14]);
					if(ret < 0)
					{
						die("Internal ctrl port send error");
					}
				}
			}
		}
		
		if(rcv_flag[100] == (slot_num - 1))
		{ 
			break;
		}
	}	
	
	rcv_msg = 0;
	for(i = 1; i <= seq_id; i++)
	{
		rcv_msg += rcv_flag[i];
	}
	fprintf(stdout,"rcv_msg = %d\n", rcv_msg);
	lost_msg = (slot_num -1)*100 - rcv_msg;
	fprintf(stdout,"lost_msg = %d\n",lost_msg);

	fwrite(&lost_msg, sizeof(lost_msg), 1, fp); 
	fclose(fp);
	return lost_msg;
}


int client(int mode, int pre_slot_num) 
{
	int snd_seq = 0;
	int ret = 0;
	int sock_rcv0 = 0;
	int sock_rcv1 = 0;
	int slot_num;
	
	slot_num = app_slot_num_get();
	struct ifreq ifr0;
	struct sockaddr_ll	dest0;
	struct ifreq ifr1;
	struct sockaddr_ll	dest1;
	
	if((sock_rcv0 = socket(PF_PACKET, SOCK_RAW, htons(RAW_PROTOCOL))) == -1)
		die("socket recv");
	bzero(&dest0,sizeof(dest0));
	
	strcpy( ifr0.ifr_name, "ctrl0" );  
	
	if ( ioctl(sock_rcv0, SIOCGIFFLAGS, &ifr0) < 0 )  
	{  
		die("get flag error");
	}  
	if(ioctl(sock_rcv0, SIOCGIFINDEX, &ifr0) < 0 )
	{
		die("get index error");
	}
	dest0.sll_family = AF_PACKET;
	dest0.sll_ifindex = ifr0.ifr_ifindex;
	dest0.sll_protocol = htons(RAW_PROTOCOL);
	if(bind(sock_rcv0,(struct sockaddr*)&dest0,sizeof(dest0)) == -1)
	{
		die("bind");
	}

    if((mode == 1)||(mode == 2))
    {
        if((sock_rcv1 = socket(PF_PACKET, SOCK_RAW, htons(RAW_PROTOCOL))) == -1)
            die("socket recv");

        bzero(&dest1,sizeof(dest1));
        strcpy( ifr1.ifr_name, "ctrl1" );  

        if ( ioctl(sock_rcv1, SIOCGIFFLAGS, &ifr1) < 0 )  
        {  
            die("get flag error");
        }  
        if(ioctl(sock_rcv1, SIOCGIFINDEX, &ifr1) < 0 )
        {
            die("get index error");
        }
        dest1.sll_family = AF_PACKET;
        dest1.sll_ifindex = ifr1.ifr_ifindex;
	    dest1.sll_protocol = htons(RAW_PROTOCOL);
        if(bind(sock_rcv1,(struct sockaddr*)&dest1,sizeof(dest1)) == -1)
        {
            die("bind");
        }
    }
		
    for(snd_seq = 1; snd_seq <= 100; snd_seq++)
    {
        test_buffer1[14]= snd_seq;
        ret = sendto(sock_rcv0, (char *)test_buffer1, 64, 0,(struct sockaddr *)&dest0, sizeof(dest0));
        //fprintf(stdout,"client ctrl0:send seq_id = %d\n", test_buffer1[14]);
        if(ret < 0)
        {
            die("Internal ctrl port send error");
        }
        usleep(10000);
    }
	if((mode == 1)||(mode == 2))
	{
	    for(snd_seq = 1; snd_seq <= 100; snd_seq++)
	    {
		    test_buffer1[14]= snd_seq;
		    ret = sendto(sock_rcv1, (char *)test_buffer1, 64, 0,(struct sockaddr *)&dest1, sizeof(dest1));
			//fprintf(stdout,"client ctrl 1:send seq_id = %d\n", test_buffer1[14]);
		    if(ret < 0)
	        {
			    die("Internal ctrl port send error");
	        }
			usleep(20000);
	    }
	}
  	
    return 0;
}


int server(int mode, int pre_slot_num) 
{
	int ret = 0;
	int sock_rcv0 = 0;
	int sock_rcv1 = 0;
	int count = 0;
	char buf[2*32767] = {0}; 
	int seq_id = 0;
	int rcv_flag[102] = {0};
	int lost_msg = 0;
	int rcv_msg = 0;
	int slot_num = 0;
	int i;
	
	slot_num = app_slot_num_get();
	
	FILE *fp;
	char path[32];
	sprintf(path, "/var/run/ctrl_test.bin");
	fp = fopen(path, "w+");
	if(fp == NULL)
	{
	   die("Unable to open file ctrl_test.bin\n");
	}
	
	struct ifreq ifr0;
	struct sockaddr_ll	dest0;
	struct ifreq ifr1;
	struct sockaddr_ll	dest1;
	struct packet_mreq	mr0;
	struct packet_mreq	mr1;
	
    if((sock_rcv0 = socket(PF_PACKET, SOCK_DGRAM, htons(RAW_PROTOCOL))) == -1)
        die("socket recv");
	
    bzero(&dest0,sizeof(dest0));

    strcpy( ifr0.ifr_name, "ctrl0" );  
	

    if ( ioctl(sock_rcv0, SIOCGIFFLAGS, &ifr0) < 0 )  
    {  
        die("get flag error");
    }  
    if(ioctl(sock_rcv0, SIOCGIFINDEX, &ifr0) < 0)
    {
        die("get index error");
    }

    dest0.sll_family = AF_PACKET;
    dest0.sll_ifindex = ifr0.ifr_ifindex;
    dest0.sll_protocol = htons(RAW_PROTOCOL);
    if(bind(sock_rcv0,(struct sockaddr*)&dest0,sizeof(dest0)) == -1)
    {
        die("bind");
    }
    memset(&mr0, 0, sizeof(mr0));
	mr0.mr_ifindex = ifr0.ifr_ifindex;
	mr0.mr_type    = PACKET_MR_PROMISC;
	if (setsockopt(sock_rcv0, SOL_PACKET, PACKET_ADD_MEMBERSHIP,&mr0, sizeof(mr0)) < 0) 
	{
		die("setsockopt REUSEADDR");  
	}
    
	if((mode == 1)||(mode == 2) )
	{
		
		if((sock_rcv1 = socket(PF_PACKET, SOCK_DGRAM, htons(RAW_PROTOCOL))) == -1)
		    die("socket recv");

		bzero(&dest1,sizeof(dest1));
		strcpy( ifr1.ifr_name, "ctrl1" );  
		
		
		if ( ioctl(sock_rcv1, SIOCGIFFLAGS, &ifr1) < 0 )  
		{  
		    die("get flag error");
		}  
		if(ioctl(sock_rcv1, SIOCGIFINDEX, &ifr1) < 0)
		{
		    die("get index error");
		}

		dest1.sll_family = AF_PACKET;
		dest1.sll_ifindex = ifr1.ifr_ifindex;
		dest1.sll_protocol = htons(RAW_PROTOCOL);
		if(bind(sock_rcv1,(struct sockaddr*)&dest1,sizeof(dest1)) == -1)
		{
		    die("bind");
		}

		memset(&mr1, 0, sizeof(mr1));
		mr1.mr_ifindex = ifr1.ifr_ifindex;
		mr1.mr_type    = PACKET_MR_PROMISC;
		if (setsockopt(sock_rcv1, SOL_PACKET, PACKET_ADD_MEMBERSHIP,&mr1, sizeof(mr1)) == -1) 
		{
		    die("setsockopt REUSEADDR");  
		}

	}		

    while(1)
    {
        int len;
        int numfds;
        fd_set readfds;
        struct timeval    tv;
        int rc;
        tv.tv_usec = 0;
        tv.tv_sec = 20;
		
        memset(buf, 0, sizeof(buf));
        if(mode == 3)
            numfds = sock_rcv0+1;
        else
            numfds = sock_rcv1+1;
		
        FD_ZERO(&readfds);
        FD_SET(sock_rcv0, &readfds);
        if((mode == 1)||(mode == 2) )
            FD_SET(sock_rcv1, &readfds);

		
        rcv_msg = 0;
        for(i = 1; i <= seq_id; i++)
        {
            rcv_msg += rcv_flag[i];
        }
         lost_msg = (slot_num -1)*100 - rcv_msg;
		
        rc = select(numfds, &readfds, NULL, NULL, &tv);
        if(rc < 0)
        {
            die("Ctrl interface select error");
        }
        if(!rc)
        {
            fprintf(stdout,"rcv_msg = %d\n", rcv_msg);
            fprintf(stdout,"lost_msg = %d\n",lost_msg);
            fprintf (stdout, "master board, ctrl port test failed.\r\n");
            fwrite(&lost_msg, sizeof(lost_msg), 1, fp); 
            fclose(fp);
            die("Ctrl interface not recv correct number of msg");
      	}
		if(rc > 0)
		{
		    if( FD_ISSET(sock_rcv0, &readfds))
		    {
		        //fprintf(stdout,"buf[0]= %d\n",buf[0]);
				len = recvfrom(sock_rcv0, (char *)buf, sizeof(buf), 0 ,NULL, NULL);
				count++;
				if((mode == 2)&&(app_act_master_running()))
					seq_id = buf[14];
				else
				    seq_id = buf[0];/*seq_id is in buf[0]*/
				rcv_flag[seq_id]++;
				//fprintf(stdout,"ctrl0: seqid = %d, rcv_flag[%d] = %d\n",seq_id,seq_id,rcv_flag[seq_id] );
				if(count == 1)
				{
					//fprintf(stdout,"first seq_id = %d\n", seq_id);
					if((mode == 2)&&(app_act_master_running()))
						rcv_flag[0] = buf[14];
					else
						rcv_flag[0] = buf[0];
				}
				/*when master board receive a packet send a broadcast packect back*/
				if((mode == 2)&&(!app_act_master_running()))
				{
				    test_buffer2[14]= seq_id;
				    ret = sendto(sock_rcv0, (char *)test_buffer2, 64, 0,(struct sockaddr *)&dest0, sizeof(dest0));
					//fprintf(stdout,"server ctrl0:send seq_id = %d\n", test_buffer2[14]);
				    if(ret < 0)
			        {
					    die("Internal ctrl port send error");
			        }
				
				}
				
	        }
			if((mode == 1)||(mode == 2) )
			{
				if( FD_ISSET(sock_rcv1, &readfds))
				{
				    len = recvfrom(sock_rcv1, (char *)buf, sizeof(buf), 0 ,NULL, NULL);
					if((mode == 2)&&(app_act_master_running()))
						seq_id = buf[14];
					else
						seq_id = buf[0];/*seq_id is in buf[0]*/
					rcv_flag[seq_id]++;
					//fprintf(stdout,"ctrl1: seqid = %d, rcv_flag[%d] = %d\n",seq_id,seq_id,rcv_flag[seq_id] );
					if((mode == 2)&&(!app_act_master_running()))
					{
					    if(rcv_flag[seq_id] > 2)
					    {
					        if(rcv_flag[100] == (slot_num - 1))
								break;
						    continue;
						}
					    test_buffer2[14]= seq_id;
					    ret = sendto(sock_rcv1, (char *)test_buffer2, 64, 0,(struct sockaddr *)&dest1, sizeof(dest1));
						//fprintf(stdout,"server ctrl1:send seq_id = %d\n", test_buffer2[14]);
					    if(ret < 0)
				        {
						    die("Internal ctrl port send error");
				        }
						
					}

				}
			}
						
		}
		
		if(rcv_flag[100] == (slot_num - 1))
	    { 
			break;
	    }
    }
	
	rcv_msg = 0;
	for(i = 1; i <= seq_id; i++)
	{
	    rcv_msg += rcv_flag[i];
	}
    fprintf(stdout,"rcv_msg = %d\n", rcv_msg);
	lost_msg = (slot_num -1)*100 - rcv_msg;
	fprintf(stdout,"lost_msg = %d\n",lost_msg);


    fwrite(&lost_msg, sizeof(lost_msg), 1, fp); 
	fclose(fp);
	return lost_msg;
}

int main(int argc, char *argv[])
{
	int slot_num;
	int master;
	int mode = 0;
	int pre_slot_num = 0;
	int slot_count = 0;
	int ret = 0;
	int is_server = 0;
	int ctrl_num;
	int btype = 0;

	btype = app_board_type_get();
	slot_num = app_slot_num_get();
	master = app_slot_work_mode_get();
	slot_count = app_local_slot_get();
	pre_slot_num = app_ctrl_pre_slot_num_get();
	ctrl_num = app_ctrl_num_get();
	//fprintf (stdout, "test echo, ctrl_num %d \n", ctrl_num);
	//fprintf (stdout, "test echo, pre_slot_num %d.\n", pre_slot_num);
	
	if (argc != 3)
	{
		die("Please input correct parameters");
	}
	mode = atoi(argv[1]);
	is_server = atoi(argv[2]);

	if ((mode == 1) || (mode == 3))
	{
		if (slot_num != 3)
			die("The number of board is not correct.\n");
		if (!master)
			die("The board must be master board");
	}
	if (mode == 2)
	{
		if (slot_num < 3)
			die("The number of board is not correct.\n");
	}
	
	//fprintf (stdout, "is server %d, mode %d, pre_slot_num %d.\n", is_server, mode, pre_slot_num);
	if(is_server)
	{
		if((btype == PPAL_BOARD_TYPE_CGM9048) 
			|| (btype == PPAL_BOARD_TYPE_CGM9048S)
			|| (btype == PPAL_BOARD_TYPE_G9604X)){
			ret = safeguard_server(mode, pre_slot_num);
		} else {
			ret = server(mode, pre_slot_num);
		}
		if (ret == 0)
			fprintf (stdout, "master board, ctrl port test successfully.\n");
		else
			fprintf (stdout, "master board, ctrl port test failed. \n");
	} 
	else
	{
		if((btype == PPAL_BOARD_TYPE_CGM9048) 
			|| (btype == PPAL_BOARD_TYPE_CGM9048S)
			|| (btype == PPAL_BOARD_TYPE_G9604X)){
			ret = safeguard_client(mode, pre_slot_num);
		} else {
			ret = client(mode, pre_slot_num);
		}
		if (ret == 0)
			fprintf (stdout, "slave board, ctrl port test successfully.\n");
		else
			fprintf (stdout, "slave board, ctrl port test failed.\n");
	}
	return 0;
}
