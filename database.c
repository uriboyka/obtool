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
 *init_database    �������ڳ�ʼ��mysql��,�����ӵ����ݿ⡣
 *����ֵ           �ɹ������㣬ʧ�ܷ��ط��㡣
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
        logger(LOG_DEBUG, __FILE__, __LINE__, "���������ݿ������ļ�ʧ��!\n");
        return OPEN_ERROR;
    }

    /*��ȡ�������ݿ������ļ�*/
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
        logger(LOG_DEBUG, __FILE__, __LINE__, "����mysql��ʧ��!\n");
        fclose(fp);
        return OPEN_ERROR;
    }
    
    mysql = mysql_init(NULL);
    if(mysql == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "��ʼ��mysqlʧ��!\n");
        fclose(fp);
        mysql_library_end();
        return OPEN_ERROR;
    }

    if(!mysql_real_connect(mysql, host,username, password, "test", port, NULL, 0))
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "�������ݿ�ʧ�ܣ�����ԭ��:%s\n", mysql_error(mysql));
        fclose(fp);
        mysql_close(mysql);
        mysql_library_end();
        return OPEN_ERROR;
    }

    /*�������ӿ��ֶ��ύ*/
    mysql_autocommit(mysql, 0);
    
    fclose(fp);
    return NORMAL_EXIT;
}



/*
 *export_data      �������ڵ����������ݵ�txt�ĵ���
 *@filename        ��Ҫ�������ļ���
 *����ֵ           �ɹ������㣬ʧ�ܷ��ط��㡣
 */
int export_data(const char *config_file, const char *output_file)
{ 
    FILE   *in        = NULL;
    FILE   *out       = NULL;
    char   *line      = NULL;
    size_t  len       = 0;
    char    sql[2048]  = "select ";
    char    table[50];

    /*���������ļ�*/
    in = fopen(config_file, "r");
    if(in == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "���������ݿ������ļ�ʧ��!\n");
        release_database();
        return OPEN_ERROR;
    }

    /*��ȡ�����ļ�������*/
    int size = 0;
    while(getline(&line,&len,in) != -1)
    {
        if(strlen(trim(line))&&strcmp(line,"\n"))
            size += 1;
    }
    /*�����ļ�ָ��*/
    rewind(in);
    
    /*�ֶγ���Ϊȥ��ͷβ���к�ĳ���*/
    size -= 2;

    MYSQL_BIND params[size];
    memset(params, 0, sizeof(params));
    TColumn *pColumn = (TColumn *)malloc(sizeof(TColumn)*size);
    memset(pColumn, 0, sizeof(TColumn)*size);

    /*ȡ��������������sql���*/
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
        logger(LOG_DEBUG, __FILE__, __LINE__, "Ԥ����sql���ʧ�ܣ�����ԭ��:%s\n", mysql_error(mysql));
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
            logger(LOG_INFO, __FILE__, __LINE__, "�����˵�%d����¼��\n", count);
    }
    logger(LOG_INFO, __FILE__, __LINE__, "��������¼: %d ����\n", count);
    fclose(out);
    mysql_stmt_close(stmt);
    
    return NORMAL_EXIT;
}


/*
 *import_data      �������ڴ��ı��ļ��������ݵ����ݱ��С�
 *@filename        ��Ҫ������ļ�����
 *����ֵ           �ɹ������㣬ʧ�ܷ��ط��㡣
 */
