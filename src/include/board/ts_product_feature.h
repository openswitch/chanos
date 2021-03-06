#ifndef TS_PRODUCT_FEATURE_H
#define TS_PRODUCT_FEATURE_H

/* Family hardware code */
#define PPAL_HWCODE_FAMILY_T9000	0x09
#define PPAL_HWCODE_FAMILY_AU3000	0xf3
#define PPAL_HWCODE_FAMILY_AS2000	0x06/*NEED TO BE CHECKED*/
#define PPAL_HWCODE_FAMILY_AS3000	0x07/*NEED TO BE CHECKED*/
#define PPAL_HWCODE_FAMILY_AS6600	0x08/*NEED TO BE CHECKED*/

#define PPAL_HWCODE_FAMILY_US3000	0xF4/*NEED TO BE CHECKED*/
#define PPAL_HWCODE_FAMILY_DS5600	0x08/*NEED TO BE CHECKED*/


/*family type code*/
#define FAMILY_UNKNOWN -1

enum
{
	FAMILY_TYPE_T9000,
	FAMILY_TYPE_AU3000,
	FAMILY_TYPE_AS3000,
	FAMILY_TYPE_AS6600,
	FAMILY_TYPE_US3000,	
	FAMILY_TYPE_AS2000,
	FAMILY_TYPE_DS5600,
	FAMILY_TYPE_MAX
};

/*product hardware code*/
#define PPAL_PRODUCT_HWCODE_AX8614  0x3
#define PPAL_PRODUCT_HWCODE_T9014  PPAL_PRODUCT_HWCODE_AX8614

#define PPAL_PRODUCT_HWCODE_AX8610  0x4
#define PPAL_PRODUCT_HWCODE_T9010  PPAL_PRODUCT_HWCODE_AX8610
#define PPAL_PRODUCT_HWCODE_AX8606  0x5
#define PPAL_PRODUCT_HWCODE_T9006  PPAL_PRODUCT_HWCODE_AX8606
#define PPAL_PRODUCT_HWCODE_G9606  0x8

#define PPAL_PRODUCT_HWCODE_T9003  0x02


#define PPAL_PRODUCT_HWCODE_AS2K_3200 0x01
#define PPAL_PRODUCT_HWCODE_NH3052 0x01
#define PPAL_PRODUCT_HWCODE_NH3028 0x01
#define PPAL_PRODUCT_HWCODE_AS6603 0x06 /*NEED TO BE CHECKED*/
#define PPAL_PRODUCT_HWCODE_AX8603 0x06 /*NEED TO BE CHECKED*/

#define PPAL_PRODUCT_HWCODE_US3000 0x07

#define PPAL_PRODUCT_HWCODE_DS5652 0x0A
#define PPAL_PRODUCT_HWCODE_DS6224 0x0B
#define PPAL_PRODUCT_HWCODE_DS5662 0x0C
#define PPAL_PRODUCT_HWCODE_DS6502 0x65


#define PPAL_PRODUCT_HWCODE_DUMMY	0xFF

/*product type*/
#define PRODUCT_UNKNOWN -1
enum
{
   	PRODUCT_DUMMY,			/*   0 	*/
    PRODUCT_T9010,	
    PRODUCT_T9006,
    PRODUCT_T9003,
    PRODUCT_NH_3052,
    PRODUCT_NH_3028,		/*   5 	*/
    PRODUCT_AS6603,
    PRODUCT_AS2K_3200,
    PRODUCT_AX8603,
    PRODUCT_US3000,
    PRODUCT_T9014,			/* 10 	*/
    PRODUCT_G9603,
    PRODUCT_G9606,
    PRODUCT_G9610,
    PRODUCT_DS5600,
    PRODUCT_DS6224,         /* 15 	*/
    PRODUCT_DS5662,
    PRODUCT_DS6502,
    PRODUCT_NH_MAX_NUM
};

/*board hardware code*/
#define PPAL_BOARD_HWCODE_TSM9002    0x00
#define PPAL_BOARD_HWCODE_TSM9024FC  0x01
#define PPAL_BOARD_HWCODE_TGM9048    0x02
#define PPAL_BOARD_HWCODE_TGM9024    0x03
#define PPAL_BOARD_HWCODE_TXM9004    0x04
#define PPAL_BOARD_HWCODE_CGM9048    0x05
#define PPAL_BOARD_HWCODE_ASX9604L	 0x10
#define PPAL_BOARD_HWCODE_FW9001     0x10
#define PPAL_BOARD_HWCODE_AS9612X	 0x03
#define PPAL_BOARD_HWCODE_AS6612C    0x10
#define PPAL_BOARD_HWCODE_TSM9002_LOONGSON    0x08
#define PPAL_BOARD_HWCODE_G9604X    0x09
#define PPAL_BOARD_HWCODE_FW9002     0x11
#define PPAL_BOARD_HWCODE_CGM9048S    0x15

