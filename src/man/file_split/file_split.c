
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* file_split.c
*
* MODIFY:
*
*
* CREATOR:
*		pangxf@autelan.com
*
* DESCRIPTION:
*		Split an image file
*
* DATE:
*		12/15/2012
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.35 $
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h> /*suijz 2012.12.25 malloc() free()*/
#include <unistd.h>
#include <sys/reboot.h> /*suijz 2012.12.25  reboot()*/

#include <sys/ioctl.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include "file_split.h"
#include "man_sysconf.h"
#include <dirent.h>
#include <sys/stat.h>
#include "squashfs_fs.h"
#include <fcntl.h>
#include <mntent.h>
#include <sys/statfs.h>

#define _PATH_MOUNTED_ "/mnt"

int get_version_and_time(char* filename,char *version,uint32_t *tm);
int check_sq_image(char *img,int *block);
int file_split_check_root_size(char *img_name, int block);
int get_boot_image_name(char* imgname);
int check_image_root(char *new_img,char *root,int block);
int get_sq_image(char *img,int block);

int file_split_is_file_exist(char *filtpath)
{
    return access(filtpath, 0);
}

int file_split_get_mnt_free_space()
{
    struct    statfs    stat;
    int    fre = 0;
    bzero(&stat,sizeof(struct statfs));

    if (-1 == statfs(_PATH_MOUNTED_,&stat))
    {
        printf("file_split_get_mnt_free_space error\r\n");
        return 0;
    }

    fre    =    stat.f_bsize*stat.f_bfree/1024;
    return fre;
}


unsigned long file_split_get_file_size(const char *filename)
{
    struct stat buf;

    if (stat(filename, &buf)<0)
    {
        return 0;
    }

    return (unsigned long)buf.st_size;
}



int file_split_check_space_num(int image_size)
{
    int freesize = 0;
    int rate = 0;
    freesize = file_split_get_mnt_free_space();

    if (image_size>0)
    {
        rate = freesize/image_size;
    }
    else
    {
        rate = 0;
    }

    return rate;
}

int file_split_get_2nd_boot_img_name(char* imgname)
{
    int fd;
    int retval;
    boot_env_t	env_args;
    char *name = "secondary_bootfile";

    memset(&env_args, 0, sizeof(env_args));
    sprintf(env_args.name,name);
    env_args.operation = GET_BOOT_ENV;
    
    fd = open("/dev/bm0", 0);

    if (fd < 0)
    {
        return 1;
    }

    retval = ioctl(fd,BM_IOC_ENV_EXCH,&env_args);

    if (retval == -1)
    {
        sprintf(imgname, "aw.2nd.img");
        sprintf(env_args.value,imgname);
        env_args.operation = SAVE_BOOT_ENV;
        retval = ioctl(fd, BM_IOC_ENV_EXCH, &env_args);

        if (retval == -1)
        {
            close(fd);
            return 2;
        }
    }
    else
    {
        sprintf(imgname,env_args.value);
    }

    close(fd);
    return 0;
}

int file_split_get_boot_img_name(char* imgname)
{
    int fd;
    int retval;
    boot_env_t	env_args;
    char *name = "bootfile";
    memset(&env_args, 0, sizeof(env_args));
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
        sprintf(imgname, "aw.img");
        sprintf(env_args.value,imgname);
        env_args.operation = SAVE_BOOT_ENV;
        retval = ioctl(fd, BM_IOC_ENV_EXCH, &env_args);

        if (retval == -1)
        {
            close(fd);
            return 2;
        }
    }
    else
    {
        sprintf(imgname,env_args.value);
    }

    close(fd);
    return 0;
}


