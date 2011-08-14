#include <string>
#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "md5/md5.h"
#include "WebSocket.h"

void WebSocket::_handshake( const std::string& header )
{
	// the request - i.e. "GET /uobnfsjldf HTTP1.1/1" or whatever the fuck it is
	_req = header.substr( 0, header.find( '\n' ) );
	
	// the Host
	_host = _getField( header, "Host" );
	boost::algorithm::trim( _host );
	
	// the origin
	_origin = _getField( header, "Origin" );
	boost::algorithm::trim( _origin );
	
	// TODO : Probably add some more
	
	// get key 1
	_key1 = _getField( header, "Sec-WebSocket-Key1" );
	boost::algorithm::trim( _key1 );
	
	// get key2
	_key2 = _getField( header, "Sec-WebSocket-Key2" );
	boost::algorithm::trim( _key2 );
	
	// the last 8 bytes is easy peasy
	_l8b = header.substr( header.length( ) - 8 );
	
	// @TODO - get the rest of the fields
	
	std::string secret = _genSecret( );
}

std::string WebSocket::_getField( const std::string& header, const std::string& field )
{
	size_t start = header.find( field );
	size_t colon;
	size_t newline;
	
	if( start != std::string::npos )
	{
		colon = header.find( ':', start );
		if( colon != std::string::npos )
		{
			newline = header.find( '\n', colon );
			if( newline != std::string::npos )
			{
				return header.substr( colon + 1, newline );
			}
		}
	}
	
	return "";
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