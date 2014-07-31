#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

struct termios orig_options;

/* configure serial */
static void config_serial(int fd)
{
	struct termios options; 
	printf("config serial %d.\n", fd);
	if (tcgetattr(fd, &orig_options) != 0) {  
		perror("tcgetattr");            
		exit(1);  
	}  
	/*
	if (tcgetattr(fd, &options) != 0) {  
		perror("tcgetattr");            
		exit(1);  
	} 	
	*/
	memset(&options , 0, sizeof(struct termios)); 
	/* set Baud Rate 19200bps */
	cfsetispeed(&options, B19200);
	cfsetospeed(&options, B19200);
	
	/* set stop bit 1 */
	options.c_cflag &= ~(CSTOPB);
	/* set Parity is None */
	options.c_cflag &= ~(PARENB);
	options.c_iflag &= ~(INPCK);
	/* set data bits */	 
	options.c_cflag &= ~(CSIZE);
	options.c_cflag |= ~(CS8); 
	
	/* set flow control is none */
	options.c_cflag &= ~(CRTSCTS); 
	options.c_iflag &= ~(IXON|IXOFF|IXANY); 
	
	/* set options raw input */
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /* Input */
	options.c_oflag &= ~(OPOST);
	
	tcflush(fd,TCIOFLUSH); 
	if (tcsetattr(fd, TCSANOW, &options) != 0) {  
		perror("tcsetattr");        
		exit(1);      
	}
}

int set_com_config(int fd, int baud_rate, 
		int data_bits, char parity, int stop_bits)
{
	struct termios new_cfg, old_cfg;
	int speed;
	
	/* 保存并调试现有串口参数设置， 在这里如果串口号等出错，
	会有相关的出错信息 */
	if (tcgetattr(fd, &old_cfg) != 0)
	{
		perror("tcgetattr");
		return -1;	
	}
	
	/*  设置字符大小 */
	memcpy(&new_cfg, &old_cfg, sizeof(struct termios));
	cfmakeraw(&new_cfg); /* 配置为原始模式 */
	new_cfg.c_cflag &= ~CSIZE;
	
	/* 设置波特率 */
	switch(baud_rate)
	{
		case 2400:
			speed = B2400;
			break;
		case 4800:
			speed = B4800;
			break;
		case 9600:
			speed = B9600;
			break;
		case 19200:
			speed = B19200;	
			break;		
		case 38400:
			speed = B38400;	
			break;
		case 115200:
		default:
			speed = B115200;
			break;				
	}
	cfsetispeed(&new_cfg, speed);
	cfsetospeed(&new_cfg, speed);
	
	/* 设置停止位 */
	switch (data_bits)
	{
		case 7:
			new_cfg.c_cflag |= CS7;
			break;
		case 8:
		default:
			new_cfg.c_cflag |= CS8;
			break;
	}
	
	/* 设置奇偶校验位 */
	switch (parity)
	{

		case 'o':
		case 'O':
			new_cfg.c_cflag |= (PARODD | PARENB);
			new_cfg.c_iflag |= INPCK;
			break;	
			
		case 'e':
		case 'E':
			new_cfg.c_cflag |= PARENB;
			new_cfg.c_cflag &= ~PARODD;
			new_cfg.c_iflag |= INPCK;
			break;
		
		case 's': /* as no parity */
		case 'S': 
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_cflag &= ~CSTOPB;
			break;

		case 'n':
		case 'N':
		default:			
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_iflag &= ~INPCK;
			break;
					
	}
	
	/* 设置停止位 */
	switch (stop_bits)
	{
		case 2:
			new_cfg.c_cflag |= CSTOPB;
			break;
		case 1:
		default:
			new_cfg.c_cflag &= ~CSTOPB;
			break;	
	}
	
	/* 设置等待时间和最小接受字符 */
	new_cfg.c_cc[VTIME] = 1;
	new_cfg.c_cc[VMIN] = 0;
	
	/* 处理未接受字符 */
	tcflush(fd, TCIFLUSH);
	/* 激活新配置 */
	if ((tcsetattr(fd, TCSANOW, &new_cfg)) != 0)
	{
		perror("tcsetattr");
		return -1;	
	}
	return 0;
}