static inline unsigned int file_split_swap32(unsigned int x)
{
    return ((unsigned int)((((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) |
                           (((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) |
                           (((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) |
                           (((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24)));
}


int get_image_file_name(char *version,char *file_name, int *block_num)
{
    char full_name[128];
    char tmp_file_name[128];
    char file_version[64];
    uint32_t tmp_tm=0;
    uint32_t cur_tm=0;
    int ret=-1;
    int flag=0;
    struct dirent *ptr;
    struct stat sb;
    DIR *dir;
    int block_no = 0;
    dir=opendir("/mnt");

    while ((ptr=readdir(dir))!=NULL)
    {
        if (stat(ptr->d_name, &sb) >= 0 && S_ISDIR(sb.st_mode))
        {
            continue;
        }

        memset(full_name,0,128);
        memset(file_version,0,64);
        sprintf(full_name,"/mnt/%s",ptr->d_name);
        ret=get_version_and_time(full_name,file_version,&tmp_tm);

        if (ret)
        {
            continue;
        }

        ret = check_sq_image(full_name, &block_no);

        if (ret)
        {
            continue;
        }

        if (!strcmp(file_version,version))
        {
            if (tmp_tm>cur_tm)
            {
                flag=1;
                *block_num = block_no;
                cur_tm = tmp_tm;
                memset(tmp_file_name,0,128);
                strncpy(tmp_file_name,full_name,strlen(full_name));
            }
        }
    }

    closedir(dir);

    if (flag)
    {
        strncpy(file_name,tmp_file_name,strlen(tmp_file_name));
        return 0;
    }

    return -1;
}


int file_split_check_root_valid(char *img_name, char *root_name, int block)
{
    int root_size = 0;
    int root_file_size = 0;
    int ret = -1;
    root_size = file_split_check_root_size(img_name, block);
    root_file_size = file_split_get_file_size(root_name);

    if (root_size == root_file_size)
    {
        ret = 0;
    }
    else
    {
        printf("root size error \r\n");
    }

    return ret;
}


int update_root_by_version(char *version)
{
    char img_name[128]={0};
    char root_name[64];
    int ret=-1;
    int block=0;
    //get_boot_file_name
    ret=get_boot_image_name(img_name); //no content function

    if (ret)
    {
        ret=get_image_file_name(version,img_name, &block);

        if (ret)
        {
            printf("Cannot get boot image file\n");
            return -1;
        }
    }

    sprintf(root_name,"/mnt/%s.root",version);
	
    ret=check_image_root(img_name,root_name,block);

    if (ret == 0)
    {
        ret=get_sq_image(img_name,block);

        if (ret)
        {
            printf("generate image error\n");
            return -1;
        }
    }

    return ret;
}

int get_boot_image_name(char* imgname)
{
    return -1;
}

int get_version_and_time(char* filename,char *version,uint32_t *tm)
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

    buf=(char *)malloc(buff_len);

    if (!buf)
    {
        fclose(fd);
        return -1;
    }

    read_len = fread(buf, 1, buff_len, fd);

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

    strncpy(version, (const char *)&(ptr->ih_name[2]), strlen((const char *)ptr->ih_name)-2);
    *tm=ntohl(ptr->ih_time);
    fclose(fd);
    free(buf);
    return 0;
}


int file_split_check_root_size(char *img_name, int block)
{
    unsigned long img_size = file_split_get_file_size(img_name);
    return img_size - BLOCK_SIZE*block;
}

int file_split_unlink_root(char *active_root)
{
    char full_name[128];
    uint32_t magic_num=0, magic = 0;
    int ret=-1;
    struct dirent *ptr;
    struct stat sb;
    DIR *dir;
	
	if(active_root == NULL)
	{
		return -1;
	}

    dir = opendir("/mnt");

    while ((ptr = readdir(dir)) != NULL)
    {
        FILE *fp=NULL;
        if (stat(ptr->d_name, &sb) >= 0 && S_ISDIR(sb.st_mode))
        {
            continue;
        }
		if(strcmp(ptr->d_name, active_root) == 0)
		{
			continue;
		}

        memset(full_name, 0, 128);
		
        sprintf(full_name,"/mnt/%s",ptr->d_name);
		
        fp = fopen(full_name, "rb");
    
        if (!fp)
        {
            continue;
        }
        
        ret = fread(&magic_num,4,1,fp);

	    fclose(fp);
		
        if (ret != 1)
        {
            continue;
        }
        magic = magic_num;
        magic_num = file_split_swap32(magic_num);

        if (magic==SQUASHFS_MAGIC||magic_num==SQUASHFS_MAGIC)
        {
			unlink(full_name);
        }
    }

    closedir(dir);
    return 0;
}

int check_image_root(char *new_img,char *root,int block)
{
    FILE *fp_img=NULL;
    FILE *fp_root=NULL;
    int block_no = 0;
    char *img_buff = NULL;
    char *root_buff = NULL;
    int img_read_len = 0;
    int root_read_len = 0;

    fflush(stdout);
    fp_img=fopen(new_img,"rb");

    if (!fp_img)
    {
        printf("open image file error.\n");
        return -1;
    }

    fp_root = fopen(root,"rb");

    if (!fp_root)
    {
        fclose(fp_img);
        return 0;
    }

    fseek(fp_img, block*CHECK_IMG_SIZE,SEEK_SET);
	
	img_buff = (char *)malloc(BLOCK_SIZE);
	if(img_buff == NULL)
	{
		printf("no enough memory.\n");
        fclose(fp_img);
        fclose(fp_root);
        reboot(0x01234567);
        return -1;
    }

    root_buff = (char *)malloc(BLOCK_SIZE);

    if (root_buff == NULL)
    {
        printf("no enough memory.\n");
        fclose(fp_img);
        fclose(fp_root);
        free(img_buff);
        reboot(0x01234567);
        return -1;
    }

    do
    {
        if (block_no++%16 == 0)
        {
            printf(".");
            fflush(stdout);
        }

        img_read_len = fread(img_buff, 1, BLOCK_SIZE, fp_img);
        root_read_len = fread(root_buff, 1, BLOCK_SIZE, fp_root);

        if (img_read_len != root_read_len)
        {
            printf(" size error. Will generate a new rootfs.\n");
            fclose(fp_img);
            fclose(fp_root);
            free(img_buff);
            free(root_buff);
            unlink(root);
            return 0;
        }

        if (0 != memcmp(img_buff, root_buff, img_read_len))
        {
            if (block_no == 1)
            {
                printf(" version mismatch. Will generate a new rootfs.\n");
            }
            else
            {
                printf(" bit error. Will generate a new rootfs.\n");
            }

            fclose(fp_img);
            fclose(fp_root);
            free(img_buff);
            free(root_buff);
            unlink(root);
            return 0;
        }
    }
    while (img_read_len == BLOCK_SIZE);

    printf(" OK.\n");
    fclose(fp_img);
    fclose(fp_root);
    free(img_buff);
    free(root_buff);
    return 1;
}

int get_img_version(char* filename,char *version)
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

    strncpy(version, (const char *)&(ptr->ih_name[2]),strlen((const char *)ptr->ih_name)-2);
    fclose(fd);
    free(buf);
    return 0;
}

int get_sq_image(char *img,int block)
{
    int ret = -1;
    FILE *fp=NULL;
    FILE *root_fp=NULL;
    char version[32]={0};
    char root_name[64];
    char *buff=NULL;
    int write_ret = -1;
    ret=get_img_version(img, version);

    if (ret)
    {
        printf("get_img_version error\n");
        return -1;
    }

    sprintf(root_name,"/mnt/%s.root",version);
    //printf("the root file is %s\n",root_name);
    //check the root exits or not,if exits,check the content matches or not
    //if the im image is newer,replace
    fp=fopen(img,"rb");

    if (!fp)
    {
        printf("open file %s error\n",img);
        return -1;
    }

    buff=(char*)malloc(BLOCK_SIZE);

    if (!buff)
    {
        printf("malloc error\n");
        fclose(fp);
        return -1;
    }

    root_fp=fopen(root_name,"wb+");

    if (!root_fp)
    {
        printf("open file error\n");
        fclose(fp);
        free(buff);
        return -1;
    }

    fseek(fp, BLOCK_SIZE*block, SEEK_SET);
    ret = BLOCK_SIZE;

    while (ret==BLOCK_SIZE)
    {
        memset(buff,0,BLOCK_SIZE);
        ret = fread(buff,1,BLOCK_SIZE,fp);

        if (ret > 0)
        {
            write_ret = fwrite(buff,1,ret,root_fp);

            if (write_ret != ret)
            {
                printf("Write rootfs error, will reboot to try again.\r\n");
                fclose(root_fp);
                fclose(fp);
                free(buff);
                reboot(0x01234567);
                return -1;
            }
        }
    }

    fclose(root_fp);
    fclose(fp);
    free(buff);
    return 0;
}

int check_sq_image(char *img,int *block)
{
    unsigned int magic=0;
    unsigned int magic_num = 0;
    int i=0;
    int ret = -1;
    FILE *fp=NULL;
    //printf("the image file is %s\n",img);
    fp=fopen(img,"rb");

    if (!fp)
    {
        printf("open file %s error\n",img);
        return -1;
    }

    ret = fseek(fp, BLOCK_SIZE*i, SEEK_SET);

    while (ret == 0)
    {
        ret = fread(&magic_num,4,1,fp);

        if (ret != 1)
        {
            ret =-1;
            break;
        }

        magic=magic_num;
        magic_num = file_split_swap32(magic_num);

        if (magic==SQUASHFS_MAGIC||magic_num==SQUASHFS_MAGIC)
        {
            ret=0;
            *block=i;
            break;
        }

        i++;
        ret = fseek(fp, BLOCK_SIZE*i, SEEK_SET);
    }

    fclose(fp);
    return ret;
}

int handle_image_files(char *img)
{
    char cmd[256]={0};
    int ret=-1;
    char img_ver[32] = {0};
    char root_name[64]={0};
    char boot_img_name[64]={0};
    char second_boot_img_name[64]={0};
    char boot_img_path[128]={0};
    char second_boot_img_path[128]={0};
    char boot_bak_path[128]={0};
    char second_boot_bak_path[128]={0};
    int space_num = 0;
    int img_size ;
    ret=file_split_get_boot_img_name(boot_img_name);

    if (ret==1)
    {
        printf("file_split_get_boot_img_name error\r\n");
        return -1;
    }

    ret=file_split_get_2nd_boot_img_name(second_boot_img_name);

    if (ret==1)
    {
        printf("file_split_get_2nd_boot_img_name error\r\n");
        return -1;
    }

    sprintf(second_boot_img_path,"/mnt/%s",second_boot_img_name);
    sprintf(boot_img_path,"/mnt/%s",boot_img_name);
    ret=get_img_version(boot_img_path, img_ver);

    if (ret)
    {
        printf("Can not get the running image version.\r\n");
    }
    sprintf(root_name,"%s.root",img_ver);
    file_split_unlink_root(root_name);

    img_size= file_split_get_file_size(img)/1024;

    //Mv aw.img second.img
    if (strcmp(second_boot_img_path, boot_img_path))
    {
        unlink(second_boot_img_path);
        rename(boot_img_path,second_boot_img_path);
    }

    //Mv aw.img.bak second.img.bak
    sprintf(second_boot_bak_path,"/mnt/%s.bak",second_boot_img_name);
    sprintf(boot_bak_path,"/mnt/%s.bak",boot_img_name);

    if (strcmp(second_boot_img_path, img))
    {
        unlink(second_boot_bak_path);
        space_num = file_split_check_space_num(img_size);

        if (space_num > 2)
        {
            rename(boot_bak_path,second_boot_bak_path);
        }
        else
        {
            unlink(boot_bak_path);
        }
    }

    //Mv aw.img.sav aw.img
    unlink(boot_img_path);
    rename(img,boot_img_path);
    //Cp aw.img aw.img.bak
    space_num = file_split_check_space_num(img_size);

    if (space_num > 2)
    {
        memset(cmd,0,256);
        sprintf(cmd,"cp %s %s",boot_img_path,boot_bak_path);
        system(cmd);
        system("sync &");
    }

    if (space_num <= 2)
    {
        ret = file_split_is_file_exist(second_boot_bak_path);

        if (!ret)
        {
            unlink(second_boot_bak_path);
            return 0;
        }

        ret = file_split_is_file_exist(boot_bak_path);

        if (!ret)
        {
            unlink(boot_bak_path);
            return 0;
        }

        if (strcmp(second_boot_img_path, boot_img_path))
        {
            ret = file_split_is_file_exist(second_boot_img_path);

            if (!ret)
            {
                unlink(second_boot_img_path);
                return 0;
            }
        }
    }

    return 0;
}

/*suijz 20130131*/
int handle_image_files_spi(char *img)
{
	int ret=-1;
	char boot_img_name[64]={0};
	char boot_img_path[128]={0};

	ret=file_split_get_boot_img_name(boot_img_name);
	if (ret==1)
	{
		printf("[spi]file_split_get_boot_img_name error\r\n");
		return -1;
	}
	 if(strncmp(img, "/tmp/update", 11) == 0){
		sprintf(boot_img_path,"/tmp/update/%s",boot_img_name);
	 } else {
		sprintf(boot_img_path,"/mnt/%s",boot_img_name);
	 }
	unlink(boot_img_path);
	rename(img, boot_img_path);

	return 0;
}


int down_sq_image(char *img)
{
    int ret=-1;
    int block=0;
    ret=check_sq_image(img,&block);

    if (ret)
    {
        printf("the image is not sq image\n");
        return -1;
    }

#if 0
    ret=get_sq_image(img,block);

    if (ret)
    {
        printf("generate image error\n");
    }

#endif
    return ret;
}

/*==============SUIJZ 2012.12.25==============*/
int exhange_dir_get_sq_image(char *img, int block, char *new_dir)
{
    int ret = -1;
    FILE *fp=NULL;
    FILE *root_fp=NULL;
    char version[32]={0};
    char root_name[64];
    char *buff=NULL;
    int write_ret = -1;
    ret=get_img_version(img, version);

    if (ret)
    {
        printf("[new_dir]get_img_version error\n");
        return -1;
    }

    sprintf(root_name,"/%s/%s.root", new_dir, version);
    //printf("the root file is %s\n",root_name);
    //check the root exits or not,if exits,check the content matches or not
    //if the im image is newer,replace
    fp=fopen(img,"rb");

    if (!fp)
    {
        printf("[new_dir]open file %s error\n",img);
        return -1;
    }

    buff=(char*)malloc(BLOCK_SIZE);

    if (!buff)
    {
        printf("[new_dir]malloc error\n");
        fclose(fp);
        return -1;
    }

    root_fp=fopen(root_name,"wb+");

    if (!root_fp)
    {
        printf("[new_dir]open file error\n");
        fclose(fp);
        free(buff);
        return -1;
    }

    fseek(fp, BLOCK_SIZE*block, SEEK_SET);
    ret = BLOCK_SIZE;

    while (ret==BLOCK_SIZE)
    {
        memset(buff,0,BLOCK_SIZE);
        ret = fread(buff,1,BLOCK_SIZE,fp);

        if (ret > 0)
        {
            write_ret = fwrite(buff, 1, ret, root_fp);

            if (write_ret != ret)
            {
                printf("[new_dir]Write rootfs error, will reboot to try again.\r\n");
                fclose(root_fp);
                fclose(fp);
                free(buff);
                reboot(0x01234567);
                return -1;
            }
        }
    }

    fclose(root_fp);
    fclose(fp);
    free(buff);
    return 0;
}

int exchange_dir_update_root(char *version, char *new_dir)
{
    char img_name[128]={0};
    char root_name[64];
    int ret=-1;
    int block=0;
    //get_boot_file_name
    ret=get_boot_image_name(img_name); //no content function

    if (ret)
    {
        ret=get_image_file_name(version,img_name, &block);

        if (ret)
        {
            printf("[new_dir]Cannot get boot image file\n");
            return -1;
        }
    }

    sprintf(root_name,"/%s/%s.root", new_dir, version);
    ret=check_image_root(img_name,root_name,block);

    if (ret == 0)
    {
        ret=exhange_dir_get_sq_image(img_name, block, new_dir);

        if (ret)
        {
            printf("[new_dir]generate image error\n");
            return -1;
        }
    }

    return ret;
}
/*============SUIJZ 2012.12.25 end=========*/

int main(int argc, char* argv[], char* dummy[])
{
    char img_name[64]={0};
    char new_dir[64] = {0}; /*SUIJZ 2012.12.25*/
    int ret=-1;

    if (argc<2)
    {
        printf("Usage:file_split -i -n name -s\r\n");
        printf("-i:handle image file \r\n");
        printf("name:the path of the image file \r\n");
        return -1;
    }

    if (!strcmp(argv[1],"-i"))
    {
        if (argc<4)
        {
            printf("image para error\r\n");
            return -1;
        }

        if (!strcmp(argv[2],"-n"))
        {
            strncpy(img_name,argv[3],strlen(argv[3]));
        }

        if (argc==5)
        {
            if (!strcmp(argv[4],"-s"))
            {
                ret=down_sq_image(img_name);

                if (ret)
                {
                    printf("down_sq_image error\n");
                    return -1;
                }
            }
        }

        ret=handle_image_files(img_name);

        if (ret)
        {
            printf("handle_image_files error\n");
        }
    }

    if (!strcmp(argv[1],"-d"))
    {
        if (argc != 4)
        {
            return -1;
        }

        strncpy(new_dir,argv[2],strlen(argv[2]));
        ret = exchange_dir_update_root(argv[3], new_dir);
    }

	/*spiflash : rename, no bak for update image*/
     if(!strcmp(argv[1],"-t")){
	 //printf("[spiflash] ===========\n");
	 if(argc != 3){
		printf("[SPIFLASH] parameter error\n");
		return -1;
	 }
         strncpy(img_name,argv[2],strlen(argv[2]));
	  ret=handle_image_files_spi(img_name);
         if (ret)
         {
             printf("[spiflash]handle_image_files error\n");
         }
     }

    if (argc==2)
    {
        ret=update_root_by_version(argv[1]);
    }

    return ret;
}

