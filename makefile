_websock := WebSocket.cpp WebSocket.h
_md5 := md5/md5.h md5/md5.cpp

_boost := /usr/lib/boost_1_44_0

all : websock md5

websock : ${_websock} md5
	g++ -O2 -I${_boost} ${_websock} -c

md5 : ${_md5}
	g++ ${_md5} -c

clean :
	rm -rf *.o