int main(int argc, char * argv[])
{
	int console_fd = -1;
	unsigned  char request_buf[12] = {0};
	unsigned  char response_buf[12] = {0};
	unsigned 
	int buf_len = 12;
	int i = 0;
	int len=0;
	char* filename ;
	
	
	if ((argc == 14) && (argv[13] != NULL))
	{
		filename = argv[13];
	}
	else
	{
		filename = "/dev/ttyS1";
	}
	
	
	printf("read console file is %s.\n", filename);
	/* printf("argc is %d.\n", argc); */
	/* translate input string to number  */
	for (i = 1; i < 12; i++)
	{
		request_buf[i-1] = (strtoul(argv[i], NULL, 16) & 0xFF);
		/* printf("0x%02X ", request_buf[i-1]);	 */
	}

	/* if frame end not give checksum need to   */
	int sum = 0;
	for (i = 0; i < 11; i++)
	{
		sum += request_buf[i];
	}
	request_buf[11]=(sum & 0xFF);
	/* printf("check sum is 0x%02X.\n", request_buf[11]); */
	
	/* open RS-485 */	
	if ((console_fd = open(filename, O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
		perror("open console");
		exit(1);
	}
	if (fcntl(console_fd, F_SETFL, 0) < 0) {
		perror("fcntrl F_SETFL\n");
	}
	
	/* config_serial(console_fd); */
	if (set_com_config(console_fd, 19200, 8, 'N', 1) < 0)
	{
		perror("set_com_config.\n");
		exit(1);			
	}
	
	printf("Resquest Command buffer:\n");
	printf("==============================");
	printf("==============================\n");
	for (i = 0; i < 12; i++)
	{
		if (i == 0)
		{
			int j = 0;
			for (j = 0; j < 12; j++)
				printf(" %-4d", j);
			printf("\n");	
			printf("------------------------------");
			printf("------------------------------\n");			
		}

		printf("0x%02X ", request_buf[i]);
	}	
	printf("\n");
	printf("------------------------------");
	printf("------------------------------\n");	
	
	/* printf("write request buf.\n" ); */
	len = write(console_fd, request_buf, buf_len);
	if (len < 0 || len != buf_len){
		perror("write console error.");
		exit(1);		
	}
	/* sleep(2); */
	/* usleep(200); */ 
	len = 0;
	len = read(console_fd, response_buf, buf_len);
	printf("read len is %d.\n", len);
	if (len < 0) {
		tcsetattr(console_fd, TCSANOW, &orig_options);
		perror("read console error.");
		exit(1);			
	}
	sum = 0;
	for (i = 0; i < 11; i++)
	{
		sum += 	response_buf[i];	
	}
	sum &= 0xFF;
	
	printf("0x%x and data is %s\n", 
		sum, (sum ==response_buf[11] ? "right" : "error"));
	printf("Response Command buffer:\n");
	printf("==============================");
	printf("==============================\n");
	for (i = 0; i < 12; i++)
	{
		if (i == 0)
		{
			int j = 0;
			for (j = 0; j < 12; j++)
				printf(" %-4d", j);
			printf("\n");	
			printf("------------------------------");
			printf("------------------------------\n");			
		}
		/*
		if (i == 6)
		{
			printf("\n");
			int j = 0;
			for (j = 6; j < 12; j++)
				printf(" %-4d", j);	
			printf("\n");		
			printf("------------------------------\n");
		}	
		*/		
	
		printf("0x%02X ", response_buf[i]);
	}	
	printf("\n");
	printf("------------------------------");
	printf("------------------------------\n");
	/* save old options */
	/* 
	if (tcsetattr(console_fd, TCSANOW, &orig_options) != 0) {  
		perror("tcsetattr");        
		exit(1);      
	}	
	*/
	return 0;
}
