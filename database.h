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


#define MAX_COL_LEN       1024           /*保存数据库字段的字符串最大长度*/

typedef struct 
{
    int32_t nType;                       /*标识联合体内存储的类型*/
    union
    {
        int        iInt;                  /*存储Int型变量*/
        double     dDouble;               /*存储Double型变量*/
        char       cChar[MAX_COL_LEN];    /*存储字符串型变量*/
        MYSQL_TIME tTime;
    }UColumn;
}TColumn;

typedef struct Node{                     /*单链表用于存储删除数据时存放查询出的主键*/
    char value[10][50];                  /*存储主键值，最多10个主键，每个键的值长50*/
    struct Node *next;
}List;


int init_database(const char *filename);
int export_data(const char *config_file, const char *output_file);
int import_data(const char *config_file, const char *input_file);
int delete_table(const char *config_file, const char *condition_sql);
int release_database();
#endif
