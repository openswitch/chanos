#ifndef __NPD_ASD_H__
#define __NPD_ASD_H__

/* user or app part */
/* include header file end */

#define NPD_ASD_HASHTBL_NAME   "npdAsdHashTbl"
#define NPD_ASD_CFGTBL_NAME    "npdAsdCfgTbl"

#define NPD_ASD_HASH_IFIDX_SIZE   (128)
#define NPD_ASD_TABLE_SIZE        (4094)

#ifdef HAVE_CAPWAP_ENGINE
#define NPD_ASD_UFDB_HASHTBL_NAME   "npdAsdHashTbl"
#define NPD_ASD_UFDB_TABLE_SIZE   (512)
#define NPD_ASD_UFDB_HASH_SIZE   (32)
#endif //HAVE_CAPWAP_ENGINE

#endif
