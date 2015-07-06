all:
	gcc -g -Wall -O2 mysql.c -o mysql -llua -lm -ldl -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient

clean:
	rm -rf mysql core
