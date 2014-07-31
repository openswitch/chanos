
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <shadow.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include "man_sysconf.h"
#include "man_db.h"
#include "npd/nbm/npd_cplddef.h"
#include "auteware_tail.h"
#include "man_product.h"
#include "ntp_conf.h"

char * slot_status[] =
{
    "NOEXIST",
    "HWINSERTED",
    "SWINSERTED",
    "REGISTERING",
    "SWUPGRADING",
    "REGISTERED",
    "READY",
    "RUNNING",
    "SWITCHOVERING",
    "REMOVING",
    "ERROR",
};


char * online_remove_str[2] =
{
    "NORMAL", 	//FALSE
    "PRESET"	//TRUE
};


int man_get_img_version(char* filename,char *version)
{
    int read_len;
    image_header_t *ptr = NULL;
    char *buf = NULL;
    int buff_len = sizeof(image_header_t);
    FILE* fd=fopen(filename,"r");

    if (!fd)
    {
        return -1;
    }

    buf=malloc(buff_len);

    if (!buf)
    {
        fclose(fd);
        return -1;
    }

    read_len = fread(buf,1,buff_len,fd);

    if (read_len!=buff_len)
    {
        fclose(fd);
        free(buf);
        return -1;
    }

    ptr=(image_header_t *)buf;

    if (ntohl(ptr->ih_magic) != IH_MAGIC)
    {
        fclose(fd);
        free(buf);
        return 1;
    }

    strncpy(version,&(ptr->ih_name[2]),strlen(ptr->ih_name)-2);
    fclose(fd);
    free(buf);
    return 0;
}

/*获取sysinfo中的board name*/
int man_sysinfo_board_name_get(char *board_name)
{
	int ret = 0;
	int i = 0;
	int len = 0;
    ret = property_file_read("/proc/sysinfo/board_name", board_name);
	if(ret == 0)
	{
		len = strlen(board_name);
		for(i = 0; i < len; i++)
		{
			if(board_name[i] == '\r' || board_name[i] == '\n')
			{
				board_name[i] = '\0';
			}
		}
	}
	return ret;
}

board_type_list *init_running_board_type_list()
{
    unsigned int slotno = 0;
    int ret = 0;
    int op_ret=0;
    struct board_mng_s board_attr = {0};
    board_type_list *head=NULL;
    board_type_list *board_t_entry=NULL;
    int local_slotno = app_local_slot_get();
	
    if (config_dbus_connection_init())
    {
        memset(&board_attr,0,sizeof(struct board_mng_s));
		
        board_t_entry = (board_type_list*)malloc(sizeof(board_type_list));

        if (!board_t_entry)
        {
            release_board_type_list(head);
            printf("Malloc board_t_entry failed\r\n");
            return NULL;
        }
        man_sysinfo_board_name_get(board_attr.shortname);
        memset(board_t_entry,0,sizeof(board_type_list));
        board_t_entry->slot_no = local_slotno;
        memcpy(board_t_entry->board_type, board_attr.shortname, strlen(board_attr.shortname));
        if(!head)
        {
            head = board_t_entry;
        }
        else
        {
            board_t_entry->next = head;
            head = board_t_entry;
        }
		return head;
    }

	if(app_box_state_get())
		slotno = -1;

    while (1)
    {
        memset(&board_attr,0,sizeof(struct board_mng_s));
        op_ret = board_info_get_next(&slotno, &board_attr);

        if (op_ret != 0)
        {
			if(slotno <= local_slotno)
			{
                board_t_entry = (board_type_list*)malloc(sizeof(board_type_list));
        
                if (!board_t_entry)
                {
                    release_board_type_list(head);
                    printf("Malloc board_t_entry failed\r\n");
                    return NULL;
                }
                man_sysinfo_board_name_get(board_attr.shortname);
                memset(board_t_entry,0,sizeof(board_type_list));
                board_t_entry->slot_no=local_slotno;
                memcpy(board_t_entry->board_type,board_attr.shortname,strlen(board_attr.shortname));
                if(!head)
                {
                    head=board_t_entry;
                }
                else
                {
                    board_t_entry->next=head;
                    head=board_t_entry;
                }
			}
            break;
        }
        if(local_slotno != slotno)
        {
            if (!strncmp(slot_status[board_attr.run_state],"NOEXIST",strlen("NOEXIST")))
            {
                continue;
            }
    
            if (!strncmp(slot_status[board_attr.run_state],"REMOVING",strlen("REMOVING")))
            {
                continue;
            }
    
            if (!strncmp(online_remove_str[board_attr.online_removed],"PRESET",strlen("PRESET")))
            {
                continue;
            }
        }

        board_t_entry=(board_type_list*)malloc(sizeof(board_type_list));

        if (!board_t_entry)
        {
            release_board_type_list(head);
            printf("Malloc board_t_entry failed\r\n");
            return NULL;
        }

        memset(board_t_entry,0,sizeof(board_type_list));
        board_t_entry->slot_no=slotno;
        memcpy(board_t_entry->board_type,board_attr.shortname,strlen(board_attr.shortname));
        if(!head)
        {
            head=board_t_entry;
        }
        else
        {
            board_t_entry->next=head;
            head=board_t_entry;
        }
    }

    return head;
}


void show_running_board_type(board_type_list *bth)
{
    board_type_list *h=bth;

    while (h)
    {
        //printf("The slot=%d,its board type is %s\n",h->slot_no,h->board_type);
        h=h->next;
    }

    return;
}


int get_local_board_type(board_type_list *head,char *board_type)
{
    int local_slotno = app_local_slot_get();
    board_type_list *h=head;

    while (h)
    {
        if (h->slot_no==local_slotno)
        {
            memcpy(board_type,h->board_type,strlen(h->board_type));
            return 0;
        }

        h=h->next;
    }

    return -1;
}


int get_slot_number_by_board_type(board_type_list *head,char *board_type,int *slotno, int number)
{
    board_type_list *h=head;
    int num = 0;

    while (h)
    {
        if (!strncmp(h->board_type,board_type,strlen(board_type)))
        {
            num++;

            if (num == number)
            {
                *slotno=h->slot_no;
                return 0;
            }
        }

        h=h->next;
    }

    return -1;
}


