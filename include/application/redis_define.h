/*
 * ds_redis.h
 *
 *  Created on: 2015年10月23日
 *      Author: liwangmijia
 */

//参考数据库结构设计(公共部分)
#ifndef __REDIS_DEFINE_H__
#define __REDIS_DEFINE_H__

// Dssaccess相关信息
#define KEY_TPS_MAP                ("TPSMap")

#define KEY_TPS_KEY_PREFIX ("TPS_")

#define FIELD_TPS_IP                  ("ServerIP")
#define FIELD_TPS_PORT                ("ServerPort")
#define FIELD_TPS_AREA              ("ServerArea")
#define FIELD_TPS_VENDORNAME          ("VendorName")
#define FIELD_TPS_STATUS            ("Status")
#define FIELD_TPS_ACTIVEINDEX       ("ActiveIndex")
#define FIELD_TPS_RETOK			    ("RetOK")
#define FIELD_TPS_RUNSECONDS		("RunSeconds")
#define FIELD_TPS_RETERROR       ("RetError")

#define VALUE_TPS_OFFLINE              ("0")
#define VALUE_TPS_ONLINE              ("1")

#endif //__REDIS_WRAP_H__

