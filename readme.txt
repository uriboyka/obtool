配置文件说明
1.连接数据库配置文件示例(connect.ini)
HOST=192.168.1.65     #数据库地址，以HOST=开头
USERNAME=admin        #数据库用户名，以USERNAME=开头
PASSWORD=admin        #数据库密码，以PASSWORD=开头
PORT=2880             #数据库端口号，以PORT=开头

2.数据库表配置文件示例
test id,name               #第一行以表名开头，空格分隔后接主键，多个主键以逗号(,)分隔
id int                     #第二行开始写字段名和字段类型，中间用空格分隔
name     varchar(65536)    #字段名 + 空格 + 类型
addr     varchar(65536)    #字段名 + 空格 + 类型
tel      varchar(65535)    #字段名 + 空格 + 类型
 ,                         #最后一行表示导出数据到文件时所用的分隔符

3.删除数据使用如下命令
data_export -c connectfile -t tablefile -d
注:如果d后面没接任何参数则会清空整张表。
   如果d后面接参数则参数需要紧接着d，即中间没有空格。
   data_export -c connectfile -t tablefile -dname='zhangsan'