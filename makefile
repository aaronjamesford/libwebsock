_websock := WebSocket.cpp WebSocket.h
_handshake := Handshake.h Handshake.cpp
_md5 := md5/md5.h md5/md5.cpp

_boost := /usr/lib/boost_1_44_0

all : md5 websock

test : md5 handshake websock
	g++ -g -I${_boost} Handshake.o WebSocket.o md5.o test.cpp -L${_boost}/stage/lib -lboost_system -lboost_thread -o test

chatbot : md5 handshake websock
	g++ -g -I${_boost} Handshake.o WebSocket.o md5.o chatbot.cpp -L${_boost}/stage/lib -lboost_system -lboost_thread -o chatbot

websock : ${_websock}
	g++ -g -I${_boost} WebSocket.cpp -c

handshake : ${_handshake}
	g++ -g -I${_boost} Handshake.cpp -c

md5 : ${_md5}
	g++ -g ./md5/md5.cpp -c

clean :
	rm -rf *.o