hw_compat_list *add_board_type_by_image_type(hw_compat_list *head,board_type_list *bth,char *img_type,char *board_type)
{
    //bth can get slot number by board type
    hw_compat_list *tmp=head;
    hw_compat_list *compat_entry=NULL;
    int slotno=0;
    int ret=-1;
    int slot_num = 2;
    board_type_list *new_board_type = NULL;

    while (tmp)
    {
        if (!strncmp(tmp->image_type,img_type,strlen(img_type)))
        {
            //find the image type
            new_board_type=(board_type_list*)malloc(sizeof(board_type_list));

            if (!new_board_type)
            {
                printf("Malloc for new_board_type failed \r\n");
                return NULL;
            }

            memset(new_board_type,0,sizeof(board_type_list));
            memcpy(new_board_type->board_type,board_type,strlen(board_type));
            ret=get_slot_number_by_board_type(bth,board_type,&slotno, 1);

            if (!ret)
            {
                new_board_type->slot_no=slotno;
            }

            if (tmp->board_type_head)
            {
                new_board_type->next=tmp->board_type_head;
                tmp->board_type_head=new_board_type;
            }
            else
            {
                tmp->board_type_head=new_board_type;
            }

            // check if some slots have the same board type
            ret = get_slot_number_by_board_type(bth,board_type,&slotno, slot_num);

            while (!ret)
            {
                new_board_type=(board_type_list*)malloc(sizeof(board_type_list));

                if (!new_board_type)
                {
                    printf("Malloc for new_board_type failed \r\n");
                    return NULL;
                }

                memset(new_board_type,0,sizeof(board_type_list));
                memcpy(new_board_type->board_type,board_type,strlen(board_type));
                new_board_type->slot_no=slotno;
                new_board_type->next=tmp->board_type_head;
                tmp->board_type_head=new_board_type;
                slot_num++;
                ret=get_slot_number_by_board_type(bth,board_type,&slotno, slot_num);
            }

            return head;
        }

        tmp=tmp->next;
    }

    //add a new compat_entry as the new head
    compat_entry=(hw_compat_list*)malloc(sizeof(hw_compat_list));

    if (!compat_entry)
    {
        printf("Malloc for compat_entry failed \r\n");
        return NULL;
    }

    memset(compat_entry,0,sizeof(hw_compat_list));
    memcpy(compat_entry->image_type,img_type,strlen(img_type));
    compat_entry->board_type_head=(board_type_list*)malloc(sizeof(board_type_list));

    if (!compat_entry->board_type_head)
    {
        free(compat_entry);
        printf("Malloc for board_type_head failed \r\n");
        return NULL;
    }

    memset(compat_entry->board_type_head,0,sizeof(board_type_list));
    memcpy(compat_entry->board_type_head->board_type,board_type,strlen(board_type));
    ret=get_slot_number_by_board_type(bth,board_type,&slotno, 1);

    if (!ret)
    {
        compat_entry->board_type_head->slot_no=slotno;
    }

    if (head)
    {
        compat_entry->next=head;
    }

    // check if some slots have the same board type
    slot_num = 2;
    ret = get_slot_number_by_board_type(bth,board_type,&slotno, slot_num);

    while (!ret)
    {
        new_board_type=(board_type_list*)malloc(sizeof(board_type_list));

        if (!new_board_type)
        {
            printf("Malloc for new_board_type failed \r\n");
            return NULL;
        }

        memset(new_board_type,0,sizeof(board_type_list));
        memcpy(new_board_type->board_type,board_type,strlen(board_type));
        new_board_type->slot_no=slotno;
        new_board_type->next=compat_entry->board_type_head;
        compat_entry->board_type_head=new_board_type;
        slot_num++;
        ret=get_slot_number_by_board_type(bth,board_type,&slotno, slot_num);
    }

    return compat_entry;
}

int release_board_type_list(board_type_list *head)
{
    board_type_list *tmp_head_b=head;
    board_type_list *b_free=NULL;

    while (tmp_head_b)
    {
        b_free=tmp_head_b;
        tmp_head_b=tmp_head_b->next;
        free(b_free);
    }

    return 0;
}

int release_hw_compat_list(hw_compat_list *head)
{
    hw_compat_list *tmp_head_compat=head;
    hw_compat_list *free_compat=NULL;

    while (tmp_head_compat)
    {
        free_compat=tmp_head_compat;
        tmp_head_compat=tmp_head_compat->next;

        if (free_compat->board_type_head)
        {
            release_board_type_list(free_compat->board_type_head);
        }

        free(free_compat);
    }

    return 0;
}

int show_hw_compat_list(hw_compat_list *head)
{
    hw_compat_list *tmp_head_compat=head;
    board_type_list *tmp_b_list=NULL;

    while (tmp_head_compat)
    {
        //printf("The image type is %s\n",tmp_head_compat->image_type);
        if (tmp_head_compat->board_type_head)
        {
            tmp_b_list=tmp_head_compat->board_type_head;

            while (tmp_b_list)
            {
                //printf("The board type is %s,its slot=%d\n",tmp_b_list->board_type,tmp_b_list->slot_no);
                tmp_b_list=tmp_b_list->next;
            }
        }

        tmp_head_compat=tmp_head_compat->next;
    }

    return 0;
}

int get_slot_sequence_by_image_type(hw_compat_list *head,char *img_type,int *slot_seq)
{
    hw_compat_list *tmp_head_compat=head;
    board_type_list *tmp_b_list=NULL;
    int cnt=0;

    while (tmp_head_compat)
    {
        if (!strncmp(tmp_head_compat->image_type,img_type,strlen(img_type)))
        {
            if (tmp_head_compat->board_type_head)
            {
                tmp_b_list=tmp_head_compat->board_type_head;

                while (tmp_b_list)
                {
                    if (tmp_b_list->slot_no!=0)
                    {
                        slot_seq[cnt]=tmp_b_list->slot_no;
                        cnt++;
                    }

                    tmp_b_list=tmp_b_list->next;
                }

                return cnt;
            }
        }

        tmp_head_compat=tmp_head_compat->next;
    }

    return cnt;
}

hw_compat_list *man_parse_hw_compat_list(board_type_list *bth)
{
    FILE *fp=fopen(HW_COMPAT_FILE,"r");
    hw_compat_list *head=NULL;
    hw_compat_list *tmp=NULL;
    char buffer[64];
    char board_type[32];
    int len=0;
    char *delimiter=NULL;

    if (!fp)
    {
        printf("Open file %s failed\r\n",HW_COMPAT_FILE);
        return NULL;
    }

    while (1)
    {
        memset(buffer,0,64);
        fgets(buffer,64,fp);
        len=strlen(buffer);

        if (len<3)
        {
            fclose(fp);
            break;
        }

        delimiter=strchr(buffer,' ');

        if (!delimiter)
        {
            printf("Parse file %s failed\r\n",HW_COMPAT_FILE);
            fclose(fp);
            release_hw_compat_list(head);
            return NULL;
        }

        memset(board_type,0,32);
        memcpy(board_type,buffer,strlen(buffer)-strlen(delimiter));
        tmp=add_board_type_by_image_type(head,bth,delimiter+1,board_type);

        if (!tmp)
        {
            printf("Function add_board_type_by_image_type failed \r\n");
            fclose(fp);
            release_hw_compat_list(head);
            return NULL;
        }

        head=tmp;
    }

    return head;
}

