#ifndef BM_CPLD_UTIL_H
#define BM_CPLD_UTIL_H

extern int debug_flag;

#undef DBG
#define DBG(f,x...) \
	if (debug_flag) { printf("DEBUG " f,##x); }


int read_file_buf (char *pathname, char *buf, int * plen);

int content_token(char *buf, char** name_buf, char ** val_buf, int * pcount);

int ax_simple_tool_itonstr(int count, char* char_addr, int char_num);

int tool_get_elemtype(char * elem_name);

char * gen_sysinfo_buf(char ** name_buf, char ** val_buf, int elem_count, int* pbuf_len);

int write_file_buf(char * pathname, char *buf, int len);



#endif 