#include "Frame_00.h"
#include <string>

namespace libwebsock
{
	size_t Frame_00::packFrame( std::string data, unsigned char*& frame, const bool maskit )
	{ // maskit is ignored. this is a lame version of the protocol
		size_t size = data.size( ) + 2;
		frame = new unsigned char[ size ];
		
		// add the padders
		frame[ 0 ] = 0x00;
		frame[ size - 1 ] = 0xFF;
		
		// add the data
		unsigned char* datastart = frame + 1;
		for( int i = 0; i < data.size( ); i++ )
		{
			datastart[ i ] = data[ i ];
		}
		
		return size;
	}
	
	void Frame_00::unpackFrame( unsigned char* rawdata, size_t size )
	{
		int end = 1;
		bool foundframe = false;
		
		while( end < size )
		{
			if( rawdata[ 0 ] == 0x00 && rawdata[ end ] == (unsigned char)0xFF)
			{
				_data = std::string( (char*)rawdata + 1, end - 1 );
				_disconnect = false;
				return;
			}
			else if( rawdata[ 0 ] == (unsigned char)0xFF && rawdata[ end ] == 0x00)
			{
				_disconnect = true;
				return;
			}
			
			end++;
		}
		
		_disconnect = true;
	}
	
	Frame_00::Frame_00( unsigned char* rawdata, size_t size )
	{
		unpackFrame( rawdata, size );
	}
}