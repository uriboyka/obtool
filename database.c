/*
 * =====================================================================================
 *
 *       Filename:  database.c
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
 #include "database.h"
 #include "common.h"
 #include <sys/time.h>

MYSQL *mysql = NULL;


/*
 *init_database    函数用于初始化mysql库,和连接到数据库。
 *返回值           成功返回零，失败返回非零。
 */
int init_database(const char *filename)
{
    char    host[50] ;
    char    username[50];
    char    password[50];
    int     port  = 2880;
    
    FILE   *fp    = NULL;
    char   *line  = NULL;
    size_t  len   = 0;

    fp = fopen(filename,"r");
    if(fp == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "打开连接数据库配置文件失败!\n");
        return OPEN_ERROR;
    }

    /*读取连接数据库配置文件*/
    while(getline(&line,&len,fp) != -1)
    {
        line = trim(line);
        char *category = strtok(line, "=");
        if(!strcmp("HOST", category))
        {
            strcpy(host, strtok(NULL, "="));
        }
        else if(!strcmp("USERNAME", category))
        {
            strcpy(username, strtok(NULL, "="));
        }
        else if(!strcmp("PASSWORD", category))
        {
            strcpy(password, strtok(NULL, "="));
        }
        else if(!strcmp("PORT", category))
        {
            port = atoi(strtok(NULL, "="));
        }
    }

    if(0 != mysql_library_init(0, NULL, NULL))
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "加载mysql库失败!\n");
        fclose(fp);
        return OPEN_ERROR;
    }
    
    mysql = mysql_init(NULL);
    if(mysql == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "初始化mysql失败!\n");
        fclose(fp);
        mysql_library_end();
        return OPEN_ERROR;
    }

    if(!mysql_real_connect(mysql, host,username, password, "test", port, NULL, 0))
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "连接数据库失败，错误原因:%s\n", mysql_error(mysql));
        fclose(fp);
        mysql_close(mysql);
        mysql_library_end();
        return OPEN_ERROR;
    }

    /*设置连接库手动提交*/
    mysql_autocommit(mysql, 0);
    
    fclose(fp);
    return NORMAL_EXIT;
}



/*
 *export_data      函数用于导出表中数据到txt文档。
 *@filename        需要导出的文件名
 *返回值           成功返回零，失败返回非零。
 */
