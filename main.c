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
        printf("\tdata_export -c connectfile -t tablefile -o outputfile.(��������)\n");
        printf("\tdata_export -c connectfile -t tablefile -i inputfile.(��������)\n");
        printf("\tdata_export -c connectfile -t tablefile -dname='test'.(ɾ�����ݣ�������)\n");
        printf("\tdata_export -c connectfile -t tablefile -d.(�������)\n");
        printf("Option:\n");
        printf("\t-c connect file\t���ӵ����ݿ�����������ļ�.\n");
        printf("\t-t table file\t���뵼������ı������ļ�.\n");
        printf("\t-i input file\t��Ҫ���뵽���ݿ�������ļ�.\n");
        printf("\t-o output file\t������ݵ������ļ���.\n");
        printf("\t-d delete condition\tɾ���������ݣ����δָ����������������ű�.\n");
        printf("Example:\n");
        printf("\t���ݿ����������ļ�:connect.ini\n");
        printf("\t���ݱ��ֶ������ļ�:table.ini\n");
        printf("\t1.�������ݵ��ļ�database.txt\n");
        printf("\tdata_export -c connnect.ini -t table.ini -o database.txt\n");
        printf("\t2.��database.txt�ļ��������ݵ����ݿ�\n");
        printf("\tdata_export -c connnect.ini -t table.ini -i database.txt\n");
        printf("\t3.��ձ�����������\n");
        printf("\tdata_export -c connnect.ini -t table.ini d\n");
        printf("\t4.ɾ������idС��100�ļ�¼\n");
        printf("\tdata_export -c connnect.ini -t table.ini d\'id<100\'\n"); 
            
        return OPERAT_ERROR;
    }

    int ch = 0;
    while((ch=getopt(argc,argv,"c:t:o:i:d::"))!=-1)
    {
        switch(ch)
        {
            case 'c':
                /*��ȡ���ݿ���������*/
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
                logger(LOG_INFO, __FILE__, __LINE__,"��������!\n");
                break;
        }
    }

    /*��ʼ�����ݿ�*/
    if(init_database(connect_cfg))
    {
        logger(LOG_INFO, __FILE__, __LINE__, "���ݿ��ʼ��ʧ��!\n");
        return OPERAT_ERROR;
    }

    /*�����ݵ�����txt�ļ�*/
    if(export_file[0])
    {
        if(export_data(config_cfg, export_file))
        {
            logger(LOG_INFO, __FILE__, __LINE__, "�����ݵ������ļ�ʧ��!\n");
            return OPERAT_ERROR;
        }
    }

    /*��txt�ļ������ݵ��뵽���Ա���*/
    if(import_file[0])
    {
        if(import_data(config_cfg, import_file))
        {
            logger(LOG_INFO, __FILE__, __LINE__, "���ļ���������ʧ��!\n");
            return OPERAT_ERROR;
        }
    }

    /*��ձ�������*/
    if(delete)
    {
        if(delete_table(config_cfg, condition_sql))
        {
            logger(LOG_INFO, __FILE__, __LINE__, "ɾ��������ʧ��!\n");
            return OPERAT_ERROR;
        }
    }

    /*�ͷ���Դ*/
    if(release_database())
    {
        logger(LOG_DEBUG, __FILE__, __LINE__, "�ͷ���Դʧ��!\n");
        return OPERAT_ERROR;
    }
    return NORMAL_EXIT;
}
