#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include  <sys/ioctl.h>
#include <linux/sockios.h>


int non_block=1;

int set_nonblock(int fd)
{
	int flags = 0;
	if ((flags = fcntl(fd, F_GETFL)) < 0)
	{
	
		return -1;
	}
	if (fcntl(fd, F_SETFL, (flags | O_NONBLOCK)) < 0)
	{
		return -1;
	}
	printf("set fd %d non block.\n");		
}



int get_sock_buf(int sockfd)
{
	int ret;
	int send_size = 0;
	int send_len = sizeof(int);
	int recv_size = 0;
	int recv_len = sizeof(int);	
	
	ret = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &send_size, &send_len);
	if (ret != 0)
	{
		printf("client: get sockfd %d send buf error.\n", sockfd);
		return 0;	
	}
	
	ret = getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recv_size, &recv_len);
	if (ret != 0)
	{
		printf("client: get sockfd %d recv buf error.\n", sockfd);
		return 0;	
	}	
	
	printf("client: get sockfd %d send buf size %d, recv buf size %d.\n", 
		sockfd, send_size, recv_size);
}

int get_sock_outq(int sockfd)
{
	int ret;
	int outq;
	
	ret = ioctl(sockfd, SIOCOUTQ, &outq);
	if (ret != 0)
	{
		printf("get sock fd %d outq error.\n");
		return ret;	
	}
	printf("client sockfd %d outq is %d.\n", sockfd, outq);
}

int set_sock_buf(int sockfd)
{
	int ret;
	unsigned long send_size = 106496 * 50;
	int send_len = sizeof(unsigned long);
	unsigned long recv_size = 106496 * 50 ;
	int recv_len = sizeof(unsigned long);	
	
	ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &send_size, send_len);
	if (ret != 0)
	{
		printf("client: set sockfd %d send buf error.\n", sockfd);
		return 0;	
	}
	
	ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recv_size, recv_len);
	if (ret != 0)
	{
		printf("client: set sockfd %d recv buf error.\n", sockfd);
		return 0;	
	}	
	
	printf("client: set sockfd %d send buf size %u, recv buf size %u.\n", 
		sockfd, send_size, recv_size);
}



int client(void)
{
	 int socket_fd;
	 struct sockaddr_un server_address; 
	 struct sockaddr_un client_address; 
	 int bytes_received, bytes_sent, integer_buffer;
	 socklen_t address_length;
	 
	 struct timeval tv_start, tv_end;
	 struct timezone tz;
	 int count = 0;
#if DATAGRAM	
	 if((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
	 {
	  perror("client: socket");
	  return 1;
	 }
#else
	 if((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	 {
	  perror("client: socket");
	  return 1;
	 }
#endif
	
	if (non_block)
		set_nonblock(socket_fd);
	       
	 memset(&client_address, 0, sizeof(struct sockaddr_un));
	 client_address.sun_family = AF_UNIX;
	 strcpy(client_address.sun_path, "/tmp/UDSDGCLNT");
	
	 unlink("/tmp/UDSDGCLNT");
	 
#if DATAGRAM	 
	 if(bind(socket_fd, (const struct sockaddr *) &client_address, 
	         sizeof(struct sockaddr_un)) < 0)
	 {
	  perror("client: bind");
	  return 1;
	 }
#endif
	 memset(&server_address, 0, sizeof(struct sockaddr_un));
	 server_address.sun_family = AF_UNIX;
	 strcpy(server_address.sun_path, "/tmp/UDSDGSRV");
	 

#if DATAGRAM

#else
	int ret = 0;
	
	
	ret = connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address));
	if (ret != 0)
	{
		printf("%s: connect  server error.\n", __FUNCTION__, count);
	}
#endif
	
	 integer_buffer = 5;
	
	 int byte_has_sent = 0;
	 
	 get_sock_buf(socket_fd);
	 set_sock_buf(socket_fd);
	 get_sock_buf(socket_fd);
	
	
		char buffer[40] = {0};
		int buffer_len = 40;
		
	 while (1)
	 {	 	
#if  DATAGRAM 	 	
		 bytes_sent = sendto(socket_fd, (char *) &integer_buffer, sizeof(int), 0,
		                     (struct sockaddr *) &server_address, 
		                     sizeof(struct sockaddr_un));
#else
		
retry:		
		get_sock_outq(socket_fd);	
		
		bytes_sent = sendto(socket_fd, buffer, buffer_len, 0, 0, 0);
//		bytes_sent = sendto(socket_fd, (char *) &integer_buffer, sizeof(int), 0,
//		                     NULL, 
//		                     NULL);
		                     
		if (bytes_sent <= 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				printf("%s: bytes_send %d, errno is %d.\n", 
						__FUNCTION__, bytes_sent, errno);
				sleep(3);
				printf("%s: client has sleep 3 second.\n");
				goto retry;		 		
			}
			else
			{
				printf("%s: send error. go out.\n",__FUNCTION__);
				return -1;
			}
				
		}
		else
		{
			byte_has_sent += bytes_sent;	
		}
                     
				                     
				                     
#endif		 		                   
		
		 address_length = sizeof(struct sockaddr_un);
		 count++;
		 printf("%s: send the %d times.\n", __FUNCTION__, count);
		 printf("%s: send bytes %d. totol send %d.\n", __FUNCTION__, bytes_sent, byte_has_sent);
		 gettimeofday(&tv_start, &tz);
#if DATAGRAM		 
		 bytes_received = recvfrom(socket_fd, (char *) &integer_buffer, sizeof(int), 0, 
		                           (struct sockaddr *) &(server_address),
		                           &address_length);
#else
		 bytes_received = recvfrom(socket_fd, (char *) &integer_buffer, sizeof(int), 0, 
		                           NULL,
		                           NULL);
#endif		
		 gettimeofday(&tv_end, &tz);
		 printf("%s: receive the time is %d.\n", 
		 	__FUNCTION__, (tv_end.tv_sec - tv_start.tv_sec));
		
		if (bytes_received < 0 )
		{
			if (errno == EAGAIN)
			{
				printf("bytes_received is error. need try again.\n");
				goto retry;	
			}	
		}
		
		 if(bytes_received != sizeof(int))
		 {
		  printf("wrong size datagram\n");
		  //close(socket_fd);
		  //return 1;
		 }
		
		 printf("%s: integer buffer is %d\n", __FUNCTION__, integer_buffer);	 	
	 }
	 close(socket_fd);
	
	 return 0;
}

int main(int argc, char** argv)
{

	if (!strcmp(argv[1], "b"))
	{
		non_block = 0;
	}	

	client();	

	return 0;
}