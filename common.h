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

#define NORMAL_EXIT   0x00     /*�����˳���*/
#define OPERAT_ERROR  0x01     /*����������*/
#define OPEN_ERROR    0x02     /*���ݿ��ʼ������*/
#define QUERY_ERROR   0x03     /*��ѯ������*/
#define QUERY_NONE    0x04     /*��ѯ���Ϊ��*/

typedef enum{            
    LOG_DEBUG = 0,      /*��־���� Debug */  
    LOG_INFO,           /*��־���� Info */  
    LOG_WARN,           /*��־���� Warn */  
    LOG_ERROR           /*��־���� Error */  
} LOGLEVEL; 

typedef enum{
    false = 0,
    true  = 1
}bool;

#include <ctype.h>
#include <string.h>

/*����ȫ�ֱ�����ʾ��־����*/
extern LOGLEVEL g_loglevel;


/*���ú���*/
char *trim(char *str);
void logger(LOGLEVEL level, const char *file, int line, const char *msg, ...);

#endif