int  get_image_type(char *filename,char *type)
{
    FILE *fp=fopen(filename,"r");
    char tmp[256]={0};
    auteware_img_pack_tail *tail_entry=NULL;
    int ret=-1;

    if (!fp)
    {
        printf("Cannot open file %s\n",filename);
        return -1;
    }

    fseek(fp,-sizeof(auteware_img_pack_tail),SEEK_END);
    ret=fread(tmp, sizeof(auteware_img_pack_tail), 1, fp);
    fclose(fp);

    if (ret<1)
    {
        printf("Cannot read tail\n");
        return -1;
    }

    if (memcmp(tmp,AUTEWARETAIL,16))
    {
        return -1;
    }

    tail_entry=(auteware_img_pack_tail*)tmp;
    memcpy(type,tail_entry->type,strlen(tail_entry->type));
    return 0;
}


void get_slots_str(int *slot_seq,char *slots,int num)
{
    int i=0;
    int cnt=0;
    char tmp[4];

    while (cnt<=num)
    {
        if (i >= 16)
        {
            break;
        }

        if (0 == slot_seq[i])
        {
            i++;
            continue;
        }
        if(cnt != 0)
        {
            strcat(slots,"-");
        }
        memset(tmp,0,4);
        sprintf(tmp,"%d",slot_seq[i]);
        strcat(slots,tmp);
        i++;
        cnt++;
    }

    return;
}


int handle_master_in_slots(board_type_list *bth,int *slot_seq,int *num)
{
    int i=0, replace= 0;
    int standby_slotno=app_sbymaster_slot_get();
    int master_slotno=app_actmaster_slot_get();
    board_type_list *h=bth;
    int running=0;
    int standby_in=0;
    int num_tmp =*num;
    replace = *num;/*standby slot add to the last by default*/
    while (h)
    {
        if (h->slot_no==standby_slotno)
        {
            running=1;
        }

        h=h->next;
    }

    for (; i<num_tmp; i++)
    {
        if (slot_seq[i] == master_slotno)
        {
            slot_seq[i]=0;
            *num-=1;
            replace = i;
        }
        else if (slot_seq[i]==standby_slotno)
        {
            standby_in=1;
        }
    }

    if (running&&!standby_in)
    {
        slot_seq[replace]=standby_slotno;
        *num+=1;
    }

    return 0;
}

int check_image_is_compatiple(char *file_name,hw_compat_list *head,board_type_list *bth)
{
    char img_type[32]={0};
    char board_type[32]={0};
    hw_compat_list *hw_compat_head=head;
    board_type_list *board_type_h=bth;
    int ret=-1;
    ret=get_image_type(file_name,img_type);

    if (ret)
    {
        printf("Cannot get image type:%s\r\n",file_name);
        return -1;
    }

    ret=get_local_board_type(bth, board_type);

    if (ret)
    {
        printf("Cannot get local board type\r\n");
        return -1;
    }

    while (hw_compat_head)
    {
        if (!strncmp(hw_compat_head->image_type,img_type,strlen(img_type)))
        {
            board_type_h=hw_compat_head->board_type_head;

            while (board_type_h)
            {
                if (!strncmp(board_type_h->board_type,board_type,strlen(board_type)))
                {
                    return 0;
                }

                board_type_h=board_type_h->next;
            }
        }

        hw_compat_head=hw_compat_head->next;
    }

    return -1;
}




int send_image_by_board_type(char *file_name,hw_compat_list *head,board_type_list *bth)
{
    char img_type[32]={0};
    int slot_seq[16]={0};
    char slots[64]={0};
    char cmd[256]={0};
    int ret=-1;
    ret=get_image_type(file_name,img_type);

    if (ret)
    {
        printf("Cannot get image type:%s\r\n",file_name);
        return -1;
    }

    memset(slot_seq, 0, sizeof(slot_seq));
    //get the compatible board types
    ret=get_slot_sequence_by_image_type(head, img_type, slot_seq);

    if (ret)
    {
        //add the standby if missed
        handle_master_in_slots(bth,slot_seq,&ret);

        if (ret)
        {
            get_slots_str(slot_seq,slots,ret);
            //printf("Slot sequence: %s num=%d\r\n",slots,ret);
            sprintf(cmd,"file_client -i %s %s",slots,file_name);
            return system(cmd);
        }

        return 0;
    }

    return ret;
}


int man_send_all_images()
{
    char full_name[128];
    int ret=-1;
    struct dirent *ptr;
    struct stat sb;
    DIR *dir;
    dir=opendir("/mnt");
    board_type_list *bth=init_running_board_type_list();
    hw_compat_list *head=man_parse_hw_compat_list(bth);

    while ((ptr=readdir(dir))!=NULL)
    {
        if (stat(ptr->d_name, &sb) >= 0 && S_ISDIR(sb.st_mode))
        {
            continue;
        }

        memset(full_name,0,128);
        sprintf(full_name,"/mnt/%s",ptr->d_name);
        ret=send_image_by_board_type(full_name,head,bth);
    }

    closedir(dir);
    release_board_type_list(bth);
    release_hw_compat_list(head);
    return 0;
}


unsigned long get_file_size(const char *filename)
{
    struct stat buf;

    if (stat(filename, &buf)<0)
    {
        return 0;
    }

    return (unsigned long)buf.st_size;
}

int get_boot_version_name(char* vername)
{
    int fd;
    int retval;
    boot_env_t	env_args={0};
    char *name = "version";
    sprintf(env_args.name,name);
    env_args.operation = GET_BOOT_ENV;
    fd = open("/dev/bm0",0);

    if (fd < 0)
    {
        return 1;
    }

    retval = ioctl(fd,BM_IOC_ENV_EXCH,&env_args);

    if (retval == -1)
    {
        close(fd);
        return 2;
    }
    else
    {
        sprintf(vername,env_args.value);
    }

    close(fd);
    return 0;
}

int get_2nd_boot_img_name(char* imgname)
{
    int fd;
    int retval;
    boot_env_t	env_args={0};
    char *name = "secondary_bootfile";
    sprintf(env_args.name,name);
    env_args.operation = GET_BOOT_ENV;
    fd = open("/dev/bm0",0);

    if (fd < 0)
    {
        sprintf(imgname, "aw.2nd.img");
        return 1;
    }

    retval = ioctl(fd,BM_IOC_ENV_EXCH,&env_args);

    if (retval == -1)
    {
        sprintf(imgname, "aw.2nd.img");
        /*
        	    sprintf(env_args.value,imgname);
        		env_args.operation = SAVE_BOOT_ENV;
        		retval = ioctl(fd, BM_IOC_ENV_EXCH, &env_args);
        		if(retval == -1)
        		{
        		    close(fd);
        		    return 2;
        		}
        */
    }
    else
    {
        sprintf(imgname,env_args.value);
    }

    close(fd);
    return 0;
}

