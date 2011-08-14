#include <string>
#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "md5/md5.h"
#include "WebSocket.h"

void WebSocket::_handshake( const std::string& header )
{
	// the last 8 bytes is easy peasy
	_l8b = header.substr( header.length( ) - 8 );
	
	// get key 1
	size_t pos = header.find( "Sec-WebSocket-Key1" );
	_key1 = header.substr( header.find( ':', pos ), header.find( '\n', pos ) );
	boost::algorithm::trim( _key1 );
	
	// get key2
	pos = header.find( "Sec-WebSocket-Key2" );
	_key2 = header.substr( header.find( ':', pos ), header.find( '\n', pos ) );
	boost::algorithm::trim( _key2 );
	
	// @TODO - get the rest of the fields
	
	std::string secret = _genSecret( );
}

std::string WebSocket::_genSecret( )
{
	int k1 = _extractKey( _key1 );
	int k2 = _extractKey( _key2 );
	
	return md5( _getBigEndRep( k1 ) + _getBigEndRep( k2 ) + _l8b, true );
}

int WebSocket::_extractKey( const std::string& token )
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