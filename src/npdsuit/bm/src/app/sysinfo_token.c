#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "bm_cpld_util.h"

int read_file_buf (char *pathname, char *buf, int * plen)
{
	int fd;
	int len;
	
	fd = open(pathname, O_RDONLY);
	if (fd < 0)
	{
		DBG("open file %s error.\n", pathname);
		return -1;
	}
	len = read(fd, buf, *plen);
	if (len < 0)
	{
		DBG("read file %s error.\n", pathname);
		return -1;
	}
	DBG("read length is %d.\n", len);
	close (fd);
	
	return len;
}

int content_token(char *buf, char** name_buf, char ** val_buf, int * pcount)
{
	char *p;
	char *buffer;
	char * delims_newline = "\n", *delims_colon = ":";
	int count = 0;
	char * str_buf[100];
	int index;
	char * temp;
	
	buffer = buf;
	DBG("source str: %s.\n", buffer);
	p = strtok(buffer, delims_newline);
	while(p != NULL)
	{
		str_buf[count++] = p;
		p = strtok(NULL, delims_newline);
	}
	DBG("remain buf is %s.\n", buffer);
	
	for (index = 0; index < count; index++)
	{
		DBG("%d:%s\n",index, str_buf[index]);
	}
	
	//token the str_buf into two buf.
	
	for (index = 0; index < count; index++)
	{
		temp = strtok(str_buf[index], delims_colon);
		name_buf[index] = temp;
		temp = strtok(NULL, ":");
		val_buf[index] = temp;
	
		DBG("index-%d, name-%s, val-%s.\n", index,
	         	name_buf[index], val_buf[index]);
	}
	
	*pcount = count;

	return 0;
}

int ax_simple_tool_itonstr(int count, char* char_addr, int char_num)
{
	int i, j;
	int n;
	for (i = char_num-1, j = 1; i >= 0; i--, j*=10)
	{
		n = (count % (10 * j))/j;
		if (n > 9 || n < 0)
		{
			DBG("the n conver error.\n");
			return -1;
		}
		char_addr[i] = n + '0';
	}
	return 0;
}

int tool_get_elemtype(char * elem_name)
{
	int index;
	int arr_length;
	struct elem_type
	{
		int type_code;
		char * name;
	};
	
	struct elem_type elem_type_arr [] =
	{
		{1,  	"module_sn"},
		{2, 	"module_name"},
		{3, 	"product_sn"},
		{4, 	"product_mac"},
		{5, 	"mac_count"},
		{6, 	"product_name"},
		{7, 	"sw_name"},
		{8,	"vendor_name"},
		{9, 	"snmp_oid"},
		{10, 	"system_oid"},
		{11, 	"admin_username"},
		{12, 	"admin_password"},
	};
	arr_length = sizeof(elem_type_arr)/sizeof(elem_type_arr[0]);
	
	for (index = 0; index < arr_length; index++)
	{
		struct elem_type temp = elem_type_arr[index];
		if (0 == strcmp(elem_name, temp.name))
		{
			return temp.type_code;
		}
	}
	return -1;
}

//将这个两个数组生成一个sysinfo的buf内容 //注意提醒说。 这是要分配内存的。 需要注意释放。
char * gen_sysinfo_buf(char ** name_buf, char ** val_buf, int elem_count, int* pbuf_len)
{
	int buf_len = 0;
	int index = 0;
	
	char * sys_buf = NULL;
	int buf_index = 0;
	int elem_len = 0;
	int elem_type = 0;
	
	
	//now calculate the buf length
	buf_len += 10 ; // add the head length
	buf_len += 1;	// add '\n' char
	for (index = 0; index < elem_count; index++)
	{
		buf_len += 6;
		buf_len += strlen(val_buf[index]);	
		buf_len += 1;	// add '\n' char
	}
	DBG("sysinfo buf length is %d.\n", buf_len);
	
	sys_buf = (char * )malloc(buf_len);
	if (!sys_buf)
	{
		DBG("alloca memory error.\n");
		goto error;
	}
	
	memset(sys_buf, 0, buf_len);
	
	//Write Head
	
	sys_buf[buf_index++] = 0 + '0';
	sys_buf[buf_index++] = 0 + '0';
	
	if (-1 == ax_simple_tool_itonstr(elem_count, &sys_buf[buf_index], 3))
		goto error;		
	buf_index += 3;
	
	if (-1 == ax_simple_tool_itonstr(buf_len, &sys_buf[buf_index], 5))
		goto error;		
	buf_index += 5;		

	sys_buf[buf_index] = '\n';
	buf_index += 1;

	
	//Write text buf
	for (index = 0; index < elem_count; index++)
	{
		elem_type = tool_get_elemtype(name_buf[index]);
		if (elem_type < 0)
		{
			DBG("there is element type error.\n");
			return NULL;
		}
		elem_len = strlen(val_buf[index]);
		
		if (-1 == ax_simple_tool_itonstr(elem_type, &sys_buf[buf_index], 3))
			return NULL;		
		buf_index += 3;		
		
		if (-1 == ax_simple_tool_itonstr(elem_len, &sys_buf[buf_index], 3))
			return NULL;		
		buf_index += 3;		
		
		memcpy(&sys_buf[buf_index], val_buf[index], elem_len);
		buf_index += elem_len;		

		sys_buf[buf_index] = '\n';
		buf_index += 1;
	}
	
	if (buf_index != buf_len)
	{
		DBG("the text buf write error");
		goto error;
				
	}
	
	*pbuf_len = buf_len;
	
	return sys_buf;

error:
	if (sys_buf != NULL)
		free(sys_buf);
	sys_buf = NULL;
	return NULL;
	
}

//然后将这个sysinfo的buf内容写入一个文件中。
int write_file_buf(char * pathname, char *buf, int len)
{
	int fd;
	int wlen;
	
	fd = open(pathname, O_RDWR | O_CREAT);
	if (fd < 0)
	{
		DBG("open file %s error.\n", pathname);
		return -1;
	}
	wlen = write(fd, buf, len);
	if (wlen != len)
	{
		DBG("write buf space isn't enough.\n");
		return -1;
	}
	
	close(fd);
	
	return 0;

}


//#define _TEST_
/*
int main(int argc, char **argv)
{
	char * name_buf[20]; 
	char * val_buf[20];
	char  file_buf[200];
	char * sys_buf;
	int sys_buf_len;
	int ret;
	int elem_count;
	int rlen = 200;
	
	if (argv[1] == NULL)
	{
		printf(" ERROR argument.\n");
		return -1;
	}
	rlen = read_file_buf(argv[1], file_buf, &rlen);
	if (rlen < 0)
	{
		printf("read file count %d error.", rlen);
		return -1;
	}
	
	file_buf[rlen] = '\0';
	
	content_token(file_buf, name_buf, val_buf, &elem_count);
	
	sys_buf = gen_sysinfo_buf(name_buf, val_buf, elem_count, &sys_buf_len);
	
	if (sys_buf != NULL)
	{
		write_file_buf("sysinfo", sys_buf, sys_buf_len);
		free(sys_buf);			
	}
		
	return 0;

}
*/