int get_boot_img_name(char* imgname)
{
    int fd;
    int retval;
    boot_env_t	env_args={0};
    char *name = "bootfile";
    sprintf(env_args.name,name);
    env_args.operation = GET_BOOT_ENV;
    fd = open("/dev/bm0",0);

    if (fd < 0)
    {
        sprintf(imgname, "aw.img");
        return 1;
    }

    retval = ioctl(fd,BM_IOC_ENV_EXCH,&env_args);

    if (retval == -1)
    {
        sprintf(imgname, "aw.img");
        /*
        	    sprintf(env_args.value,imgname);
        		env_args.operation = SAVE_BOOT_ENV;
        		retval = ioctl(fd, BM_IOC_ENV_EXCH, &env_args);
        		if(retval == -1)
        		{
        		    close(fd);
        		    return 2;
        		}
        */
    }
    else
    {
        sprintf(imgname,env_args.value);
    }

    close(fd);
    return 0;
}

int set_boot_img_name(char* imgname)
{
    int fd;
    int retval;
    boot_env_t	env_args={0};
    char ori_img_name[128];
    char *name = "bootfile";
    char *secondary_name = "secondary_bootfile";
    memset(ori_img_name, 0, 128);
    retval = get_boot_img_name(ori_img_name);

    if (retval == 0 && (strncmp(imgname, ori_img_name, 128) ==0))
    {
        return 0;
    }

    fd = open("/dev/bm0",0);

    if (fd < 0)
    {
        return 1;
    }

    sprintf(env_args.name,name);
    sprintf(env_args.value,imgname);
    env_args.operation = SAVE_BOOT_ENV;
    retval = ioctl(fd,BM_IOC_ENV_EXCH,&env_args);

    if (retval == -1)
    {
        close(fd);
        return 2;
    }

    if (retval == 0 && (strncmp(imgname, ori_img_name, 128) != 0))
    {
        memset(&env_args, 0, sizeof(boot_env_t));
        sprintf(env_args.name,secondary_name);
        sprintf(env_args.value,ori_img_name);
        env_args.operation = SAVE_BOOT_ENV;
        ioctl(fd,BM_IOC_ENV_EXCH,&env_args);
    }

    close(fd);
    return 0;
}


