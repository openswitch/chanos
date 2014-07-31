
#ifndef __PACK_TAIL_H__
#define __PACK_TAIL_H__

#define BLOCK_SIZE (64*1024)
#define TAIL_TMP "/mnt/tailtmp"
#define IMAGE_TMP "/mnt/imgtmp"
#define IMG_NO_OPERATION 2
#define PACK_TAIL_SCRIPT "./pack_tail.sh"
#define AUTEWARETAIL "AWFS.d1@rdwH3%*$"
#define HANDLE_IMG_SCRIPT "sudo /usr/bin/handle_image.sh"

#define HW_COMPAT_FILE "/etc/hw_compat"

typedef struct _auteware_img_pack_tail_
{
	char magic[16];
	char type[32]; 
	char version[16]; 
    unsigned int time; 
	char filename[64]; 
	unsigned int length; 
	unsigned int checksum; 
}auteware_img_pack_tail;

typedef struct _board_type_s
{
    char board_type[32];
    char image_type[16];
    struct _board_type_s *next;
}board_type_t;

typedef struct _auteware_tail_s
{
    auteware_img_pack_tail tail_entry;
    struct _auteware_tail_s *next;       
}auteware_tail_t;






#endif 
