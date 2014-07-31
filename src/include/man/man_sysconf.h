#ifndef _MAN_SYS_CONFIG_H_
#define _MAN_SYS_CONFIG_H_
#include <stdint.h>
#include <unistd.h>



typedef struct _board_type_list
{
    int slot_no;
    char board_type[32];
    struct _board_type_list *next;
}board_type_list;

typedef struct _hw_compat_list
{
    char image_type[32];
    board_type_list *board_type_head;
    struct _hw_compat_list *next;
}hw_compat_list;




#define MEMINFO_FILE "/var/run/meminfo.tmp"
#define SHOW_MEMORY_SCRIPT "sudo /usr/bin/show_memory.sh" 
#define SET_PWD_SCRIPT "sudo /usr/bin/set_pwd.sh"
#define SHOW_PWD_SCRIPT "sudo /usr/bin/show_pwd_info.sh"
#define PWD_INFO_FILE "/var/run/pwd_info.tmp"
#define MANUFACTURE_TEST_SH "sudo /usr/bin/manu_test.sh"

#define MAX_DNS_SERVER 3    

#define SYS_CONTACT_FILE "/var/run/sys_contact"
#define SYSTEM_DESC_FILE "/etc/sys_desc"
#define VTYSH_CLI_LOG_CONFIG_FILE "/var/run/cli_log.conf"
#define RIPD_SOCKET_BUF_FILE "/var/run/Ripd_socket_buf.conf"
#define RIPD_MAX_SOCKET_BUF "RIPD MAX SOCKET BUF:"

#define SYS_LOCATION_CONFIG_FILE "/var/run/sys_location"
#define SYS_LOCATION_STR_LEN 128
#define SYS_LOCATION_PREFIX  "SYSTEM LOCATION:"
#define NET_ELEMENT_CONFIG_FILE  "/var/run/net_elemnet"
#define NET_ELEMENT_STR_LEN 128
#define NET_ELEMENT_PREFIX  "NET ELEMENT:"
#define PAM_STATE_FILE "/etc/pam.tmp"
#ifndef PATH_MAX
#ifdef POSIX_PATH_MAX
#define PATH_MAX POSIX_PATH_MAX
#else
#define PATH_MAX 255 
#endif
#endif
#define IH_NMLEN		32	/* Image Name Length		*/
#define IH_MAGIC	0x27051956	
typedef struct image_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
//	uint32_t 	ih_ver;
} image_header_t;
//#define BM_IOC_MAGIC 0xEC
//#define BM_IOC_ENV_EXCH		_IOWR(BM_IOC_MAGIC,10,boot_env_t)
#define SAVE_BOOT_ENV 	1
#define GET_BOOT_ENV	0

#if 0
typedef struct boot_env
{	
	char name[64];	
	char value[128];	
	int operation;
}boot_env_t;
#endif
#define BM_IOC_BOOTROM_EXCH    _IOWR(BM_IOC_MAGIC, 17,bootrom_file)
typedef struct bootrom_file
{
	char path_name[PATH_MAX];
}bootrom_file;
#define IMG_NAME_FILE "/var/run/img_name.tmp"



unsigned long get_file_size(const char *filename);
int get_boot_version_name(char* vername);
int get_boot_img_name(char* imgname);
int set_boot_img_name(char* imgname);
int check_img(char* filename);
int write_bootrom(char *path);
int read_hosts_file_item(char *item,char *service,char *addr);
int get_pam_status(char *rad_state,char *tac_state);
int get_offset_time();
int set_offset_time(int offset_time);
int set_sys_location(char *location);
char* get_net_element(char *net_element);
int set_net_element(char *net_element);
int get_sys_desc(char *desc);
char* get_sys_location(char *location);
 
int set_cli_syslog_str(int num);
int get_dns_str(char **name);
int set_dns_str(const char **name,int num);
int set_ripd_socket_buffer(unsigned int entry);
int get_next_login_permit(int number,char *filter);
int get_next_login_deny(int number,char *filter);
int get_next_radius_server(int number,char *addr);
int get_next_tacplus_server(int number,char *addr);
int get_dns_server(int number,char *addr,char **name,int count);
int set_sys_desc(char *desc);
int get_idle_time(char *idle_str);
int set_idle_timeout(int minutes);
int get_ripd_socket_buffer(char *rip_buffer);
int get_next_image(int *number,char *img_name,unsigned long *file_size, unsigned long *create_time);
void get_sys_contact(char *contact);
int set_sys_contact(char *contact);
int get_system_time(char *sys_time);
int get_system_uptime(char *uptime_str);
int delete_dns_server(char *addr);
int get_cli_syslog_str(void);
int download_image_by_url(char *url,char *usr,char *pwd,char *err_msg);
int reboot_system(char *para);

  int sync_sys_description(char *sys_desc);
 int sync_sys_location(char *sys_location);
  int sync_sys_contact(char *sys_contact);
 int sync_login_timeout(char *time);
 
  int sync_add_dns(char *dns_server);
  int sync_delete_dns(char *dns_server);
 
 
int get_mem_info(int *total,int *available);
int sync_pwd_aging_day(char *day);
int sync_pwd_retry(char *times);
board_type_list *init_running_board_type_list();
int release_board_type_list(board_type_list *head);
int release_hw_compat_list(hw_compat_list *head);
int show_hw_compat_list(hw_compat_list *head);
void show_running_board_type(board_type_list *bth);
hw_compat_list *man_parse_hw_compat_list(board_type_list *bth);
int check_image_is_compatiple(char *file_name,hw_compat_list *head,board_type_list *bth);
int send_image_by_board_type(char *file_name,hw_compat_list *head,board_type_list *bth);
int  get_image_type(char *filename,char *type);
#endif
