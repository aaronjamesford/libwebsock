_websock := WebSocket.cpp WebSocket.h
_md5 := md5/md5.h md5/md5.cpp

_boost := /usr/lib/boost_1_44_0

all : md5 websock

websock : ${_websock}
	g++ -O2 -I${_boost} WebSocket.cpp -c

md5 : ${_md5}
	g++ ./md5/md5.cpp -c

clean :
	rm -rf *.o