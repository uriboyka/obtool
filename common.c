/*
 * =====================================================================================
 *
 *       Filename:  common.c
 *
 *    Description:  common function
 *
 *        Version:  1.0
 *        Created:  08/20/2015 11:44:57 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  syf
 *   Organization:  hs
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include "common.h"

LOGLEVEL    g_loglevel = LOG_DEBUG;
 

/*
 *trim    该函数主要用于去除字符串两边的空白。
 *@str    输入需要处理的字符串。
 *返回值  返回去掉字符串两边空白后的指针。
 */
char *trim(char *str)
{
    int start;
    int end; 
    int i;
    
    for(start=0;isspace(str[start]);start++)
        ;/*空语句，查找左边空字符长度*/
    
    for(end = strlen(str)-1;isspace(str[end]);end--)
        ;/*空语句，查找右边空字符长度*/
    
    for(i=start;i<=end;i++)
    {
        str[i-start]=str[i];
    }
    
    /*给新构造好的字符串添加结束符*/
    str[end-start+1]='\0';
    
    return str;
}


/*
 *logger    该函数用于日志的输出。
 *@level    设置日志的级别。
 *@msg      日志内容。
 *@...      可变参数。
 */
void logger(LOGLEVEL level, const char *file, int line, const char *msg, ...)
{
    /*根据编译宏来确定要显示的日志级别*/
    if(level < g_loglevel)
    {
        return;
    }

    /*消息缓存*/
    char log_buf[2048];

    /*可变参数部分的处理*/
    va_list vl_fmt;
    va_start(vl_fmt, msg);
    vsprintf(log_buf, msg, vl_fmt);
    va_end(vl_fmt);

    /*获取系统时间*/
    time_t t = time(NULL);
    char *time = ctime(&t);
    time = trim(time);

    /*用字符串来显示枚举值*/
    char *type = NULL;
    switch(level)
    {
        case LOG_DEBUG:
            type = "Debug";
            printf("[%s][%s][file:%s][line:%d]: %s", time, type, file, line, log_buf);
            break;
        case LOG_INFO:
            type = "Info";
            printf("[%s][%s]: %s", time, type, log_buf);
            break;
        case LOG_WARN:
            type = "Warn";
            printf("[%s][%s]: %s", time, type, log_buf);
            break;
        case LOG_ERROR:
            type = "Error";
            printf("[%s][%s]: %s", time, type, log_buf);
            break;         
        default:
            type = "Debug";
            printf("[%s][%s][file:%s][line:%d]: %s", time, type, file, line, log_buf);
            break;
    }

}

