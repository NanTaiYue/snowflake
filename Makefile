linux : snowflake.so

clean :
	rm -f *.so *.o snowflake

snowflake.so : snowflake.c
	gcc -Wall -g -o $@ -fPIC --shared $^ -I/usr/local/include -L/usr/local/bin -llua53