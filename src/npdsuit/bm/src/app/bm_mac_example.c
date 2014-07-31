
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
typedef struct sys_mac{
	
	unsigned char mac_add[6];
	unsigned char reserve[2];
}sys_mac_add;

sys_mac_add mac_add;

#define BM_MINOR_NUM 0
#define BM_MAJOR_NUM 0xEC

#define BM_IOC_MAGIC 0xEC 
#define BM_IOC_RESET	_IO(BM_IOC_MAGIC,0)




#define BM_IOC_GET_MAC	_IOWR(BM_IOC_MAGIC, 16, sys_mac_add) 

int main(int argc, char **argv) {
	int ret;
	int fd;	
	int i;
	memset(&mac_add,0,sizeof(mac_add));
	fd = open("/dev/bm0",0);
	if(fd == -1)
		{
			printf("open error!");
		}
	ret = ioctl(fd,BM_IOC_GET_MAC,&mac_add);
			for(i = 0; i < 5; i++)
			printf("%02X:",mac_add.mac_add[i]);
			printf("%02X",mac_add.mac_add[5]);
	
			printf("\n");



	
	return ret;;
}
