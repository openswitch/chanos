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

	if (tcgetattr(fd, &orig_options) != 0) {  
		perror("tcgetattr");            
		exit(1);  
	}  
	memset(&cfsetispeed, 0, sizeof(struct termios));
	/* set Baud Rate 19200bps */
	cfsetispeed(&options, B19200);
	cfsetospeed(&options, B19200);
	/* set stop bit 1 */
	options.c_cflag &= ~(CSTOPB);
	/* set Parity is None */
	options.c_cflag &= ~PARENB;
	
	/* set flow control is none */
	options.c_cflag &= ~(CRTSCTS); 
	options.c_iflag &= ~(IXON|IXOFF);
	
	/* set options raw input */
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	if (tcsetattr(fd, TCSANOW, &options) != 0) {  
		perror("tcsetattr");        
		exit(1);      
	}
}

int main(int argc, char * argv[])
{
	int console_fd = -1;
	unsigned  char request_buf[12] = {0};
	unsigned  char response_buf[12] = {0};
	unsigned 
	int buf_len = 12;
	
	if (argc != 12)
	{
		perror("argument count is not right.");
		exit(1);		
	}
	printf("argc is %d.\n", argc);
	for (int i = 0; i < argc; i++)
	{
		request_buf[i] = (strtoul(argv[i], 16, NULL) & 0xFF);
		printf("reguest_buf[%d]=%d ", i, request_buf[i]);		
	}
	printf("\n");
	/* if frame end not give checksum need to   */
	if (strcmp(artv[argc-1], "##"))
	{
		int sum = 0;
		for (i = 0; i < 12; i++)
		{
			sum += request_buf[i];
		}
	}
	
	/* open RS-485 */
	if ((console_fd = open("/dev/ttyS1", O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0) {
		perror("open console");
		exit(1);
	}
	len = write(console_fd, request_buf, buf_len);
	if (len < 0 || len != buf_len){
		perror("write console error.");
		exit(1);		
	}
	len = read(console_fd, response_buf, buf_len);
	if (len != buf_len) {
		perror("read console error.");
		exit(1);			
	}
	for (int i = 0; i < 12; i++)
	{		
		printf("reguest_buf[%d]=%d ", i, request_buf[i]);		
	}	
	
	/* save old options */
	if (tcsetattr(fd, TCSANOW, &orig_options) != 0) {  
		perror("tcsetattr");        
		exit(1);      
	}	
	return 0;
}