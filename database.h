/*
 * =====================================================================================
 *
 *       Filename:  database.h
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
#ifndef _DATABASE_H_
#define _DATABASE_H_


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mysql/mysql.h>


#define MAX_COL_LEN       1024           /*�������ݿ��ֶε��ַ�����󳤶�*/

typedef struct 
{
    int32_t nType;                       /*��ʶ�������ڴ洢������*/
    union
    {
        int        iInt;                  /*�洢Int�ͱ���*/
        double     dDouble;               /*�洢Double�ͱ���*/
        char       cChar[MAX_COL_LEN];    /*�洢�ַ����ͱ���*/
        MYSQL_TIME tTime;
    }UColumn;
}TColumn;

typedef struct Node{                     /*���������ڴ洢ɾ������ʱ��Ų�ѯ��������*/
    char value[10][50];                  /*�洢����ֵ�����10��������ÿ������ֵ��50*/
    struct Node *next;
}List;


int init_database(const char *filename);
int export_data(const char *config_file, const char *output_file);
int import_data(const char *config_file, const char *input_file);
int delete_table(const char *config_file, const char *condition_sql);
int release_database();
#endif
