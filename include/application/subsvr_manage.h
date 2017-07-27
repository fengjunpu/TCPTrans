#ifndef __SUBSVR_MANAGE_H__
#define __SUBSVR_MANAGE_H__

//#include "common.h"
#include "redis_define.h"
#include "redis_wrap.h"

#ifdef __cplusplus
extern "C" {
#endif

//返回值:  		>=0  成功，< 0  失败
int	start_subsvr_manage();

//返回值:  		>=0  成功，< 0  失败
int	stop_subsvr_manage();

#ifdef __cplusplus
}
#endif

#endif//__SUBSVR_MANAGE_H__

