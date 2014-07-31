#ifndef __NAM_FDB_H__
#define __NAM_FDB_H__



#define NAM_RV_REPLACE_OK(tmp_rv, rv, ok_rv) \
    if ((tmp_rv) < 0 && (tmp_rv) != (ok_rv)) (rv) = (tmp_rv)


#endif

