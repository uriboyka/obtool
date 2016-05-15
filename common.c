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
 *trim    �ú�����Ҫ����ȥ���ַ������ߵĿհס�
 *@str    ������Ҫ������ַ�����
 *����ֵ  ����ȥ���ַ������߿հ׺��ָ�롣
 */
char *trim(char *str)
{
    int start;
    int end; 
    int i;
    
    for(start=0;isspace(str[start]);start++)
        ;/*����䣬������߿��ַ�����*/
    
    for(end = strlen(str)-1;isspace(str[end]);end--)
        ;/*����䣬�����ұ߿��ַ�����*/
    
    for(i=start;i<=end;i++)
    {
        str[i-start]=str[i];
    }
    
    /*���¹���õ��ַ�����ӽ�����*/
    str[end-start+1]='\0';
    
    return str;
}


/*
 *logger    �ú���������־�������
 *@level    ������־�ļ���
 *@msg      ��־���ݡ�
 *@...      �ɱ������
 */
void logger(LOGLEVEL level, const char *file, int line, const char *msg, ...)
{
    /*���ݱ������ȷ��Ҫ��ʾ����־����*/
    if(level < g_loglevel)
    {
        return;
    }

    /*��Ϣ����*/
    char log_buf[2048];

    /*�ɱ�������ֵĴ���*/
    va_list vl_fmt;
    va_start(vl_fmt, msg);
    vsprintf(log_buf, msg, vl_fmt);
    va_end(vl_fmt);

    /*��ȡϵͳʱ��*/
    time_t t = time(NULL);
    char *time = ctime(&t);
    time = trim(time);

    /*���ַ�������ʾö��ֵ*/
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

