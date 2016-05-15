CC = gcc
TARGET = db_export
CFLAGS = -g -Wall -D __DEBUG__  -I /usr/include -L /usr/lib64/mysql -lmysqlclient
OBJS = main.o database.o common.o
RM = rm -rf
$(TARGET):$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)
$(OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY:clean install
clean:
	$(RM) $(TARGET) $(OBJS)
install:
	mkdir -p ${HOME}/bin
	cp -f $(TARGET) ${HOME}/bin/
