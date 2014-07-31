/*
board management application --- read address in kernel address sspace

*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <util/npd_list.h>

#include "../kmod/t9000_series/bmk_main.h"

int DEBUG=0;
#undef DBG
#define DBG(f,x...) \
	if (DEBUG) { printf("DEBUG " f,##x); }

static char  help_msg[] = "\
Usage: bmutil -d -r -w -l -a [address] -n [bitnum] -v [value]\n\
-d\t\tDebug input value(Should be input as the first option).\n\
-r\t\tread operation\n\
-w\t\twrite operation\n\
-l\t\tDestination little endian, system default is big endian.\n\
-a\t\tAddress(hex) to be read or write.\n\
-n\t\tnumber of bits(only 8,16,32,64 is accept,default is 64) to be read or write.\n\
-v\t\tValue to write(hex)\n\n";

bm_op_args bmu_op_args;

unsigned int reverse_int(unsigned int or) {
	unsigned int i;
	unsigned char *p = (unsigned char *)&i;
	unsigned char *c = (unsigned char *)&or;
	
	p[0] = c[3];
	p[1] = c[2];
	p[2] = c[1];
	p[3] = c[0];
	
	return i;
}

int print_result(bm_op_args *ops) {

	printf(" got [%d] bits value ",ops->op_len);
	switch (ops->op_len) {
		case 8:
			printf("              [0x%02x]\n",(unsigned char)ops->op_value);
			break;
		case 16:
			printf("            [0x%04x]\n",(unsigned short)ops->op_value);
			break;
		case 32:
			printf("        [0x%08x]\n",(unsigned int)ops->op_value);
			break;
		case 64:
			printf("[0x%016llx]\n",ops->op_value);
			break;
		default:
			printf("ERROR value len illegal\n");
			break;
	}
	
	DBG("64bit value[0x%016llx]\n",ops->op_value);
	return 0;

}


int read_addr(bm_op_args * ops){
	int fd;
	int retval;
	fd = open("/dev/bm0",0);
	
	retval = ioctl (fd,BM_IOC_G_,ops);
	if (0 ==retval) {
		if (BM_LITTLE_ENDIAN == ops->op_byteorder) {
			if (32 == ops->op_len) {
				ops->op_value = (unsigned long long) reverse_int((unsigned int)ops->op_value);
			}
		}
		printf("Read at addr [0x%016llx]",ops->op_addr);
		print_result(ops);
	} else {
		printf("Read failed return [%d]\n",retval);
	}

	//printf("FD is %d,addr [0x%x] Return value is [0x%0lx]\n",fd,addr,val);
	close(fd);
	return retval;

}
	
int write_addr(bm_op_args * ops) {
	int fd;
	int retval;
	fd = open("/dev/bm0",0);
	
	printf("Writ at addr [0x%016llx] use [%d] bits value [0x%016llx]\n",ops->op_addr,ops->op_len,ops->op_value);
	if (BM_LITTLE_ENDIAN == ops->op_byteorder) {
		if (32 == ops->op_len) {
			ops->op_value = (unsigned long long) reverse_int((unsigned int)ops->op_value);
		}
	}
		
	retval = ioctl (fd,BM_IOC_X_,ops);
	if (0 ==retval) {
		if (BM_LITTLE_ENDIAN == ops->op_byteorder) {
			if (32 == ops->op_len) {
				ops->op_value = (unsigned long long) reverse_int((unsigned int)ops->op_value);
			}
		}
		printf("R &w at addr [0x%016llx]",ops->op_addr);
		print_result(ops);
	} else {
		printf("Read failed return [%d]\n",retval);
	}

	//printf("FD is %d,addr [0x%x] Return value is [0x%0lx]\n",fd,addr,val);
	close(fd);
	return retval;

}
	
	

int main(int argc, char **argv) {
	int optype=0; // 1for read, 2 for write;
	int opt;
	int value_to_set = 0;
	int ret;
	
	memset(&bmu_op_args,0,sizeof(bmu_op_args));
	bmu_op_args.op_len = 64;
	
	
	while ((opt = getopt(argc,argv,"drwla:n:v:")) != -1) {
		switch (opt) {
			case 'd':
				DEBUG = 1;
				break;
			case 'r':
				optype = 1;	
				DBG("Read operation\n");
				break;
			case 'w':
				optype = 2;
				DBG("Write operation\n");
				break;
			case 'a':
				bmu_op_args.op_addr = strtoull(optarg,NULL,16);
				DBG("Input addr %s got 0x%016llx\n",optarg,bmu_op_args.op_addr);
				break;
			case 'l':
				bmu_op_args.op_byteorder = 1;
				break;
			case 'n':
				bmu_op_args.op_len = strtoul(optarg,NULL,0);
				switch (bmu_op_args.op_len) {
					case 8:
					case 16:
					case 32:
					case 64:
						break;
					default:
						printf("Length must be 8,16,32 or 64.\n");
						exit(0);
						break;
				};
				DBG("Input len %s got %d\n",optarg,bmu_op_args.op_len);
				break;
			case 'v':
				bmu_op_args.op_value= strtoull(optarg,NULL,16);
				value_to_set = 1;
				DBG("Input value %s got %0llx\n",optarg,bmu_op_args.op_value);
				break;
			default: /* '?' */
				printf("%s",help_msg);
				exit(0);
		}
	}			
	
	if ((0 == optype)||(0 == bmu_op_args.op_addr)) {
		printf("%s",help_msg);
		return 0;
	}
	
	if ((2 == optype) && (0 == value_to_set)) {
		printf("%s",help_msg);
		return 0;
	};
	
	if (1 == optype)	{
		ret = read_addr(&bmu_op_args);
	} else if ( 2 == optype) {
		ret = write_addr(&bmu_op_args);
	}	
	
       
	return ret;;
}
