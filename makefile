_websock := WebSocket.cpp WebSocket.h
_handshake := Handshake.h Handshake.cpp
_md5 := md5/md5.h md5/md5.cpp

_boost := /usr/lib/boost_1_44_0

all : md5 websock

test : md5 handshake websock
	g++ -I${_boost} Handshake.o WebSocket.o md5.o test.cpp -L${_boost}/stage/lib -lboost_system -o test

websock : ${_websock}
	g++ -I${_boost} WebSocket.cpp -c

handshake : ${_handshake}
	g++ -I${_boost} Handshake.cpp -c

md5 : ${_md5}
	g++ ./md5/md5.cpp -c

clean :
	rm -rf *.o