const unsigned long crc_table[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

/* ========================================================================= */
#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);

/* ========================================================================= */
unsigned long  crc32(unsigned long  crc, const unsigned char *buf, unsigned int len)
{
    crc = crc ^ 0xffffffffL;
    while (len >= 8)
    {
      DO8(buf);
      len -= 8;
    }
    if (len) do {
      DO1(buf);
    } while (--len);
    return crc ^ 0xffffffffL;
}


int check_img(char* filename)
{
    int read_len;
    image_header_t *ptr = NULL;
    char *buf = NULL;
    char *imag_buf = NULL;
    unsigned long checksum = 0;
    unsigned long header_checksum = 0;
    int image_len = 0;
	
    int buff_len = sizeof(image_header_t);
    FILE* fd=fopen(filename,"r");

    if (!fd)
    {
        return -1;
    }

    buf=(char *)malloc(buff_len);

    if (!buf)
    {
        fclose(fd);
        return -1;
    }

    read_len = fread(buf,1,buff_len,fd);

    if (read_len!=buff_len)
    {
        fclose(fd);
        free(buf);
        return -1;
    }

    ptr=(image_header_t *)buf;

    if (ntohl(ptr->ih_magic) != IH_MAGIC)
    {
        fclose(fd);
        free(buf);
        return 1;
    }
    /*crc check uImage header*/
    header_checksum = ntohl (ptr->ih_hcrc);     
    ptr->ih_hcrc = 0;
    checksum = crc32 (0, (unsigned char *) ptr, buff_len) ;
    if (header_checksum != checksum) {
		/*Try octeon header( have ih_ver in the end of image_header_t)*/
		fread(buf + read_len,1,4,fd);
		checksum = crc32 (0, (unsigned char *) ptr, buff_len + 4) ;
		if(checksum != header_checksum)
		{
            fclose(fd);
	        free(buf);
            printf ("Bad Header Checksum : cal 0x%08x -- real 0x%08x\n", checksum, header_checksum);
            return 1;
		}
    }

    /*crc check uImage data*/
    image_len = ntohl (ptr->ih_size);
    read_len = 0;
    imag_buf = (char *)malloc(image_len);
    if (!imag_buf)
    {
         printf ("malloc image buf  failure\n");
        fclose(fd);
        free(buf);
        return -1;
    }

    memset(imag_buf, 0, image_len);
    read_len = fread(imag_buf,1,image_len,fd);

    if (read_len != image_len)
    {
        printf ("read uimage data  failure\n");
        fclose(fd);
        free(buf);
        free(imag_buf);
        return -1;
    }

    checksum = crc32 (0, (unsigned char *) imag_buf, image_len);
    if (checksum != ntohl (ptr->ih_dcrc)) {
        printf ("Bad Data CRC : cal 0x%08x -- real 0x%08x\n", checksum, ntohl (ptr->ih_dcrc));
	    fclose(fd);
        free(buf);
        free(imag_buf);
        return -1;
    }


    fclose(fd);
    free(buf);
    free(imag_buf);
    return 0;
}

int write_bootrom(char *path)
{
    int fd;
    bootrom_file bootrom;
    int retval;
    memset(bootrom.path_name,0,sizeof(bootrom.path_name));
    strcpy(bootrom.path_name,path);
    fd = open("/dev/bm0",0);

    if (fd < 0)
    {
        return -1;
    }

    retval = ioctl(fd,BM_IOC_BOOTROM_EXCH,&bootrom);

    if (retval < 0)
    {
        printf("update bootrom failed return [%d]\n",retval);
        close(fd);
        return retval;
    }

    close(fd);
    return 0;
}

int read_hosts_file_item(char *item,char *service,char *addr)
{
    if (!strncmp(item,"in.telnetd",strlen("in.telnetd")))
    {
        /* telnetd filter */
        memcpy(service,"in.telnetd",strlen("in.telnetd"));
        memcpy(addr,item+strlen("in.telnetd")+1,strlen(item)-strlen("in.telnetd")-1);
    }

    if (!strncmp(item,"sshd",strlen("sshd")))
    {
        /* sshd filter */
        memcpy(service,"sshd",strlen("sshd"));
        memcpy(addr,item+strlen("sshd")+1,strlen(item)-strlen("sshd")-1);
    }

    return 0;
}

int get_pam_status(char *rad_state,char *tac_state)
{
    int ret =0;
    FILE *fp=NULL;
    fp = fopen(PAM_STATE_FILE,"r");

    if (!fp)
    {
        /*printf("%% Open file %s failed\r\n",PAM_STATE_FILE);*/
        return -1;
    }

    fscanf(fp,"%s",rad_state);
    fscanf(fp,"%s",tac_state);
    fclose(fp);
    return ret;
}

int get_offset_time()
{
    FILE *fp;
    char buffer[100];
    unsigned int timesec;
    fp = fopen("/var/run/offset_time","r");

    if (NULL == fp)
    {
        return 0;
    }

    if (fgets(buffer,100,fp))
    {
        timesec = atoi(buffer);
    }
    else
    {
        timesec = 0;
    }

    fclose(fp);
    return timesec;
}

int set_offset_time(int offset_time)
{
    FILE *fp;
    char  buffer[100];
    char* tmpbuf;
    int ret =0;
    fp = fopen("/var/run/offset_time","w");

    if (NULL == fp)
    {
        return -1;
    }

    fprintf(fp,"%d",offset_time);
    fclose(fp);
    return ret;
}

int set_sys_location(char *location)
{
    FILE* fp=NULL;
    char ptr[SYS_LOCATION_STR_LEN];

    if (!location)
        return -1;

    memset(ptr,0,SYS_LOCATION_STR_LEN);
    fp = fopen(SYS_LOCATION_CONFIG_FILE,"w");

    if (!fp)
        return -1;

    sprintf(ptr,"%s%s\n",SYS_LOCATION_PREFIX,location);
    fputs(ptr,fp);
    fclose(fp);
    return 0;
}

char* get_net_element(char *net_element)
{
    FILE* fp=NULL;
    char ptr[NET_ELEMENT_STR_LEN];
    memset(ptr,0,NET_ELEMENT_STR_LEN);
    fp = fopen(NET_ELEMENT_CONFIG_FILE,"r");

    if (!fp)
    {
        return NULL;
    }

    while (fgets(ptr,NET_ELEMENT_STR_LEN,fp))
    {
        if (!strncmp(ptr,NET_ELEMENT_PREFIX,strlen(NET_ELEMENT_PREFIX)))
        {
            sprintf(net_element,ptr+strlen(NET_ELEMENT_PREFIX));
            fclose(fp);
            return net_element;
        }
    }

    fclose(fp);
    return NULL;
}

int set_net_element(char *net_element)
{
    FILE* fp=NULL;
    char ptr[NET_ELEMENT_STR_LEN];

    if (!net_element)
        return -1;

    memset(ptr,0,NET_ELEMENT_STR_LEN);
    fp = fopen(NET_ELEMENT_CONFIG_FILE,"w");

    if (!fp)
        return -1;

    sprintf(ptr,"%s%s\n",NET_ELEMENT_PREFIX,net_element);
    fputs(ptr,fp);
    fclose(fp);
    return 0;
}

int get_sys_desc(char *desc)
{
    FILE * fp=NULL;
    fp = fopen("/etc/sys_desc","r");

    if (!fp)
    {
        //printf("%% Open file:/etc/sys_desc failed\r\n");
        return -1;
    }

    fgets(desc,256,fp);
    fclose(fp);
    return 0;
}

char* get_sys_location(char *location)
{
    FILE* fp=NULL;
    char ptr[SYS_LOCATION_STR_LEN];
    memset(ptr,0,SYS_LOCATION_STR_LEN);
    fp = fopen(SYS_LOCATION_CONFIG_FILE,"r");

    if (!fp)
    {
        return NULL;
    }

    while (fgets(ptr,SYS_LOCATION_STR_LEN,fp))
    {
        if (!strncmp(ptr,SYS_LOCATION_PREFIX,strlen(SYS_LOCATION_PREFIX)))
        {
            sprintf(location,ptr+strlen(SYS_LOCATION_PREFIX));
            fclose(fp);
            return location;
        }
    }

    fclose(fp);
    return NULL;
}


int get_cli_syslog_str(void)
{
    FILE *fp=NULL;
    char *ptr=NULL;
    int ret=0;
    fp = fopen(VTYSH_CLI_LOG_CONFIG_FILE,"r");

    if (!fp)
        return 0;

    ptr=malloc(8);
    memset(ptr,0,8);

    if (!ptr)
    {
        fclose(fp);
        return 0;
    }

    fgets(ptr,8,fp);
    ret = atoi(ptr);
    free(ptr);
    fclose(fp);
    return ret;
}
int set_cli_syslog_str(int num)
{
    FILE * fp=NULL;
    char *ptr=NULL;
    int i=0;
    fp = fopen(VTYSH_CLI_LOG_CONFIG_FILE,"w");

    if (!fp)
        return -1;

    ptr=malloc(8);

    if (!ptr)
    {
        fclose(fp);
        return -1;
    }

    sprintf(ptr,"%d\n",num);
    fputs(ptr,fp);
    free(ptr);
    fclose(fp);
    return 0;
}

int get_dns_str(char **name)
{
    FILE *fp=NULL;
    char *ptr=NULL;
    int i=0;
    fp = fopen("/etc/resolv.conf","r");

    if (!fp)
        return -1;

    ptr=malloc(128);

    if (!ptr)
    {
        fclose(fp);
        return -1;
    }

    while (fgets(ptr,128,fp))
    {
        if (!strncmp(ptr,"nameserver ",11))
        {
            sprintf(*(name+i),ptr+11);
            i++;
        }
    }

    free(ptr);
    fclose(fp);
    return i;
}
int set_dns_str(const char **name,int num)
{
    FILE * fp=NULL;
    char *ptr=NULL;
    int i=0;
    fp = fopen("/etc/resolv.conf","w");

    if (!fp)
        return -1;

    ptr=malloc(128);

    if (!ptr)
    {
        fclose(fp);
        return -1;
    }

    for (i=0; i<num; i++)
    {
        sprintf(ptr,"nameserver %s\n",*(name+i));
        fputs(ptr,fp);
    }

    free(ptr);
    fclose(fp);
    return 0;
}

int delete_dns_server(char *addr)
{
    char *dnsstr[MAX_DNS_SERVER];
    int ret,i;

    if (!addr)
    {
        return 0;
    }

    for (i=0; i<MAX_DNS_SERVER; i++)
    {
        dnsstr[i] = malloc(128);

        if (!dnsstr[i])
        {
            goto ret_err;
        }

        memset(dnsstr[i],0,128);
    }

    ret = get_dns_str(&dnsstr);

    if (ret<0 || ret > MAX_DNS_SERVER)
    {
        printf("Can't get system dns seting\n");
        goto ret_err;
    }
    else
    {
        int i=0;

        for (i=0; i<ret; i++)
        {
            if (!strncmp(addr,dnsstr[i],strlen(addr)))
            {
                int j ;

                if (i<ret-1)
                {
                    for (j=i; j<ret-1; j++)
                    {
                        sprintf(dnsstr[j],dnsstr[j+1]);
                    }
                }

                break;
            }
        }

        if (i >= ret)
        {
            printf("Can't get the dns %s\n",addr);
            goto ret_err;
        }

        if (set_dns_str(&dnsstr,ret-1))
        {
            printf("Delete system dns error\n");
            goto ret_err;
        }
    }

    printf("delete dns success \n");
    return ret;
ret_err:

    for (i=0; i<MAX_DNS_SERVER; i++)
    {
        if (dnsstr[i])
            free(dnsstr[i]);
    }

    return -1;
}

int set_ripd_socket_buffer(unsigned int entry)
{
    FILE* confp=NULL;
    char* buf = NULL;
    char tmp[128];
    int ret=-1,buflen=0;
    memset(tmp,0,128);
    confp = fopen(RIPD_SOCKET_BUF_FILE,"w");

    if (!confp)
    {
        fprintf(stdout,"Can't open %s file\n",RIPD_SOCKET_BUF_FILE);
        return ret;
    }

    buflen = entry*42;/*42=41600/(40*25)*/
    sprintf(tmp,"%s%d\n",RIPD_MAX_SOCKET_BUF,buflen);
    ret = fputs(tmp,confp);

    if (EOF == ret)
    {
        fclose(confp);
        fprintf(stdout,"Write file %s error\n",RIPD_SOCKET_BUF_FILE);
        return -1;
    }

    fclose(confp);
    return ret;
}

int dcli_image_is_bak(char *name)
{
    if (strncasecmp(name + strlen(name) - 4, ".bak", 4))
    {
        return 0;
    }

    return 1;
}

/* return value:1 is for boot image,0 is for normal image,-1 is for error */
int get_boot_image_build_time(unsigned long *create_time)
{
    int read_len;
    image_header_t *ptr = NULL;
    int buff_len = sizeof(image_header_t);
    FILE* fd = NULL;
    char full_file_name[128];
    int num=0;
    char imgname[64];
    char img_hdr[80] = {0};
    int ret=-1;
    memset(imgname,0,64);
    get_boot_img_name(imgname);
    memset(full_file_name, 0, 128);
    sprintf(full_file_name, "/mnt/%s", imgname);
    fd = fopen(full_file_name, "r");

    if (fd == NULL)
    {
        return -1;
    }

    read_len = fread(img_hdr, 1, buff_len, fd);

    if (read_len != buff_len)
    {
        fclose(fd);
        return -1;
    }

    ptr = (image_header_t *)img_hdr;

    if (ntohl(ptr->ih_magic) != IH_MAGIC)
    {
        fclose(fd);
        return -1;
    }
    else
    {
        *create_time = ntohl(ptr->ih_time);
        fclose(fd);
    }

    return 0;
}

/* return value:1 is for boot image,0 is for normal image,-1 is for error */
int get_next_image(int *number,char *img_name,unsigned long *file_size, unsigned long *create_time)
{
    int read_len;
    image_header_t *ptr = NULL;
    int buff_len = sizeof(image_header_t);
    FILE* fd = NULL;
    char full_file_name[128];
    char file_name[64];
    int num=0;
    char imgname[64] ;
    int ret=-1;
    FILE *fp=NULL;
    memset(imgname,0,64);
    get_boot_img_name(imgname);
    fp = fopen(IMG_NAME_FILE, "r");

    if (!fp)
    {
        printf("open image file:%s failed \n",IMG_NAME_FILE);
        return -1;
    }

    memset(file_name, 0, 64);
    fscanf(fp,"%s",file_name);
    ret = -1;

    while (strlen(file_name))
    {
        num++;

        if (num >= *number)
        {
            char img_hdr[80] = {0};
            ret =0;

            if (dcli_image_is_bak(file_name))
            {
                memset(file_name,0,64);
                fscanf(fp,"%s",file_name);
                ret = -1;
                continue;
            }

            memset(full_file_name, 0, 128);
            sprintf(full_file_name, "/mnt/%s", file_name);
            *file_size = get_file_size(full_file_name);

            if (!strcmp(file_name, imgname))
            {
                ret = 1;
            }

            fd = fopen(full_file_name, "r");

            if (fd == NULL)
            {
                memset(file_name,0,64);
                fscanf(fp,"%s",file_name);
                ret = -1;
                continue;
            }

            read_len = fread(img_hdr, 1, buff_len, fd);

            if (read_len != buff_len)
            {
                fclose(fd);
                memset(file_name,0,64);
                fscanf(fp, "%s",file_name);
                ret = -1;
                continue;
            }

            ptr = (image_header_t *)img_hdr;

            if (ntohl(ptr->ih_magic) != IH_MAGIC)
            {
                fclose(fd);
                memset(file_name,0,64);
                fscanf(fp,"%s",file_name);
                ret = -1;
                continue;
            }
            else
            {
                memcpy(img_name, ptr->ih_name, IH_NMLEN);
                *create_time = ntohl(ptr->ih_time);
                fclose(fd);
                *number = num + 1;
                break;
            }
        }

        memset(file_name,0,64);
        fscanf(fp,"%s",file_name);
    }

    fclose(fp);
    return ret;
}


int get_image_name_by_number(int number,char *img_name)
{
	int read_len;
	image_header_t *ptr = NULL;
	int buff_len = sizeof(image_header_t);
	FILE* fd = NULL;
    char full_file_name[128];
    char file_name[64];
    int num=0;
    char imgname[64] ;
    int ret=-1;
    FILE *fp=NULL;	
    fp = fopen(IMG_NAME_FILE, "r");
    if(!fp)
    {
        printf("open image file:%s failed \n",IMG_NAME_FILE);
        return -1;
    }
    memset(file_name, 0, 64);
    fscanf(fp,"%s",file_name);
    while(strlen(file_name))
    {
       
		char img_hdr[80] = {0};
        if(dcli_image_is_bak(file_name))
        {
            memset(file_name,0,64);
            fscanf(fp,"%s",file_name);
            ret = -1;
            continue;
        }
        memset(full_file_name, 0, 128);
		sprintf(full_file_name, "/mnt/%s", file_name);
		fd = fopen(full_file_name, "r");
		if(fd == NULL)
		{
            memset(file_name,0,64);
            fscanf(fp,"%s",file_name);
			ret = -1;
			continue;
		}
        read_len = fread(img_hdr, 1, buff_len, fd);
        if(read_len != buff_len)
        {
	        fclose(fd);
            memset(file_name,0,64);
            fscanf(fp, "%s",file_name);
			ret = -1;
			continue;
        }
        ptr = (image_header_t *)img_hdr;
        fclose(fd);
        if(ntohl(ptr->ih_magic) != IH_MAGIC)
        {	        
            memset(file_name,0,64);
            fscanf(fp,"%s",file_name);
			ret = -1;
			continue;
        }
		else
		{				
			num++;
            if(num == number)
            {
                memcpy(img_name, file_name, strlen(file_name));
                ret = 0;
                break;
            }
		}     
        memset(file_name,0,64);
        fscanf(fp,"%s",file_name);
    }
    fclose(fp); 
    return ret;
    
    
}

int man_tail_check_auteware_image(char *file_name,int *block)
{
    int i=0;
    int ret = -1;
    FILE *fp=NULL;
    char magic_str[32];
    int location=0;
		
    fp=fopen(file_name,"rb");
    if(!fp)
    {
        return -1;
    }
    ret = fseek(fp, BLOCK_SIZE*i, SEEK_SET); 
    while(1)
    {
        memset(magic_str,0,32);
        ret = fread(magic_str,16,1,fp);
        if(ret != 1)
        {
            ret = IMG_NO_OPERATION;
            break;
        }        
        if(!memcmp(magic_str, AUTEWARETAIL, 16))
        {
            *block = i;
            ret = 0;
            //break;
            fclose(fp);
			return ret;
        }
        i++;
        ret = fseek(fp, BLOCK_SIZE*i, SEEK_SET); 
    }
    //check if the image needs operation or not
    if(ret==0)
    {
        location=BLOCK_SIZE*i+sizeof(auteware_img_pack_tail);
        fseek(fp, location, SEEK_SET); 
        ret = fread(magic_str,16,1,fp);
        if(ret!=1)
        {
            ret=IMG_NO_OPERATION;
        }
        else
        {
            ret=0;
        }
    }
    fclose(fp);
    return ret;
}

int get_next_login_permit(int number,char *filter)
{
    FILE * fp= NULL;
    char tmp[64];
    int num=0;
    int ret=-1;
    fp= fopen("/etc/hosts.allow","r");

    if (fp)
    {
        memset(tmp,0,64);
        fscanf(fp,"%s",tmp);

        while (strlen(tmp))
        {
            num++;

            if (number==num)
            {
                memcpy(filter,tmp,strlen(tmp));
                ret=0;
                break;
            }

            memset(tmp,0,64);
            fscanf(fp,"%s",tmp);
        }

        fclose(fp);
    }

    return ret;
}

int get_next_login_deny(int number,char *filter)
{
    FILE * fp= NULL;
    char tmp[64];
    int num=0;
    int ret=-1;
    fp= fopen("/etc/hosts.deny","r");

    if (fp)
    {
        memset(tmp,0,64);
        fscanf(fp,"%s",tmp);

        while (strlen(tmp))
        {
            num++;

            if (number==num)
            {
                memcpy(filter,tmp,strlen(tmp));
                ret=0;
                break;
            }

            memset(tmp,0,64);
            fscanf(fp,"%s",tmp);
        }

        fclose(fp);
    }

    return ret;
}

int get_next_radius_server(int number,char *addr)
{
    char tmp[20];
    char key[20];
    int num=0;
    int ret=-1;
    FILE *fp=NULL;
    fp = fopen("/etc/radius.tmp","r");

    if (!fp)
    {
        /*printf("%% Open file /etc/radius.tmp failed\r\n");*/
        return -1;
    }

    memset(tmp,0,20);
    fscanf(fp,"%s",tmp);

    while (strlen(tmp))
    {
        num++;

        if (number==num)
        {
            memcpy(addr,tmp,strlen(tmp));
            ret=0;
            break;
        }

        fscanf(fp,"%s",key);
        memset(tmp,0,20);
        fscanf(fp,"%s",tmp);
    }

    fclose(fp);
    return ret;
}

int get_next_tacplus_server(int number,char *addr)
{
    char tmp[20];
    int num=0;
    int ret=-1;
    FILE *fp=NULL;
    fp = fopen("/etc/tacplus.tmp","r");

    if (!fp)
    {
        /*printf("%% Open file /etc/tacplus.tmp failed\r\n");*/
        return -1;
    }

    memset(tmp,0,20);
    fscanf(fp,"%s",tmp);

    while (strlen(tmp))
    {
        num++;

        if (number==num)
        {
            memcpy(addr,tmp,strlen(tmp));
            ret=0;
            break;
        }

        memset(tmp,0,20);
        fscanf(fp,"%s",tmp);
    }

    fclose(fp);
    return ret;
}

int get_dns_server(int number,char *addr,char **name,int count)
{
    if (number>count||number<1)
    {
        printf("Can't get system dns seting\n");
        return -1;
    }
    else
    {
        memcpy(addr,name[number-1],strlen(name[number-1]));
        return 0;
    }
}

int set_sys_desc(char *desc)
{
    FILE * fp=NULL;
    fp = fopen(SYSTEM_DESC_FILE,"w");

    if (!fp)
    {
        return -1;
    }

    fputs(desc,fp);
    fclose(fp);
    return 0;
}

int get_idle_time(char *idle_str)
{
    FILE* fp=NULL;
    char ptr[64];
    char get_idle=0;
    int idle_time;
    memset(ptr,0,64);
    fp = fopen("/var/run/idle_timeout.conf","r");

    if (fp)
    {
        while (fgets(ptr,64,fp))
        {
            if (!strncmp(ptr,"idle_timeout ",13))
            {
                get_idle=1;
                break;
            }
        }

        fclose(fp);
    }

    if (get_idle)
    {
        memcpy(idle_str,ptr+13,strlen(ptr)-13);
    }
    else
    {
        sprintf(idle_str,"%d",10);
    }

    return 0;
}
int set_idle_timeout(int minutes)
{
    FILE* fp=NULL;
    char ptr[64];
    memset(ptr,0,64);
    fp = fopen("/var/run/idle_timeout.conf","w");

    if (!fp)
    {
        return -1;
    }

    sprintf(ptr,"idle_timeout %d\n",minutes);
    fputs(ptr,fp);
    fclose(fp);
    return 0;
}
int get_ripd_socket_buffer(char *rip_buffer)
{
    FILE* confp=NULL;
    int ret=-1;
    confp = fopen(RIPD_SOCKET_BUF_FILE,"r");

    if (!confp)
    {
        fprintf(stdout,"%% Can't open %s file\n",RIPD_SOCKET_BUF_FILE);
        return ret;
    }

    fgets(rip_buffer,64,confp);
    fclose(confp);
    return 0;
}



void get_sys_contact(char *contact)
{
    FILE *fp=fopen(SYS_CONTACT_FILE,"r");

    if (!fp)
    {
        memcpy(contact,"undefined",strlen("undefined"));
    }
    else
    {
        fgets(contact,256,fp);
        fclose(fp);
    }

    return;
}

int set_sys_contact(char *contact)
{
    FILE *fp=fopen(SYS_CONTACT_FILE,"w");

    if (!fp)
    {
        printf("%%Cannot open file %s",SYS_CONTACT_FILE);
        return -1;
    }
    else
    {
        fputs(contact,fp);
        fclose(fp);
    }

    return 0;
}


int get_system_time(char *sys_time)
{
    struct tm *tm_ptr;
    time_t now_time;
    (void)time(&now_time);
    tm_ptr = localtime(&now_time);
    strftime(sys_time, 30, "%Y/%m/%d %H:%M:%S", tm_ptr);
    return 0;
}

int get_system_uptime(char *uptime_str)
{
    FILE *fp;
    char buffer[100];
    char* tmpbuf;
    unsigned int timesec;
    unsigned int days,hours,minutes,tmp;
    fp = fopen("/proc/uptime","r");

    if (NULL == fp)
    {
        printf("Can't get system start time\n");
        return -1;
    }

    if (fgets(buffer,100,fp))
    {
        tmpbuf = strchr(buffer,'.');
        *tmpbuf = '\0';
        timesec = atoi(buffer)+get_offset_time();
        days = timesec/(24*60*60);
        tmp = timesec %(24*60*60);
        hours = tmp/(60*60);
        tmp = tmp % (60*60);
        minutes = tmp/60;
        sprintf(uptime_str,"%d days %d hours %d minutes\n",days,hours,minutes);
    }
    else
    {
        printf("Can't get system start time\n");
        return -1;
    }

    fclose(fp);
    return 0;
}


int sync_sys_description(char *sys_desc)
{
    int ret =-1;
    ret=db_update_via_para(SYS_DESC, 1, sys_desc, NULL);

    if (ret)
    {
        printf("%%Config %s failed \r\n",SYS_DESC);
        return -1;
    }

    return 0;
}

int sync_delete_dns(char *dns_server)
{
    int ret =-1;
    ret= db_entry_delete_via_para(NO_DNS_SERVER, 1, dns_server, DNS_EXTRA);

    if (ret)
    {
        printf("%%Delete dns server %s failed \r\n",dns_server);
        return -1;
    }

    return 0;
}


int get_mem_info(int *total,int *available)
{
    FILE *fp;
    char tmp[16];
    system(SHOW_MEMORY_SCRIPT);
    fp=fopen(MEMINFO_FILE,"r");

    if (!fp)
    {
        /*printf("Open file %s failed \r\n",MEMINFO_FILE);*/
        return -1;
    }

    memset(tmp,0,16);
    fscanf(fp,"%s",tmp);
    *total=atoi(tmp);
    memset(tmp,0,16);
    fscanf(fp,"%s",tmp);
    *available=atoi(tmp);
    fclose(fp);
    return 0;
}


int get_password_info(char *aging_day,char *retry)
{
    FILE *fp;
    fp=fopen(PWD_INFO_FILE,"r");

    if (!fp)
    {
        /*printf("%%Open file %s failed\r\n",PWD_INFO_FILE);*/
        return -1;
    }

    fscanf(fp,"%s",aging_day);
    fscanf(fp,"%s",retry);
    fclose(fp);
    return 0;
}

int download_image_by_url(char *url,char *usr,char *pwd,char *err_msg)
{
    char cmd[512],path[256], path_sav[256], boot_img_name[64], second_boot_name[64];
    char* filename=strrchr(url,'/');
    int ret;

    if (!filename)
    {
        memcpy(err_msg,"The URL is wrong,pls check it",strlen("The URL is wrong,pls check it"));
        return -1;
    }

    filename++;
    memset(cmd,0,512);
    memset(path,0,256);
    memset(path_sav,0,256);
    memset(boot_img_name,0,64);
    memset(second_boot_name,0,64);
    get_boot_img_name(boot_img_name);
    get_2nd_boot_img_name(second_boot_name);

    if (filename)
    {
        int i=0;
        int filenamelen = strlen(filename);

        if (filenamelen > 128)
        {
            return -1;
        }

        sprintf(path,"/mnt/%s.bak",boot_img_name);
        sprintf(path_sav, "/mnt/%s.bak", second_boot_name);
        rename(path, path_sav);
        sprintf(path,"/mnt/%s",boot_img_name);
        sprintf(path_sav, "/mnt/%s", second_boot_name);
        rename(path, path_sav);
        sprintf(cmd,"downimg.sh %s %s %s %s \n",url,usr,pwd,filename);

        if (system(cmd))
        {
            memcpy(err_msg,"Can't download img successly",strlen("Can't download img successly"));
            sprintf(path,"/mnt/%s",boot_img_name);
            sprintf(path_sav, "/mnt/%s", second_boot_name);
            rename(path_sav, path);
            sprintf(path,"/mnt/%s.bak",boot_img_name);
            sprintf(path_sav, "/mnt/%s.bak", second_boot_name);
            rename(path_sav, path);
            return -1;
        }

        sprintf(path,"/mnt/%s",filename);

        if (!check_img(path))
        {
            memset(cmd,0,sizeof(cmd));
            sprintf(path,"/mnt/%s",boot_img_name);
            sprintf(cmd, "cp /mnt/%s %s.bak", filename, path);
            system(cmd);
            sprintf(cmd, "mv /mnt/%s %s", filename, path);
            system(cmd);
            system("sync &");
            memset(cmd,0,sizeof(cmd));
            sprintf(cmd,"sudo file_client -i all %s\n",path);
            system(cmd);
        }
        else
        {
            memcpy(err_msg,"The img file is error",strlen("The img file is error"));
            sprintf(path,"/mnt/%s",boot_img_name);
            sprintf(path_sav, "/mnt/%s", second_boot_name);
            rename(path_sav, path);
            sprintf(path,"/mnt/%s.bak",boot_img_name);
            sprintf(path_sav, "/mnt/%s.bak", second_boot_name);
            rename(path_sav, path);
            return -1;
        }
    }
    else
    {
        memcpy(err_msg,"The URL is wrong",strlen("The URL is wrong"));
        return -1;
    }

    return 0;
}
int reboot_system(char *para)
{
    char cmd[128];
    memset(cmd, 0, 128);

    if (strncmp(para, "all", strlen(para)) == 0)
    {
        system("sudo -s dcli_reboot.sh all 1>/dev/null");
    }
    else
    {
        int slot = atoi(para);

        if (slot > 0 && slot < 16)
        {
            sprintf(cmd, "sudo -s dcli_reboot.sh %d", slot);
            system(cmd);
        }
    }

    return 0;
}
int get_ntp_server(int number, char *srv_addr)
{
    char addr[20];
    int num = 0;
    int ret = -1;
    FILE *fp = NULL;
    fp = fopen(NTP_TMP_FILE, "r");

    if (!fp)
    {
        printf("%% Open file:%s failed\n", NTP_TMP_FILE);
        return -1;
    }

    num++;
    memset(addr, 0, 20);
    fscanf(fp, "%s", addr);

    while (strlen(addr))
    {
        if (num == number)
        {
            memcpy(srv_addr, addr, strlen(addr));
            ret = 0;
            break;
        }

        num++;
        memset(addr, 0, 20);
        fscanf(fp, "%s", addr);
    }

    fclose(fp);
    return ret;
}

SYNC_CONFIG(sync_sys_location, SYS_LOCATION, 1, NULL)
SYNC_CONFIG(sync_login_timeout, SET_IDLE_TIME, 1, NULL)
SYNC_CONFIG(sync_sys_contact, SYS_CONTACT, 1, NULL)
SYNC_CONFIG(sync_add_dns, SET_DNS_SERVER, 1, NULL)
SYNC_CONFIG(sync_pwd_aging_day, PWD_AGING_DAY, 1, NULL)
SYNC_CONFIG(sync_pwd_retry, PWD_RETRY, 1, NULL)

