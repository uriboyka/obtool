�����ļ�˵��
1.�������ݿ������ļ�ʾ��(connect.ini)
HOST=192.168.1.65     #���ݿ��ַ����HOST=��ͷ
USERNAME=admin        #���ݿ��û�������USERNAME=��ͷ
PASSWORD=admin        #���ݿ����룬��PASSWORD=��ͷ
PORT=2880             #���ݿ�˿ںţ���PORT=��ͷ

2.���ݿ�������ļ�ʾ��
test id,name               #��һ���Ա�����ͷ���ո�ָ������������������Զ���(,)�ָ�
id int                     #�ڶ��п�ʼд�ֶ������ֶ����ͣ��м��ÿո�ָ�
name     varchar(65536)    #�ֶ��� + �ո� + ����
addr     varchar(65536)    #�ֶ��� + �ո� + ����
tel      varchar(65535)    #�ֶ��� + �ո� + ����
 ,                         #���һ�б�ʾ�������ݵ��ļ�ʱ���õķָ���

3.ɾ������ʹ����������
data_export -c connectfile -t tablefile -d
ע:���d����û���κβ������������ű�
   ���d����Ӳ����������Ҫ������d�����м�û�пո�
   data_export -c connectfile -t tablefile -dname='zhangsan'