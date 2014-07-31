#ifndef __NPD_DIAG_H__
#define __NPD_DIAG_H__

typedef struct unimac_data_s {
	int rw;
	int type;
	int unit;
	int port;
	unsigned int data;
}unimac_data_t;

typedef enum{
	COOMAND_CONFIG,
	GPORT_CONFIG,
	MAC_0,
	MAC_1,
	FRM_LENGTH,
	PAUSE_QUNAT,
	SFD_OFFSET,
	MAC_MODE,
	TAG_0,
	TAG_1,
	TX_IPG_LENGTH,
	PAUSE_CONTROL,
	IPG_HD_BKP_CNTL,
	FLUSH_CONTROL,
	RXFIFO_STAT,
	TXFIFO_STAT,
	GPORT_RSV_MASK,
	GPORT_STAT_UPDATE_MASK,
	GPORT_TPID,
	GPORT_SOP_S1,
	GPORT_SOP_S0,
	GPORT_SOP_S3,
	GPORT_SOP_S4,
	GPORT_MAC_CRS_SEL
}unimac_type;

DBusMessage * npd_dbus_diagnosis_pci_hw_rw_reg(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_diagnosis_cpld_hw_rw_reg(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_diagnosis_env_monitor_control(DBusConnection *conn, DBusMessage *msg, void *user_data) ;

/*
  **********************************
  *	extern function
  **********************************
  */
/**********************************************************************************
 *	nbm_eeprom_reg_read
 * 
 *  DESCRIPTION:
 *		 read eeprom register
 *
 *  INPUT:
 *		unsigned char twsi_channel,		- TWSI channel
 *		unsigned int eeprom_addr,		- eeprom address
 *		unsigned int eeprom_type,		- eeprom type
 *		unsigned int validOffset,			- whether the slave has offset (i.e. Eeprom  etc.), true: valid false: in valid
 *		unsigned int moreThan256,		- whether the ofset is bigger than 256, true: valid false: in valid
 *		unsigned int regAddr				- address of eeprom's register
 *  
 *  OUTPUT:
 *		unsigned char *regValue			- value of eeprom's register
 *
 *  RETURN:
 * 		NBM_ERR		- set fail
 *	 	NBM_OK		- set ok
 *
 **********************************************************************************/
extern unsigned int nbm_eeprom_reg_read
(
	unsigned char twsi_channel,
	unsigned int eeprom_addr,
	unsigned int eeprom_type,
	unsigned int validOffset,
	unsigned int moreThan256,
	unsigned int regAddr,
	unsigned char *regValue
);

/**********************************************************************************
 *	nbm_eeprom_reg_write
 * 
 *  DESCRIPTION:
 *		write eeprom register
 *
 *  INPUT:
 *		unsigned char twsi_channel,		- TWSI channel
 *		unsigned int eeprom_addr,		- eeprom address
 *		unsigned int eeprom_type,		- eeprom type
 *		unsigned int validOffset,			- whether the slave has offset (i.e. Eeprom  etc.), true: valid false: in valid
 *		unsigned int moreThan256,		- whether the ofset is bigger than 256, true: valid false: in valid
 *		unsigned int regAddr				- address of eeprom's register
 *		unsigned char *regValue			- value of eeprom's register
 *  
 *  OUTPUT:
 *		NULL
 *
 *  RETURN:
 * 		NBM_ERR		- set fail
 *	 	NBM_OK		- set ok
 *
 **********************************************************************************/
extern unsigned int nbm_eeprom_reg_write
(
	unsigned char twsi_channel,
	unsigned int eeprom_addr,
	unsigned int eeprom_type,
	unsigned int validOffset,
	unsigned int moreThan256,
	unsigned int regAddr,
	unsigned char regValue
);

/**********************************************************************************
 * nbm_cpld_reg_write
 *
 * General purpose API to write a CPLD register
 *
 *	INPUT:
 *		addr	- CPLD register address
 *		value 	- CPLD register value
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0 - if no error occur.
 *		DIAG_RETURN_CODE_ERROR - if error occurred.
 *
 **********************************************************************************/
extern int nbm_cpld_reg_write(int addr, unsigned char value);

/**********************************************************************************
 * nbm_cpld_reg_read
 *
 * General purpose API to read a CPLD register
 *
 *	INPUT:
 *		addr	- CPLD register address
 *	
 *	OUTPUT:
 *		value	- CPLD register value
 *
 *	RETURN:
 *		0 - if no error occur.
 *		DIAG_RETURN_CODE_ERROR - if error occurred.
 *
 **********************************************************************************/
extern int nbm_cpld_reg_read(int reg, unsigned char *value);

/**********************************************************************************
 * nbm_hardware_watchdog_control
 *
 * 	Set hardware watchdog enable or disable
 *
 *	INPUT:
 *		enabled	- enable/disable hardware watchdog
 *	
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		0 - if no error occur.
 *		DIAG_RETURN_CODE_ERROR - if error occurred.
 *
 **********************************************************************************/
extern int nbm_hardware_watchdog_control_set
(
	unsigned int enabled
);

/**********************************************************************************
 * nbm_hardware_watchdog_control_get
 *
 * 	Get hardware watchdog enable or disable
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		enabled	- enable/disable hardware watchdog
 *
 *	RETURN:
 *		0 - if no error occur.
 *		DIAG_RETURN_CODE_ERROR - if error occurred.
 *
 **********************************************************************************/
extern int nbm_hardware_watchdog_control_get
(
	unsigned int *enabled
);

/**********************************************************************************
 * nbm_hardware_watchdog_timeout_set
 *
 * 	Set hardware watchdog timeout value
 *
 *	INPUT:
 *		timeout	- hardware watchdog timeout value
 *	
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		0 - if no error occur.
 *		DIAG_RETURN_CODE_ERROR - if error occurred.
 *
 **********************************************************************************/
extern int nbm_hardware_watchdog_timeout_set
(
	unsigned int timeout
);

/**********************************************************************************
 * nbm_hardware_watchdog_timeout_get
 *
 * 	Get hardware watchdog timeout value
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		timeout	- hardware watchdog timeout value
 *
 *	RETURN:
 *		0 - if no error occur.
 *		DIAG_RETURN_CODE_ERROR - if error occurred.
 *
 **********************************************************************************/
extern int nbm_hardware_watchdog_timeout_get
(
	unsigned int *timeout
);
#endif
