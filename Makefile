#使用编译器
CC=g++
#库路径
libmysqlclient_path=/usr/lib64/mysql
libevent_path=/usr/local/lib
libopenssl_path=/usr/local/lib
#库头文件路径
libopenssl_include = /usr/include
libevent_include = /usr/include
libmysqlclient_include = /usr/include

app:test
test:test.o CMutex.o mysql_db.o database.o HttpApiServer.o ./CJsonObject/cJSON.o ./CJsonObject/CJsonObject.o ./Http_websocket/ev_httpd.o ./Http_websocket/ev_websocket.o ./Http_websocket/evbase_threadpool.o ./Http_websocket/Threadpool.o ./Http_websocket/config.o
	$(CC) $^ -std=c++11 -o $@  -lpthread -L $(libevent_path) -levent -L $(libopenssl_path) -lcrypto -lssl -L $(libmysqlclient_path) -lmysqlclient -g 
db_test:CMutex.o mysql_db.o database.o db_test.o
	$(CC) $^ -std=c++11 -o $@ -lpthread -L $(libmysqlclient_path) -lmysqlclient
%.o:%.c 
	$(CC) $^ -std=c++11  -c -o $@ -Wall -I $(libmysqlclient_include) -I $(libopenssl_include) -I $(libevent_include) -g 
%.o:%.cpp 
	$(CC) $^ -std=c++11 -c -o $@ -Wall -I $(libmysqlclient_include) -I $(libopenssl_include) -I $(libevent_include) -g 

.PHONY:clean
clean:
	@rm -rf db_test *.o ./Http_websocket/*.o
