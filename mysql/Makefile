CFLAGS=-g -Wall -O2
MYSQL_INC=-I/usr/include/mysql
MYSQL_LIB=-L/usr/lib/mysql
LIBS=-llua -ldl -lm -lmysqlclient
CC=gcc

all:luamysql mysql.so

luamysql:mysql.o
	$(CC) $(CFLAGS) $(MYSQL_INC) $(MYSQL_LIB) mysql.o -o $@ $(LIBS)


mysql.so:mysql.o
	$(CC) $(CFLAGS) $(MYSQL_INC) $(MYSQL_LIB) -fPIC -shared mysql.o -o $@ $(LIBS)

mysql.o:mysql.c
	@$(CC) $(CFLAGS) $(MYSQL_INC) -c mysql.c 


clean:
	rm -rf luamysql core *.o *.so
