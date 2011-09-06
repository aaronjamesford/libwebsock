#include "Frame_08.h"

#include <string>
#include <cstdlib>

namespace libwebsock
{
	size_t Frame_08::packFrame( std::string data, unsigned char*& frame, const bool maskit )
	{
		// + 2 for the required header, any additional will be added later
		size_t size = data.length( ) + 2;
		if( maskit )
		{ // Need space for the mask
			size += 4;
		}
		
		if( data.length( ) > 126 && data.length( ) < (unsigned short)0x7FFF )
		{ // need an extra long payloadlen
			size += 2;
		}
		
		// create the space for the frame and set some start values
		frame = new unsigned char[ size ];
		unsigned char* datastart = frame + 2;
		unsigned char* maskstart = frame + 2;
		
		// Because I'm  a lame programmer, this is gonna be the only frame...
		
		// set the FIN = 1, rsv1,2,3 = {0} and opcode = 0x01 all in one
		frame[ 0 ] = (unsigned char)0x81;
		
		if( data.length( ) < 126 )
		{
			frame[ 1 ] = (unsigned char)data.length( );
		}
		else if( data.length( ) < (unsigned short)0x7FFF )
		{
			frame[ 1 ] = (unsigned char)126;
			
			unsigned short payloadlen = (unsigned short)data.length( );
			unsigned char* payloadptr = (unsigned char*)&payloadlen;
			unsigned char* payloadstart = frame + 2;
			
			for( int i = 0; i < 2; i++ )
			{
				payloadstart[ i ] = payloadptr[ i ];
			}
			
			datastart += 2;
			maskstart += 2;
		}
		// @TODO : make more length options!
		
		if( maskit )
		{ // generate the mask
			// first, set the maskflag bit
			frame[ 1 ] = frame[ 1 ] | (unsigned char)0x80;
			
			unsigned int mymask = rand( );
			
			// assign some pointers for easy use
			unsigned char* maskptr = (unsigned char*)(&mymask);
			
			// this just sets the bytes in the frame for the mask
			for( int i = 0; i < 4; i++ )
			{
				maskstart[ i ] = maskptr[ i ];
			}
			
			unsigned char* masked = new unsigned char[ data.length( ) ];
			mask( (unsigned char*)data.c_str( ), mymask, data.length( ), masked );
			data = std::string( (char*)masked, data.length( ) );
			
			// set teh datastart to soemthing FRESH
			datastart = datastart + 4;
		}
		
			// set the data
		for( int i = 0; i < data.length( ); i++ )
		{
			datastart[ i ] = (unsigned char)data[ i ];
		}
		
		return size;
	}
	
	void Frame_08::unpackFrame( unsigned char* rawdata, size_t size )
	{
		// return new Frame_08( rawdata, size );
		_rawframe = rawdata;
		_size = size;
		
		_extract( );
	}
	
	Frame_08::Frame_08( unsigned char* rawframe, size_t size )
	{
		unpackFrame( rawframe, size );
	}
		
	void Frame_08::_extract( )
	{
		_finflag = _rawframe[ 0 ] & (unsigned char)0x80;
		
		_rsv1flag = _rawframe[ 0 ] & (unsigned char)0x40;
		_rsv2flag = _rawframe[ 0 ] & (unsigned char)0x20;
		_rsv3flag = _rawframe[ 0 ] & (unsigned char)0x10;
		
		_opcode = _rawframe[ 0 ] & (unsigned char)0x0F;
		_disconnect = (_opcode == (unsigned char)0x08);
		
		_maskflag = _rawframe[ 1 ] & (unsigned char)0x80;
		
		_payloadlen = _rawframe[ 1 ] & (unsigned char)0x7F;
		
		size_t datastart = 2;
		if( _maskflag )
		{
			datastart += 4;
		}
		
		// This is nly a guess :S
		unsigned long long ourpayloadlen = _payloadlen;
		if( _payloadlen == 126 )
		{
			datastart += 2;
			
			unsigned char* payloadptr = (unsigned char*)&_payloadlen1;
			unsigned char* payloadstart = _rawframe + 2;
			for( int i = 0; i < 2; i++ )
			{
				payloadptr[ i ] = payloadstart[ i ];
			}
			
			ourpayloadlen = _payloadlen1;
		}
		/**
		 * - TODO: Add support for the 64bit lengths (unlikely to happen)
		 */
		else
		{
			int* _maskptr = (int*)(_rawframe + 2);
			_mask = (_maskflag) ? (unsigned int)*_maskptr : 0;
		}
		
		unsigned char* data = _rawframe + datastart;
		_rawdata = new unsigned char[ _payloadlen ];
		for( int i = 0; i < _payloadlen; i++ )
		{
			_rawdata[ i ] = data[ i ];
		}
		
		if( _maskflag )
		{
			_unmask( );
		}
	}
	
	void Frame_08::_unmask( )
	{
		unsigned char* unmasked = new unsigned char[ _payloadlen ];
		mask( _rawdata, _mask, _payloadlen, unmasked );
		
		_data = std::string( (char*)unmasked, _payloadlen );
	}
	
	void Frame_08::mask( unsigned char* unmasked, unsigned int mask, size_t payloadlen, unsigned char* masked )
	{
		unsigned char* maskptr = (unsigned char*)&mask;
		
		for( int i = 0; i < payloadlen; i++ )
		{
			size_t j = i % 4;
			
			masked[ i ] = (unsigned char)(unmasked[ i ] ^ maskptr[ j ]);
		}
	}
}