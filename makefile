_websock := WebSocket.cpp WebSocket.h
_handshake := Handshake.h Handshake.cpp
_md5 := md5/md5.h md5/md5.cpp

_boost := /usr/lib/boost_1_44_0

all : libwebsock.a

chatbot : all
	g++ -g  chatbot.cpp libwebsock.a -lboost_system -lboost_thread -lssl -o chatbot
	
libwebsock.a : websock.o handshake.o md5.o
	ar rcs libwebsock.a md5.o Handshake.o WebSocket.o

websock.o : ${_websock}
	g++ -g WebSocket.cpp -c

handshake.o : ${_handshake}
	g++ -g  Handshake.cpp -c

md5.o : ${_md5}
	g++ -g ./md5/md5.cpp -c

clean :
	rm -rf *.o
	rm -rf libwebsock.a
