/*
 * =====================================================================================
 *
 *       Filename:  main.c
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
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "database.h"

int main(int argc, char **argv)
{
    char connect_cfg[256]    = {0};
    char config_cfg[256]     = {0};
    char export_file[256]    = {0};
    char import_file[256]    = {0};
    char condition_sql[1024] = {0};

    bool delete              = false;

    
#ifdef __DEBUG__
    g_loglevel = LOG_DEBUG;
#elif __INFO__
    g_loglevel = LOG_INFO;
#elif __WARN__
    g_loglevel = LOG_WARN;
#else
    g_loglevel = LOG_ERROR;
#endif

    if(argc < 6)
    {
        printf("Usage:\n");
        printf("\tdata_export -c connectfile -t tablefile -o outputfile.(导出数据)\n");
        printf("\tdata_export -c connectfile -t tablefile -i inputfile.(导入数据)\n");
        printf("\tdata_export -c connectfile -t tablefile -dname='test'.(删除数据，带条件)\n");
        printf("\tdata_export -c connectfile -t tablefile -d.(清空数据)\n");
        printf("Option:\n");
        printf("\t-c connect file\t连接到数据库所需的配置文件.\n");
        printf("\t-t table file\t导入导出所需的表配置文件.\n");
        printf("\t-i input file\t需要导入到数据库的数据文件.\n");
        printf("\t-o output file\t存放数据导出的文件名.\n");
        printf("\t-d delete condition\t删除表中数据，如果未指定条件，则清空整张表.\n");
        printf("Example:\n");
        printf("\t数据库连结配置文件:connect.ini\n");
        printf("\t数据表字段配置文件:table.ini\n");
        printf("\t1.导出数据到文件database.txt\n");
        printf("\tdata_export -c connnect.ini -t table.ini -o database.txt\n");
        printf("\t2.从database.txt文件导入数据到数据库\n");
        printf("\tdata_export -c connnect.ini -t table.ini -i database.txt\n");
        printf("\t3.清空表中所有数据\n");
        printf("\tdata_export -c connnect.ini -t table.ini d\n");
        printf("\t4.删除表中id小于100的记录\n");
        printf("\tdata_export -c connnect.ini -t table.ini d\'id<100\'\n"); 
            
        return OPERAT_ERROR;
    }

    int ch = 0;
    while((ch=getopt(argc,argv,"c:t:o:i:d::"))!=-1)
    {
        switch(ch)
        {
            case 'c':
                /*读取数据库连接配置*/
                strcpy(connect_cfg, optarg);
                break;
            case 't':
                strcpy(config_cfg, optarg);
                break;
            case 'o':
                strcpy(export_file, optarg);
                break;
            case 'i':
                strcpy(import_file, optarg);
                break;
            case 'd':
                if(optarg)
                    strcpy(condition_sql, optarg);
                delete = true;
                break;
            default:
                logger(LOG_INFO, __FILE__, __LINE__,"参数错误!\n");
                break;
        }
    }

    /*初始化数据库*/
    if(init_database(connect_cfg))
    {
        logger(LOG_INFO, __FILE__, __LINE__, "数据库初始化失败!\n");
        return OPERAT_ERROR;
    }

    /*将数据导出到txt文件*/
    if(export_file[0])
    {
        if(export_data(config_cfg, export_file))
        {
            logger(LOG_INFO, __FILE__, __LINE__, "将数据导出到文件失败!\n");
            return OPERAT_ERROR;
        }
    }

    /*将txt文件中数据导入到测试表中*/
    if(import_file[0])
    {
        if(import_data(config_cfg, import_file))
        {
            logger(LOG_INFO, __FILE__, __LINE__, "从文件导入数据失败!\n");
            return OPERAT_ERROR;
        }
    }

    /*清空表中数据*/
    if(delete)
    {
        if(delete_table(config_cfg, condition_sql))
        {
            logger(LOG_INFO, __FILE__, __LINE__, "删除表数据失败!\n");
            return OPERAT_ERROR;
        }
    }

    /*释放资源*/
    if(release_database())
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "释放资源失败!\n");
        return OPERAT_ERROR;
    }
    return NORMAL_EXIT;
}
