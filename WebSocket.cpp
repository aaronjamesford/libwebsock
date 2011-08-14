#include <string>
#include "md5/md5.h"

void WebSocket::_handshake( const std::string& header )
{
	
}

std::string WebSocket::_genSecret( )
{
	int k1 = _extractKey( _key1 );
	int k2 = _extrackKey( _key2 );
	
	return md5( _getBigEndRep( k1 ) + _getBigEndRep( k2 ) + _l8b, true );
}

int Websocket::_extractKey( const std::string& token )
{
	unsigned int spaceCount = 0;
	unsigned int res = 0;
	
	std::string::const_iterator it = token.begin( );
	while( it != token.end( ) )
	{
		if( *it == ' ' )
		{
			++spaceCount;
		}
		else
		{
			int c = *it - '0';
			if( c >= 0 && c <= 9 )
			{
				res = res * 10;
				res = res + c;
			}
		}
		
		++it;
	}
	
	if( spaceCount == 0 )
	{ // error time
		return -1;
	}
	
	return res / spaceCount;
}

std::string _getBigEndRep( unsigned int x )
{
	int test = 1;
	char* endian = (char*)&test;
	
	if( endian[ 0 ] )
	{ // little endian
		x = (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
	}
	
	unsigned char* r = (unsigned char*)&x;
	std::string res = "";
	for( int i = 0; i < 4; i++ )
	{
		res += r[ i ];
	}
	
	return res;
}