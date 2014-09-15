/*
* Copyright (c) 2008,Autelan - AuteCS
* All rights reserved.
*
*$Source: /rdoc/AuteCS/cgic205/snmp_agent/ws_snmpd_engine.h,v $
*$Author: qiaojie $
*$Date: 2009/06/16 08:57:24 $
*$Revision: 1.14 $
*$State: Exp $
*
*$Log: ws_snmpd_engine.h,v $
*Revision 1.14  2009/06/16 08:57:24  qiaojie
*消除编译警告
*
*Revision 1.13  2009/06/01 06:43:19  tangsiqi
*no message
*
*Revision 1.12  2009/05/19 10:01:04  tangsiqi
*修改OID生成规则，初始化SNMP出厂配置
*
*Revision 1.11  2009/05/11 10:43:51  tangsiqi
*no message
*
*Revision 1.10  2009/05/07 11:14:44  tangsiqi
*for trap mib
*
*Revision 1.9  2009/05/07 09:57:52  tangsiqi
*no message
*
*Revision 1.8  2009/04/29 09:12:35  tangsiqi
*no message
*
*Revision 1.7  2009/04/09 07:32:44  tangsiqi
*完成对OEM的OID兼容
*
*Revision 1.6  2009/03/23 06:36:06  shaojunwu
*添加release函数，与init函数对应
*
*Revision 1.5  2009/01/05 10:42:52  tangsiqi
*for sysname in miblib
*
*Revision 1.4  2009/01/05 10:02:41  tangsiqi
*no message
*
*Revision 1.3  2008/12/30 09:59:07  tangsiqi
*no message
*
*Revision 1.2  2008/12/29 06:02:27  tangsiqi
*snmp基本功能版
*
*Revision 1.1  2008/12/16 02:32:41  tangsiqi
*snmp module
*
*
*/

#ifndef _MAN_SNMPD_ENGINE_H
#define _MAN_SNMPD_ENGINE_H

#include "ws/ws_snmpd_engine.h"
#include "ws/ws_sysinfo.h"
#include "ws/ws_usrinfo.h"
#include "ws/ws_public.h"
#include "ws/ws_conf_engine.h"
#include "ws/ws_log_conf.h"
#include "ws/ws_ec.h"


#define DISABLE_STATUS 0
#define ENABLE_STATUS  1

#endif

