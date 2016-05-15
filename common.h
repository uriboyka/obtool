/*
 * =====================================================================================
 *
 *       Filename:  common.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/20/2015 11:44:43 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  syf
 *   Organization:  hs
 *
 * =====================================================================================
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#define NORMAL_EXIT   0x00     /*正常退出码*/
#define OPERAT_ERROR  0x01     /*操作错误码*/
#define OPEN_ERROR    0x02     /*数据库初始错误码*/
#define QUERY_ERROR   0x03     /*查询错误码*/
#define QUERY_NONE    0x04     /*查询结果为空*/

typedef enum{            
    LOG_DEBUG = 0,      /*日志级别 Debug */  
    LOG_INFO,           /*日志级别 Info */  
    LOG_WARN,           /*日志级别 Warn */  
    LOG_ERROR           /*日志级别 Error */  
} LOGLEVEL; 

typedef enum{
    false = 0,
    true  = 1
}bool;

#include <ctype.h>
#include <string.h>

/*定义全局变量表示日志级别*/
extern LOGLEVEL g_loglevel;


/*公用函数*/
char *trim(char *str);
void logger(LOGLEVEL level, const char *file, int line, const char *msg, ...);

#endif



