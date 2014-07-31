/*******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology
********************************************************************************

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* pack_tail.c
*
* MODIFY:
*
*
* CREATOR:
*		pangxf@autelan.com
*
* DESCRIPTION:
*		pack an image file  with a tail   .
*
* DATE:
*		02/15/2012
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.35 $
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include "auteware_tail.h"
#include <sys/time.h>
#include <time.h>
#include "man_sysconf.h"

#define SHOW_USAGE printf("Usage:unpack [-u] image type version outname\n");\
                   printf("      -u       :unpack image\n");\
                   printf("      image    :image name\n");\
                   printf("      type     :image type\n");\
                   printf("      version  :image version\n");\
                   printf("      outname  :output image name\n");return -1;
#define READ_FILE_FAILED printf("Read file %s failed\r\n",file_name); if(fp){fclose(fp);}return -1;

int auteware_tail_generate_tail_tmp(auteware_img_pack_tail* tail_entry)
{
    FILE *fp=fopen(TAIL_TMP,"wb+");
    int ret=-1;
    if(!fp)
    {
        printf("Open file %s error\n",TAIL_TMP);
        return -1;
    }
    ret=fwrite(tail_entry,sizeof(auteware_img_pack_tail),1,fp); 
    fclose(fp);
    if(ret<1)
    {
        printf("Write file %s error\n",TAIL_TMP);        
        unlink(TAIL_TMP);
        return -1;
    }
    return 0;
    
}

int auteware_tail_get_packing_time(auteware_img_pack_tail* tail_entry)
{
    struct timeval tv;               
    gettimeofday(&tv, NULL);     
    tail_entry->time= tv.tv_sec;
    return 0;    
}

static int auteware_tail_check_sum(char *data, int size)
{
	int sum = 0, n = 0;
	while(n < size)
	{
		sum += (unsigned char)(data[n]);
		n++;
	}
	return sum;
}

int auteware_tail_get_length_and_checksum(char *file_name,auteware_img_pack_tail* tail_entry)
{
    struct stat buf;  
    unsigned int len=0;
    unsigned int checksum=0;
    char tmp[BLOCK_SIZE];
    FILE *fp=NULL;
    int ret=-1;
    if(stat(file_name, &buf)<0)      
    {
        READ_FILE_FAILED  
    }    
    tail_entry->length=(unsigned long)buf.st_size; 
    fp=fopen(file_name,"r");
    if(!fp)
    {
        READ_FILE_FAILED
    }
    while(len<tail_entry->length)
    {
        memset(tmp,0,BLOCK_SIZE);
        ret=fread(tmp, 1, BLOCK_SIZE, fp);
        if(ret<1)
        {
            fclose(fp);
            READ_FILE_FAILED
        }
        len+=ret;
        checksum+=auteware_tail_check_sum(tmp, ret);      
    }
    fclose(fp);
    tail_entry->checksum=checksum;
    return 0;
    
    
}

void auteware_tail_config_show(auteware_img_pack_tail* tail_entry)
{
    printf("magic=%s\n",tail_entry->magic);
    printf("type=%s\n",tail_entry->type);
    printf("version=%s\n",tail_entry->version);
    printf("name=%s\n",tail_entry->filename);
    printf("time=%d\n",tail_entry->time);
    printf("length=%d\n",tail_entry->length);
    printf("checksum=%x\n",tail_entry->checksum);
}

int auteware_tail_pack_tail(char *file_name,auteware_img_pack_tail* tail_entry)
{
    int ret=-1;
    char cmd[256]={0};
    ret=auteware_tail_generate_tail_tmp(tail_entry);
    if(ret)
    {
        printf("Generate_tail_tmp failed");
        unlink(TAIL_TMP);
        return -1;
    }
    sprintf(cmd,"%s %s %s",PACK_TAIL_SCRIPT,file_name,tail_entry->filename);
    ret=system(cmd);
    return ret;   
}

int auteware_tail_check_tail(auteware_img_pack_tail* tail_entry)
{
    FILE *fp=fopen(tail_entry->filename,"r");
    char tmp[256]={0};
    int ret=-1;
    if(!fp)
    {
        printf("Cannot open packed file %s\n",tail_entry->filename);
        return -1;
    }
    fseek(fp,-sizeof(auteware_img_pack_tail),SEEK_END);
    ret=fread(tmp, sizeof(auteware_img_pack_tail), 1, fp);
    fclose(fp);
    if(ret<1)
    {
        printf("Cannot read tail\n");
        
        return -1;
    }
    if(memcmp(tmp,tail_entry,sizeof(auteware_img_pack_tail)))
    {
        printf("Check tail failed\n");
        return -1;
    }
    printf("Check tail success\n");
    return 0;
}

int auteware_tail_check_auteware_image(char *file_name,int *block)
{
    int i=0;
    int ret = -1;
    FILE *fp=NULL;
    char magic_str[32];
    int location=0;
    fp=fopen(file_name,"rb");
    if(!fp)
    {
        READ_FILE_FAILED
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
            break;
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

int auteware_tail_unpack_image(char *file_name,int block,char *new_path)
{
    int ret = -1;
    FILE *fp=NULL;
    FILE *new_fp=NULL;
    FILE *short_fp=NULL;
    char new_name[64]={0};
    char short_name[64]={0};
    char *buff=NULL;
    int num=0;
    auteware_img_pack_tail *tail_entry;
    
    fp=fopen(file_name,"rb");
    if(!fp)
    {
        READ_FILE_FAILED
    }
    buff=(char*)malloc(BLOCK_SIZE);
    if(!buff)
    {
        printf("Malloc error in function:auteware_tail_unpack_image\n");
        fclose(fp);
        return -1;
    } 
    //write new image tmp
    new_fp=fopen(IMAGE_TMP,"wb+");
    if(!new_fp)
    {
        printf("Open file %s error\n",IMAGE_TMP);
        fclose(fp);
        free(buff);
        return -1;
    }    
    while(num<block)
    {
        memset(buff,0,BLOCK_SIZE);
        ret = fread(buff,BLOCK_SIZE,1,fp);
        if(ret<1)
        {
            fclose(fp);
            fclose(new_fp);
            free(buff);           
            READ_FILE_FAILED
            
        }
        fwrite(buff,BLOCK_SIZE,1,new_fp);  
        num++;
    }
    memset(buff,0,BLOCK_SIZE);
    ret = fread(buff,sizeof(auteware_img_pack_tail),1,fp);
    if(ret<1)
    {
        fclose(fp);
        fclose(new_fp);
        free(buff);
        READ_FILE_FAILED       
    }
    fwrite(buff,sizeof(auteware_img_pack_tail),1,new_fp); 
    fclose(new_fp);
    tail_entry=(auteware_img_pack_tail*)buff;
    //change the new image tmp to normal name
    sprintf(new_name,"/mnt/%s.sav",tail_entry->filename);
    unlink(new_name);
    rename(IMAGE_TMP,new_name);
    //output the new image path
    memcpy(new_path,new_name,strlen(new_name));
    //write short image tmp    
    sprintf(short_name,"%s.tmp",file_name);
    short_fp=fopen(short_name,"wb+");
    if(!short_fp)
    {
        printf("Open file %s error\n",short_name);
        fclose(fp);
        free(buff);
        return -1;
    }
    while(1)
    {
        memset(buff,0,BLOCK_SIZE);
        ret = fread(buff,1,BLOCK_SIZE,fp);
        fwrite(buff,ret,1,short_fp);  
        if(ret==0)
        {      
            fclose(short_fp); 
            break;
        }
    }
    fclose(fp);
    free(buff);
    //short image tmp replace old image
    unlink(file_name);
    rename(short_name,file_name);
    return 0;
}

int auteware_tail_handle_last_image_name(char *file_name,char *new_path)
{
    auteware_img_pack_tail *tail_entry=NULL;
    char tmp[256]={0};
    char name[128]={0};
    FILE *fp=fopen(file_name,"rb");   
    if(!fp)
    {
        READ_FILE_FAILED
    }
    fseek(fp, -sizeof(auteware_img_pack_tail), SEEK_END); 
    fread(tmp,sizeof(auteware_img_pack_tail),1,fp);
    fclose(fp);
    tail_entry=(auteware_img_pack_tail*)tmp;
    if(strncmp(file_name, "/tmp/update", 11) == 0){
        //printf("%s : %s[%d]\n", __FILE__, __FUNCTION__, __LINE__);
        sprintf(name,"/tmp/update/%s.sav",tail_entry->filename);
    } else {
        sprintf(name,"/mnt/%s.sav",tail_entry->filename);
    }
    
    rename(file_name,name);
    //output the new image name
    memcpy(new_path,name,strlen(name));
    return 0;    
    
}


int auteware_tail_handle_unpacked_image(board_type_list *bth,hw_compat_list *hw_compat_head,char *file_name, int local_flag)
{
    int ret=-1;
    char cmd[256]={0};
    ret=check_image_is_compatiple(file_name,hw_compat_head,bth);
    if(ret)
    {
		if(local_flag == 0)
		{
            ret = send_image_by_board_type(file_name,hw_compat_head,bth);
		}
        return 0;
    }
	if(local_flag == 0)
	{
        ret=send_image_by_board_type(file_name,hw_compat_head,bth);
	}
    sprintf(cmd,"%s %s",HANDLE_IMG_SCRIPT,file_name);
    ret=system(cmd);
    return ret;
}

int auteware_tail_unpack_image_recursive(char *filename, int local_flag)
{
    int ret=-1;
    int block=0;
    char new_image[128];
    board_type_list *bth=init_running_board_type_list();
    hw_compat_list *hw_compat_h=man_parse_hw_compat_list(bth);
    show_running_board_type(bth);
    show_hw_compat_list(hw_compat_h);
    while(1)
    {
        ret=auteware_tail_check_auteware_image(filename,&block);
        if(ret==IMG_NO_OPERATION)
        {
            break;
        }
        if(ret==0)
        {
            memset(new_image,0,128);
            ret=auteware_tail_unpack_image(filename, block,new_image);
            if(ret!=0)
            {
                printf("Unpack image error\n");
                break;
                ret=-1;
            }
            ret=auteware_tail_handle_unpacked_image(bth,hw_compat_h,new_image, local_flag);
            if(ret)
            {
                printf("Unpack image failed\n");
                ret=-1;
                break;               
            }
        }
        if(ret<0)
        {
            printf("Function check_auteware_image failed\n");
            ret=-1;
            break;            
        }
    }
    if(ret==IMG_NO_OPERATION)
    {
        memset(new_image,0,128);
        ret=auteware_tail_handle_last_image_name(filename,new_image);
        ret=auteware_tail_handle_unpacked_image(bth,hw_compat_h,new_image, local_flag);
    }
    release_board_type_list(bth);
    release_hw_compat_list(hw_compat_h);
    return ret;    
}

int main(int argc, char* argv[], char* dummy[])
{
    int ret=-1;
    auteware_img_pack_tail auteware_tail;
    char file_name[32]={0};
    char file_out[32]={0};
    char file_type[32];
    char cmd[256]={0};
	int local_flag = 0;
    if(argc==3)
    {
		if(strcmp(argv[1],"-l") == 0)
		{
			local_flag = 1;
		}
		else if(strcmp(argv[1],"-u") == 0)
		{
			local_flag = 0;
		}
		else
        {
            SHOW_USAGE            
        }
        memcpy(file_name,argv[2],strlen(argv[2]));
        ret=get_image_type(file_name,file_type);
        if(ret)
        {
            printf("Old image without tail\r\n");
            //check the image and send out
            if(!app_box_state_get() && (local_flag == 0))
            {
                printf("Send image to all online slots\r\n");
                memset(cmd,0,sizeof(cmd));
                sprintf(cmd,"file_client -i all %s\n",file_name);
                system(cmd);
            }
            memset(cmd,0,sizeof(cmd));
            sprintf(cmd,"%s %s",HANDLE_IMG_SCRIPT,file_name);
           
            return system(cmd);
        }
        ret=auteware_tail_unpack_image_recursive(file_name, local_flag);
        if(ret)
        {
            printf("Unpack image error\n");
            return -1;
        }
        return 0;
        
    }
    if(argc<6)
    {
        SHOW_USAGE
    } 
     //get input name 
    memcpy(file_name,argv[1],strlen(argv[1]));
    memset(&auteware_tail,0,sizeof(auteware_img_pack_tail));
    //get unpacked name
    memcpy(auteware_tail.filename,argv[2],strlen(argv[2]));
    //get type 
    memcpy(auteware_tail.type,argv[3],strlen(argv[3]));
    //get version
    memcpy(auteware_tail.version,argv[4],strlen(argv[4]));
    //get output name 
    memcpy(file_out,argv[5],strlen(argv[5]));
    auteware_tail_get_packing_time(&auteware_tail);
    ret=auteware_tail_get_length_and_checksum(file_name,&auteware_tail);
    if(ret)
    {
        printf("Function auteware_tail_get_length_and_checksum failed\n");
        return -1;
    }
    memcpy(auteware_tail.magic,AUTEWARETAIL,16);
    //config_show(&auteware_tail);
    ret=auteware_tail_pack_tail(file_name,&auteware_tail);
    if(ret)
    {
        printf("Function auteware_tail_pack_tail failed\n");
    }
    ret=auteware_tail_check_tail(&auteware_tail);
	return ret;
}