int export_data(const char *config_file, const char *output_file)
{ 
    FILE   *in        = NULL;
    FILE   *out       = NULL;
    char   *line      = NULL;
    size_t  len       = 0;
    char    sql[2048]  = "select ";
    char    table[50];

    /*解析配置文件*/
    in = fopen(config_file, "r");
    if(in == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "打开连接数据库配置文件失败!\n");
        release_database();
        return OPEN_ERROR;
    }

    /*获取配置文件总行数*/
    int size = 0;
    while(getline(&line,&len,in) != -1)
    {
        if(strlen(trim(line))&&strcmp(line,"\n"))
            size += 1;
    }
    /*重置文件指针*/
    rewind(in);
    
    /*字段长度为去除头尾两行后的长度*/
    size -= 2;

    MYSQL_BIND params[size];
    memset(params, 0, sizeof(params));
    TColumn *pColumn = (TColumn *)malloc(sizeof(TColumn)*size);
    memset(pColumn, 0, sizeof(TColumn)*size);

    /*取出表名，用于组sql语句*/
    getline(&line,&len,in);
    line = trim(line);
    line = strtok(line, " ");
    strcpy(table, line);
    int i = 0;
    while(getline(&line,&len,in) != -1 && i < size)
    {
        
        line = trim(line);
        char *field = strtok(line, " ");
        char *type = strtok(NULL, " ");
        strcat(sql, field);
        if(i < size - 1)
            strcat(sql, ", ");
        else
            strcat(sql, " from ");
        
        if(strstr(type, "int"))
        {
            pColumn[i].nType = MYSQL_TYPE_LONG;
            params[i].buffer_type = MYSQL_TYPE_LONG;
            params[i].buffer = &pColumn[i].UColumn.iInt;
        }
        else if(strstr(type, "double"))
        {
            pColumn[i].nType = MYSQL_TYPE_DOUBLE;
            params[i].buffer_type = MYSQL_TYPE_DOUBLE;
            params[i].buffer = &pColumn[i].UColumn.dDouble;
        }
        else if(strstr(type, "varchar"))
        {
            pColumn[i].nType = MYSQL_TYPE_STRING;
            params[i].buffer_type = MYSQL_TYPE_STRING;
            params[i].buffer = pColumn[i].UColumn.cChar;
            params[i].buffer_length = sizeof(pColumn[i].UColumn.cChar);
        }
         else if(strstr(type, "timestamp"))
        {
            pColumn[i].nType = MYSQL_TYPE_TIMESTAMP;
            params[i].buffer_type = MYSQL_TYPE_TIMESTAMP;
            params[i].buffer = &pColumn[i].UColumn.tTime;
            params[i].buffer_length = sizeof(pColumn[i].UColumn.tTime);
        }
        i += 1;
    }
    fclose(in);
    strcat(sql, table);
    strcat(sql, ";");

    MYSQL_STMT *stmt = mysql_stmt_init(mysql);
    if(mysql_stmt_prepare(stmt, sql, strlen(sql)))
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "预编译sql语句失败，错误原因:%s\n", mysql_error(mysql));
        release_database();
        return OPERAT_ERROR;
    }
    mysql_stmt_bind_result(stmt, params);
    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);

    out = fopen(output_file,"w");
    int count = 0;
    while(mysql_stmt_fetch(stmt) == 0)
    {
        int i;
        for(i = 0; i < size; i++)
        {
            if(pColumn[i].nType == MYSQL_TYPE_LONG)
            {
                if(i == size - 1)
                {
                    fprintf(out, "%d\n",pColumn[i].UColumn.iInt);
                }
                else
                {
                    fprintf(out, "%d%s",pColumn[i].UColumn.iInt, trim(line));
                }
            }
            if(pColumn[i].nType == MYSQL_TYPE_DOUBLE)
            {
                if(i == size - 1)
                {
                    fprintf(out, "%lf\n",pColumn[i].UColumn.dDouble);
                }
                else
                {
                    fprintf(out, "%lf%s",pColumn[i].UColumn.dDouble, trim(line));
                }
            }
            if(pColumn[i].nType == MYSQL_TYPE_STRING)
            {
                if(i == size - 1)
                {
                    fprintf(out, "%s\n",pColumn[i].UColumn.cChar);
                }
                else
                {
                    fprintf(out, "%s%s",pColumn[i].UColumn.cChar, trim(line));
                }
            }
            if(pColumn[i].nType == MYSQL_TYPE_TIMESTAMP)
            {
                if(i == size - 1)
                {
                    fprintf(out, "%04d-%02d-%02d-%02d-%02d-%02d-%ld\n",\
                        pColumn[i].UColumn.tTime.year,\
                        pColumn[i].UColumn.tTime.month,\
                        pColumn[i].UColumn.tTime.day,\
                        pColumn[i].UColumn.tTime.hour,\
                        pColumn[i].UColumn.tTime.minute,\
                        pColumn[i].UColumn.tTime.second,\
                        pColumn[i].UColumn.tTime.second_part);
                }
                else
                {
                    fprintf(out, "%04d-%02d-%02d-%02d-%02d-%02d-%ld%s",\
                        pColumn[i].UColumn.tTime.year,\
                        pColumn[i].UColumn.tTime.month,\
                        pColumn[i].UColumn.tTime.day,\
                        pColumn[i].UColumn.tTime.hour,\
                        pColumn[i].UColumn.tTime.minute,\
                        pColumn[i].UColumn.tTime.second,\
                        pColumn[i].UColumn.tTime.second_part,\
                        trim(line));
                }
            }
            
        }
        count += 1;
        if(count % 10000 == 0)
            logger(LOG_INFO, __FILE__, __LINE__, "导出了第%d条记录。\n", count);
    }
    logger(LOG_INFO, __FILE__, __LINE__, "共导出记录: %d 条。\n", count);
    fclose(out);
    mysql_stmt_close(stmt);
    
    return NORMAL_EXIT;
}