int import_data(const char *config_file, const char *input_file)
{
    FILE   *config     = NULL;
    FILE   *source     = NULL;
    char   *line       = NULL;
    size_t  len        = 0;
    char    sql[2048]  = "insert into ";
    char    split[10];


    /*���������ļ�*/
    config = fopen(config_file, "r");
    if(config == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "�����ݿ������ļ�ʧ��!\n");
        release_database();
        return OPEN_ERROR;
    }

    /*��ȡ�����ļ�������*/
    int size = 0;
    while(getline(&line,&len,config) != -1)
    {
        if(strlen(trim(line))&&strcmp(line,"\n"))
            size += 1;
    }
    /*�����ļ�ָ��*/
    rewind(config);
    
    /*�ֶγ���Ϊȥ��ͷβ���к�ĳ���*/
    size -= 2;

    TColumn *pColumn = (TColumn *)malloc(sizeof(TColumn)*size);
    memset(pColumn, 0, sizeof(TColumn)*size);

    /*ȡ��������������sql���*/
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
        logger(LOG_DEBUG, __FILE__, __LINE__, "Ԥ����sql���ʧ�ܣ�����ԭ��:%s\n", mysql_error(mysql));
        release_database();
        return OPERAT_ERROR;
    }

    source = fopen(input_file, "r");
    if(source == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "�������ļ�ʧ��!\n");
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
                    logger(LOG_INFO, __FILE__, __LINE__, "�����ļ���ʽ����,����ʧ��!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.year = atoi(year);
                
                char *month = strtok(NULL, "-");
                if(month == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "�����ļ���ʽ����,����ʧ��!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.month = atoi(month);
                
                char *day = strtok(NULL, "-");
                if(day == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "�����ļ���ʽ����,����ʧ��!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.day = atoi(day);

                char *hour = strtok(NULL, "-");
                if(hour == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "�����ļ���ʽ����,����ʧ��!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.hour = atoi(hour);

                char *minute = strtok(NULL, "-");
                if(minute == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "�����ļ���ʽ����,����ʧ��!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.minute = atoi(minute);

                char *second = strtok(NULL, "-");
                if(second == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "�����ļ���ʽ����,����ʧ��!\n");
                    release_database();
                    fclose(source);
                    return OPERAT_ERROR;
                }
                pColumn[i].UColumn.tTime.second = atoi(second);

                char *second_part = strtok(NULL, "-");
                if(second_part == NULL)
                {
                    logger(LOG_INFO, __FILE__, __LINE__, "�����ļ���ʽ����,����ʧ��!\n");
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
            logger(LOG_INFO, __FILE__, __LINE__, "ִ�в������ʧ�ܣ�����ԭ��:%s\n", mysql_error(mysql));
            release_database();
            fclose(source);
            return OPERAT_ERROR;
        }
    
        count += 1;
        if (count % 10000 == 0)
        { 
            mysql_commit(mysql);
            logger(LOG_INFO, __FILE__, __LINE__, "�����ı�����ʱ����%d����¼�ύ��һ��!\n", count);
        }
    }
    logger(LOG_INFO, __FILE__, __LINE__, "�������¼: %d ��!\n", count);
 
    fclose(source);
    mysql_commit(mysql);
    mysql_stmt_close(stmt); 
    
    return NORMAL_EXIT;
}


int delete_table(const char *config_file, const char *condition_sql)
{
    FILE   *config              = NULL;           /*�����ļ�*/
    char   *line                = NULL;           /*��ȡ�����ļ���*/
    size_t  len                 = 0;              /*��ȡһ���ı�����*/
    char    select_sql[100]     = "select ";      /*��ѯsql���*/
    char    delete_sql[100]     = "delete from "; /*ɾ��sql���*/
    char    table_name[50]      = {0};            /*�������������sql*/
    char    primary_key[10][50] = {{0}};          /*�������������10��������������50*/
    int     key_count           = 0;              /*������������*/

    MYSQL_RES *res;
    MYSQL_ROW row;

    /*���������ļ�*/
    config = fopen(config_file, "r");
    if(config == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "�����ݿ������ļ�ʧ��!\n");
        release_database();
        return OPEN_ERROR;
    }

    /*���������ļ���һ�У�ȡ������������*/
    getline(&line,&len,config);
    line = trim(line);
    line = strtok(line, " ");
    strcpy(table_name, line);

    line = strtok(NULL, " ");
    if(line == NULL)
    {
        logger(LOG_INFO, __FILE__, __LINE__, "�����ļ�����û����д����!\n");
        release_database();
        fclose(config);
        return OPERAT_ERROR;
    }
    
    /*ƴ�Ӳ�ѯ����sql*/
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
    
    /*�����������ѯ���������ҵ�ָ��������¼������*/
    if(strlen(condition_sql) != 0)
    {
        strcpy(select_sql+strlen(select_sql)-1, " where ");
        strcat(select_sql, condition_sql);
        strcat(select_sql, ";");
    }
    
    fclose(config);

    /*��ѯ������*/
    if(mysql_query(mysql, select_sql))
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "ɾ��������ѯ����ʱʧ�ܣ�����ԭ��:%s\n", mysql_error(mysql));
        release_database();
        return OPERAT_ERROR;
    }

    /*�洢������*/
    res = mysql_store_result(mysql);
    if(res == NULL)
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "ɾ��������ȡ��¼��ʱʧ�ܣ�����ԭ��:%s\n", mysql_error(mysql));
        release_database();
        return OPERAT_ERROR;
    }

    /*�����ѯ����������뵥����*/
    List *L = (List *)malloc(sizeof(List));
    L->next = NULL;
    List *p = NULL;
    List *r = NULL;
    r = L;
    while((row = mysql_fetch_row(res)) != NULL)
    {
        p = (List *)malloc(sizeof(List));    /*�����½��*/

        int i;
        for(i=0;i<mysql_num_fields(res);i++)
        {
            strcpy(p->value[i], row[i]);
        }
        r->next = p;                         /*�½�����β��*/
        r = p;                               /*βָ��ָ�����*/
    }
    r->next = NULL;
    mysql_free_result(res);
    
    strcat(delete_sql, table_name);
    strcat(delete_sql, " where ");
    int l = strlen(delete_sql);
    
    /*����������ɾ����������*/
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
        
        /*��װɾ��sql���*/
        strcpy(delete_sql+l, tmp2);
        
        if(strlen(condition_sql)!=0)
        {
            strcpy(delete_sql+strlen(delete_sql)-1, " and ");
            strcat(delete_sql, condition_sql);
            strcat(delete_sql, ";");
        }
        
        if(mysql_query(mysql, delete_sql))
        {
            logger(LOG_DEBUG, __FILE__, __LINE__, "ִ��ɾ�����ʱʧ�ܣ�����ԭ��:%s\n", mysql_error(mysql));
            return OPERAT_ERROR;
        }
        
        count += 1;
        if (count % 10000 == 0)
        { 
            mysql_query(mysql,"COMMIT;");
            logger(LOG_INFO, __FILE__, __LINE__, "ɾ������ʱ����%d����¼�ύ��һ��!\n", count);
        }
        t = t->next;
    }
    mysql_query(mysql,"COMMIT;");
    logger(LOG_INFO, __FILE__, __LINE__, "��ɾ����¼: %d ��!\n", count);

    /*��ѯִ����ϣ�ɾ��������*/
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
 *release_database    ���������ͷ�ǰ�潨������Դ��
 *����ֵ              �ɹ������㣬ʧ�ܷ��ط��㡣
 */
int release_database()
{
    mysql_close(mysql);
    mysql_library_end();
    return NORMAL_EXIT;
}

