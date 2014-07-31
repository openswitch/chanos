#ifndef __NPD_FDB_H__
#define __NPD_FDB_H__


#define AX_STATIC_FDB_ENTRY_SIZE (1024)

/*show running cfg mem*/
#define NPD_FDB_SHOWRUN_CFG_SIZE	(256*1024*2+1024*1024*2)  /* for maximum 1024 static FDB entry*/

#define NPD_FDB_HASHTBL_NAME   "npdFdbHashTbl"
#define NPD_FDB_CFGTBL_NAME    "npdFdbCfgTbl"

 

#endif