/*
 *import_data      函数用于从文本文件请入数据到数据表中。
 *@filename        需要导入的文件名。
 *返回值           成功返回零，失败返回非零。
 */
int import_data(const char *config_file, const char *input_file)
{
    FILE   *config     = NULL;
    FILE   *source     = NULL;
    char   *line       = NULL;
    size_t  len        = 0;
    char    sql[2048]  = "insert into ";
    char    split[10];


    /*解析配置文件*/
    config = fopen(config_file, "r");
    if(config == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "打开数据库配置文件失败!\n");
        release_database();
        return OPEN_ERROR;
    }

    /*获取配置文件总行数*/
    int size = 0;
    while(getline(&line,&len,config) != -1)
    {
        if(strlen(trim(line))&&strcmp(line,"\n"))
            size += 1;
    }
    /*重置文件指针*/
    rewind(config);
    
    /*字段长度为去除头尾两行后的长度*/
    size -= 2;

    TColumn *pColumn = (TColumn *)malloc(sizeof(TColumn)*size);
    memset(pColumn, 0, sizeof(TColumn)*size);

    /*取出表名，用于组sql语句*/
    getline(&line,&len,config);
    line = trim(line);
    line = strtok(line, " ");
    strcat(sql, line);
    strcat(sql, "(");
    int i = 0;
    while(getline(&line,&len,config) != -1 && i < size)
    {
        
        line = trim(line);
        char *field = strtok(line, " ");
        char *type = strtok(NULL, " ");
        strcat(sql, field);
        if(i < size - 1)
            strcat(sql, ", ");
        else
            strcat(sql, ") values( ");
        
        if(strstr(type, "int"))
        {
            pColumn[i].nType = MYSQL_TYPE_LONG;
        }
        else if(strstr(type, "double"))
        {
            pColumn[i].nType = MYSQL_TYPE_DOUBLE;
        }
        else if(strstr(type, "timestamp"))
        {
            pColumn[i].nType = MYSQL_TYPE_TIMESTAMP;
        }
        else if(strstr(type, "varchar"))
        {
            pColumn[i].nType = MYSQL_TYPE_STRING;
        }
        i += 1;
    }
    strcpy(split, trim(line));
    fclose(config);
    for(i = 0;i < size - 1; i++)
    {
        strcat(sql, "?, ");
    }
    strcat(sql, "?);");

    MYSQL_BIND params[size];
    memset(params, 0, sizeof(params));
    MYSQL_STMT *stmt = mysql_stmt_init(mysql);
    
    if(mysql_stmt_prepare(stmt, sql, strlen(sql)))
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "预编译sql语句失败，错误原因:%s\n", mysql_error(mysql));
        release_database();
        return OPERAT_ERROR;
    }

    source = fopen(input_file, "r");
    if(source == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "打开输入文件失败!\n");
        release_database();
        return OPEN_ERROR;
    }

    int count = 0;
    while(getline(&line,&len,source) != -1)
    {
        line = trim(line);
        char *result = strtok(line, split);

        int i = 0;
        while(result != NULL)
        {   
            if(pColumn[i].nType == MYSQL_TYPE_LONG)
            {
                pColumn[i].UColumn.iInt = atoi(result);
                if(count == 0)
                {
                    params[i].buffer_type = MYSQL_TYPE_LONG;
                    params[i].buffer = &pColumn[i].UColumn.iInt;
                }
            }
            else if(pColumn[i].nType == MYSQL_TYPE_DOUBLE)
            {
                pColumn[i].UColumn.dDouble = atof(result);
                if(count == 0)
                {
                    params[i].buffer_type = MYSQL_TYPE_DOUBLE;
                    params[i].buffer = &pColumn[i].UColumn.dDouble;
                }
            }
            else if(pColumn[i].nType == MYSQL_TYPE_STRING)
            {
                strcpy(pColumn[i].UColumn.cChar, result);
                if(count == 0)
                {
                    params[i].buffer_type = MYSQL_TYPE_STRING;
                    params[i].buffer = pColumn[i].UColumn.cChar;
                    params[i].buffer_length = strlen(pColumn[i].UColumn.cChar);
                }
            }
            else if(pColumn[i].nType == MYSQL_TYPE_TIMESTAMP)
            {
                char tmp[50];
                strcpy(tmp, result);
                char *year = strtok(tmp, "-");
                if(year == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "数据文件格式错误,导入失败!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.year = atoi(year);
                
                char *month = strtok(NULL, "-");
                if(month == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "数据文件格式错误,导入失败!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.month = atoi(month);
                
                char *day = strtok(NULL, "-");
                if(day == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "数据文件格式错误,导入失败!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.day = atoi(day);

                char *hour = strtok(NULL, "-");
                if(hour == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "数据文件格式错误,导入失败!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.hour = atoi(hour);

                char *minute = strtok(NULL, "-");
                if(minute == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "数据文件格式错误,导入失败!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.minute = atoi(minute);

                char *second = strtok(NULL, "-");
                if(second == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "数据文件格式错误,导入失败!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.second = atoi(second);

                char *second_part = strtok(NULL, "-");
                if(second_part == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "数据文件格式错误,导入失败!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.second_part = atol(second_part);
                
                if(count == 0)
                {
                    params[i].buffer_type = MYSQL_TYPE_TIMESTAMP;
                    params[i].buffer = &pColumn[i].UColumn.tTime;
                }
            }
           
            result = strtok(NULL , split);
            i += 1;
        } 

        if(count == 0)
        {
            mysql_stmt_bind_param(stmt,params);
        }

        if(mysql_stmt_execute(stmt))
        {
            logger(LOG_INFO, __FILE__, __LINE__, "执行插入语句失败，错误原因:%s\n", mysql_error(mysql));
            release_database();
            fclose(source);
            return OPERAT_ERROR;
        }
    
        count += 1;
        if (count % 10000 == 0)
        { 
            mysql_commit(mysql);
            logger(LOG_INFO, __FILE__, __LINE__, "导入文本数据时，第%d条记录提交了一次!\n", count);
        }
    }
    logger(LOG_INFO, __FILE__, __LINE__, "共导入记录: %d 条!\n", count);
 
    fclose(source);
    mysql_commit(mysql);
    mysql_stmt_close(stmt); 
    
    return NORMAL_EXIT;
}


int delete_table(const char *config_file, const char *condition_sql)
{
    FILE   *config              = NULL;           /*配置文件*/
    char   *line                = NULL;           /*读取配置文件行*/
    size_t  len                 = 0;              /*读取一行文本长度*/
    char    select_sql[100]     = "select ";      /*查询sql语句*/
    char    delete_sql[100]     = "delete from "; /*删除sql语句*/
    char    table_name[50]      = {0};            /*缓存表名用于组sql*/
    char    primary_key[10][50] = {{0}};          /*存主键名，最多10个联合主键，长50*/
    int     key_count           = 0;              /*保存主键长度*/

    MYSQL_RES *res;
    MYSQL_ROW row;

    /*解析配置文件*/
    config = fopen(config_file, "r");
    if(config == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "打开数据库配置文件失败!\n");
        release_database();
        return OPEN_ERROR;
    }

    /*解析配置文件第一行，取出表名，主键*/
    getline(&line,&len,config);
    line = trim(line);
    line = strtok(line, " ");
    strcpy(table_name, line);

    line = strtok(NULL, " ");
    if(line == NULL)
    {
        logger(LOG_INFO, __FILE__, __LINE__, "配置文件错误，没有填写主键!\n");
        release_database();
        fclose(config);
        return OPERAT_ERROR;
    }
    
    /*拼接查询主键sql*/
    char *key = strtok(line, ",");
    while(key != NULL)
    {
        strcat(select_sql, key);
        strcpy(primary_key[key_count++], key);
        key = strtok(NULL, ",");
        if(key)
            strcat(select_sql, ", ");
    }
    strcat(select_sql, " from ");
    strcat(select_sql, table_name);
    strcat(select_sql, ";");
    
    /*如果参数带查询条件，先找到指定条件记录的主键*/
    if(strlen(condition_sql) != 0)
    {
        strcpy(select_sql+strlen(select_sql)-1, " where ");
        strcat(select_sql, condition_sql);
        strcat(select_sql, ";");
    }
    
    fclose(config);

    /*查询主键列*/
    if(mysql_query(mysql, select_sql))
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "删除操作查询主键时失败，错误原因:%s\n", mysql_error(mysql));
        release_database();
        return OPERAT_ERROR;
    }

    /*存储输出结果*/
    res = mysql_store_result(mysql);
    if(res == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "删除操作获取记录集时失败，错误原因:%s\n", mysql_error(mysql));
        release_database();
        return OPERAT_ERROR;
    }

    /*输出查询结果，并存入单链表*/
    List *L = (List *)malloc(sizeof(List));
    L->next = NULL;
    List *p = NULL;
    List *r = NULL;
    r = L;
    while((row = mysql_fetch_row(res)) != NULL)
    {
        p = (List *)malloc(sizeof(List));    /*申请新结点*/

        int i;
        for(i=0;i<mysql_num_fields(res);i++)
        {
            strcpy(p->value[i], row[i]);
        }
        r->next = p;                         /*新结点插入尾部*/
        r = p;                               /*尾指针指向最后*/
    }
    r->next = NULL;
    mysql_free_result(res);
    
    strcat(delete_sql, table_name);
    strcat(delete_sql, " where ");
    int l = strlen(delete_sql);
    
    /*遍历单链表，删除表中数据*/
    List *t= L->next;
    int count = 0;
    while(t)
    {
        int i;
        char tmp1[100] = {0};
        char tmp2[200] = {0};
        for(i=0;i<key_count;i++)
        {
           sprintf(tmp1, "%s='%s'", primary_key[i], t->value[i]);
           strcat(tmp2, tmp1);
           if(i<key_count - 1)
               strcat(tmp2, " and ");
           else
               strcat(tmp2, ";");
        }
        
        /*组装删除sql语句*/
        strcpy(delete_sql+l, tmp2);
        
        if(strlen(condition_sql)!=0)
        {
            strcpy(delete_sql+strlen(delete_sql)-1, " and ");
            strcat(delete_sql, condition_sql);
            strcat(delete_sql, ";");
        }
        
        if(mysql_query(mysql, delete_sql))
        {
            logger(LOG_DEBUG, __FILE__, __LINE__, "执行删除语句时失败，错误原因:%s\n", mysql_error(mysql));
            return OPERAT_ERROR;
        }
        
        count += 1;
        if (count % 10000 == 0)
        { 
            mysql_query(mysql,"COMMIT;");
            logger(LOG_INFO, __FILE__, __LINE__, "删除数据时，第%d条记录提交了一次!\n", count);
        }
        t = t->next;
    }
    mysql_query(mysql,"COMMIT;");
    logger(LOG_INFO, __FILE__, __LINE__, "共删除记录: %d 条!\n", count);

    /*查询执行完毕，删除单链表*/
    List *d ;
    while(L)
    {
        d = L;
        L = L->next;
        d->next = NULL;
        free(d);
        d = NULL;
    }
    
    return NORMAL_EXIT;
}


/*
 *release_database    函数用于释放前面建立的资源。
 *返回值              成功返回零，失败返回非零。
 */
int release_database()
{
    mysql_close(mysql);
    mysql_library_end();
    return NORMAL_EXIT;
}

