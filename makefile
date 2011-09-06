_websock := WebSocket.cpp WebSocket.h
_handshake := Handshake.h Handshake.cpp
_frame := Frame_08.h Frame_08.cpp
_md5 := md5/md5.h md5/md5.cpp

_boost := /usr/lib/boost_1_44_0

all : libwebsock.a

chatbot : all
	g++ -g  chatbot.cpp libwebsock.a -lboost_system -lboost_thread -lssl -o chatbot
	
libwebsock.a : Websock.o Handshake.o md5.o Frame_08.o
	ar rcs libwebsock.a md5.o Handshake.o WebSocket.o Frame_08.o

Frame_08.o : ${_frame}
	g++ -g Frame_08.cpp -c

Websock.o : ${_websock}
	g++ -g WebSocket.cpp -c

Handshake.o : ${_handshake}
	g++ -g  Handshake.cpp -c

md5.o : ${_md5}
	g++ -g ./md5/md5.cpp -c

clean :
	rm -rf *.o
	rm -rf libwebsock.a