/*AS6603*/
#define PPAL_BOARD_HWCODE_AX63GE48   0x03/*NEED TO BE CHECKED*/
#define PPAL_BOARD_HWCODE_AX63GE24   0x07/*NEED TO BE CHECKED*/
#define PPAL_BOARD_HWCODE_AX63GE24I  0x02/*NEED TO BE CHECKED*/
#define PPAL_BOARD_HWCODE_ASX6602    0x10/*NEED TO BE CHECKED*/

#define PPAL_BOARD_HWCODE_DUMMY		 0xFF


#define PPAL_HWCODE_NH_MCU     201
#define PPAL_HWCODE_NH_48GTX   202
#define PPAL_HWCODE_NH_3052    0x01
#define PPAL_HWCODE_NH_3028    0x03/*NEED TO BE CHECKED*/

/* AS2k */
#define PPAL_BOARD_HWCODE_AS2K_3200   0x04/*NEED TO BE CHECKED*/
#define PPAL_BOARD_HWCODE_AS2K_3209GC_PWR   0x05/*NEED TO BE CHECKED*/
#define PPAL_BOARD_HWCODE_AS2K_3228GC_PWR   0x06/*NEED TO BE CHECKED*/
#define PPAL_BOARD_HWCODE_AS2K_3224GC_PWR   0x07/*NEED TO BE CHECKED*/

/*  us3k */
#define PPAL_HWCODE_US_3000    0x07
#define PPAL_HWCODE_US_4628GS  0x08
#define PPAL_HWCODE_US_4629GX  0x09
#define PPAL_HWCODE_US_4629GX_PWRL  0x0A
#define PPAL_HWCODE_US_4629GX_PWR  0x0B
#define PPAL_HWCODE_US_SUB_4SFP  0x0C
#define PPAL_HWCODE_US_SUB_SFP_PLUS  0x0D


/*ds5600*/
#define PPAL_BOARD_HWCODE_DS5652    0x07
#define PPAL_BOARD_HWCODE_DS6224    0x08
#define PPAL_BOARD_HWCODE_DS5662    0x09

#define TSM9002_NAME     "TSM9002"
#define TSM9002_SN       "1000"

#define TSM9024FC_NAME   "TSM9024FC"
#define TSM9024FC_SN     ""

#define TGM9048_NAME   "TGM9048"
#define TGM9048_SN     ""

#define TGM9024_NAME   "TGM9024"
#define TGM9024_SN     ""

#define TXM9004_NAME   "TXM9004"
#define TXM9004_SN     ""

#define NH_3052_NAME   "AU3052"
#define NH_3052_SN     "1001"

#define FW9001_NAME    "fw9001"
#define FW9001_SN      ""

#define FW9002_NAME    "HFW2-FW9002"
#define FW9002_SN      ""

/*board type code*/
#define BOARD_UNKNOWN -1
/*新增单板类型，只能往后加，不能中间插入*/
enum
{
	PPAL_BOARD_TYPE_DUMMY = 0,
    PPAL_BOARD_TYPE_NH_NONE = 0,
    PPAL_BOARD_TYPE_TSM9002,
    PPAL_BOARD_TYPE_TSM9024FC,
    PPAL_BOARD_TYPE_TGM9048,
    PPAL_BOARD_TYPE_TGM9024,
    PPAL_BOARD_TYPE_TXM9004,      //----5
    PPAL_BOARD_TYPE_NH_3052,
    PPAL_BOARD_TYPE_NH_3028,
    PPAL_BOARD_TYPE_AX63GE48,
    PPAL_BOARD_TYPE_AX63GE24,
    PPAL_BOARD_TYPE_AX63GE24I,    //---10
	PPAL_BOARD_TYPE_CGM9048,
	PPAL_BOARD_TYPE_FW9001,
    PPAL_BOARD_TYPE_AS2K_3200,
    PPAL_BOARD_TYPE_AS2K_3209,
    PPAL_BOARD_TYPE_AS2K_3228,    //--15
    PPAL_BOARD_TYPE_US_3000, 
    PPAL_BOARD_TYPE_ASX6602,  
    PPAL_BOARD_TYPE_ASX9604L,
    PPAL_BOARD_TYPE_AS9612X,
    PPAL_BOARD_TYPE_US_4628GS,     //--20
    PPAL_BOARD_TYPE_US_4629GX,
    PPAL_BOARD_TYPE_US_4629GX_PWRL,
    PPAL_BOARD_TYPE_US_4629GX_PWR,
    PPAL_BOARD_TYPE_AS2K_3224P,
	PPAL_BOARD_TYPE_AS6612C,        //--25
	PPAL_BOARD_TYPE_US_SUB_SFP_PLUS,
	PPAL_BOARD_TYPE_US_SUB_4SFP,
	PPAL_BOARD_TYPE_G96SUP,
	PPAL_BOARD_TYPE_G9604X,
	PPAL_BOARD_TYPE_FW9002,			//--30
	PPAL_BOARD_TYPE_CGM9048S,
	PPAL_BOARD_TYPE_DS5652,
	PPAL_BOARD_TYPE_DS6224,
	PPAL_BOARD_TYPE_DS5662,
    PPAL_BOARD_TYPE_NH_MAX
};
#define TSERIES_PORT_PER_ASICMODULE 32
#endif

