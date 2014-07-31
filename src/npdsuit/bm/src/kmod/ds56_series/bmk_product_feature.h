#ifndef BMK_PRODUCT_FEATURE_H
#define BMK_PRODUCT_FEATURE_H

#include <linux/fs.h>
#include <linux/interrupt.h>

#include "npd/nbm/npd_cplddef.h"

#define LENGTH(x) (sizeof(x)/sizeof(x[0]))

#define MASTER_SLOT_MAX_NUM 2


#define CPLD_REG_BASE_ADDR 0x800000001D010000

#define POWER_NAME_LEN		20



/* ioctl struct define */
typedef int (* IOCTL_FUC) (/*struct inode *inode, */struct file *filp, unsigned int cmd,  unsigned long arg);
typedef struct _ioctl_proc_
{
	unsigned int 	cmd;
	IOCTL_FUC  		func;
}ioctl_proc;

typedef struct bm_cpld_args {
	unsigned int	func_code;	//CPLD func_code
	unsigned char	data;	//CPLD register value
}bm_cpld_args;


/* int struct define */
typedef struct _cpld_int_reg_param_
{
	unsigned int	int_addr;	//CPLD int register address
	unsigned int 	add_addr;	// cpld additioanal register address
} cpld_int_reg_param;

typedef struct _cpld_int_reg_data_
{
	unsigned char	int_data;	//cpld int register value
	unsigned char 	add_data; 	//cpld additional register value
} cpld_int_reg_data;

/* proc file */
typedef struct _proc_file_
{
	char * name;					//要创建的文件名
	struct file_operations* fops;		// 创建的文件结构
} proc_file;


typedef struct _cpld_reg_data_
{
	int 			func_code;
	unsigned char 	value;
}cpld_reg_data;

/* cpld reg ctrl define */
typedef struct _cpld_reg_ctrl_
{
	unsigned int 	cmd_code;
	int 			offset;
	int 			bytes;
	int 			mask;
	int				shift;
	void 			(*cpld_reg_ctrl_handler)(void*);
}cpld_reg_ctl;



typedef union
{
	uint8_t value;
	struct
	{
		uint8_t	ocp	: 1;
		uint8_t uvp	: 1;
		uint8_t ovp	: 1;
		uint8_t fan_ok	: 1;
		uint8_t ac_ok	: 1;
		uint8_t temp_ok	: 1;
		uint8_t sb3v3_ok :1; /* standby power */  
		uint8_t dv12v_ok	: 1; /* main power */
	}s;
}ds_state_t;	//ds_state

typedef struct _ps_state_data_
{
	ds_state_t 	work_state;
	int	state_dev_addr;
}ps_state_data;

typedef struct ds_fix_param_s
{
	int 	ds_index;
	int 	info_dev_addr;
	char 	name[POWER_NAME_LEN];
	
	ps_state_data * data;
	int (* get_state)(void *);
}ds_fix_param_t;


typedef struct ds_param_s
{
	int power_num;
	ds_fix_param_t * ds_fix_array;	
}ds_param_t;

typedef struct kfan_param_s
{
	int num;
}kfan_param_t;


typedef struct _kboard_fix_param_
{
	long board_code;
	long board_type;
	char* board_name;
	
	int (*board_init)(void);
	void (*board_cleanup)(void);

	int (*ioctl)(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg);
	ioctl_proc* ioctl_proc_arr;		/* ioctl proc array */
	int ioctl_proc_count;
    void (*cpld_interrupt_ctrl)(int value);
	irq_handler_t interrupt_handler;
	unsigned int irq_board ;
	cpld_int_reg_param*	cpld_int_reg_param_arr;		/* int reg */
	int cpld_int_reg_param_count;
	npd_int_content* cpld_int_data;
	
	unsigned long long cpld_reg_base_addr;		
	cpld_reg_ctl* cpld_reg_ctrl_arr;
	int cpld_reg_ctrl_count;
	void (*board_get_mngt_eth_mac)(int port, char * mac);

} kboard_fix_param;


typedef struct _kproduct_fix_param_
{
	long product_code;
	long product_type;
	
	char * product_short_name;
	char * product_name;

	unsigned short slotnum;
	unsigned short master_slotnum;
	unsigned short master_slot_id[MASTER_SLOT_MAX_NUM];
	
	unsigned char * proc_dir_name;
	struct proc_dir_entry *proc_dir_entry;
	
	proc_file * proc_common_files;
	int proc_common_count;
	proc_file * proc_board_spec_files;
	int proc_board_spec_count;

	int (*ioctl)(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg);
	ioctl_proc* ioctl_proc_arr;		/* ioctl proc array */
	int ioctl_proc_count;


	ds_param_t*	ds_param;
	kfan_param_t * fan_param;

	int (*product_param_init)(void);
	void (*product_param_cleanup)(void);
	int (*proc_files_init)(void);
	int (*proc_files_cleanup)(void);
	int (*get_board_online_state)(int );
	int (*reset_board)(int);
	int (*slot_index_get)(void);
	int (*online_slot_num)(void);
}kproduct_fix_param;



typedef struct _kfamily_fix_param_
{
	long family_code;
	long family_type;
	kproduct_fix_param ** kproduct_arr; 
	kboard_fix_param ** kboard_arr; 
}kfamily_fix_param;

typedef struct _kboard_param_
{
	long family_type;
	long product_type;
	long board_type;

	int	slot_index;	

	kboard_fix_param* fix_param; 
}kboard_param;


typedef struct _proc_files_struct_
{
	proc_file* array;
	int		   count;
} proc_file_struct;


typedef struct _port_isolation_slot_
{
	int insert;	/* insert = 0 slot is not online; insert = 1 slot is online */
	int slot;
	int port;
	int portAddr;
	unsigned int egress_port_bitmap;
} port_isolation_slot_t;

extern kfamily_fix_param* ko_family;
extern kproduct_fix_param* ko_product;
extern kboard_fix_param* ko_board;
extern kboard_param*  kboard_info;


#endif //BMK_PRODUCT_FEATURE_H
