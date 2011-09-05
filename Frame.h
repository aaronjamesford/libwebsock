#include <string>
#include <cstdlib>

namespace libwebsock
{
	
	class Frame
	{
	public:
		static size_t createFrame( std::string data, unsigned char*& frame, const bool maskit = false )
		{
			// + 2 for the required header + 1 for the '\0'
			size_t size = data.length( ) + 2;
			if( maskit )
			{
				size += 4;
			}
			
			frame = new unsigned char[ size ];
			
			// Because I'm  a lame programmer, this is gonna be the only frame...
			
			// set the FIN = 1, rsv1,2,3 = {0} and opcode = 0x01 all in one
			frame[ 0 ] = (unsigned char)0x81;
			
			if( data.length( ) < 126 )
			{
				frame[ 1 ] = (unsigned char)data.length( );
			}
			// @TODO : make more length options!
			
			unsigned char* datastart = frame + 2;
			
			if( maskit )
			{ // generate the mask
				// first, set the maskflag bit
				frame[ 1 ] = frame[ 1 ] | (unsigned char)0x80;
				
				unsigned int mask = rand( );
				
				// assign some pointers for easy use
				unsigned char* maskptr = (unsigned char*)(&mask);
				unsigned char* maskstart = frame + 2;
				
				// this just sets the bytes in the frame for the mask
				for( int i = 0; i < 4; i++ )
				{
					maskstart[ i ] = maskptr[ i ];
				}
				
				// mask the string time?
				for( int i = 0; i < data.length( ); i++ )
				{
					size_t j = i % 4;
					data[ i ] = (unsigned char)(data[ i ] ^ maskptr[ j ]);
				}
				
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
	
		Frame( unsigned char* rawframe, size_t size )
		{
			_rawframe = rawframe;
			_size = size;
			
			_extract( );
		}
		
		// ~Frame( ) { delete[] _rawframe; delete[] _rawdata; }
		
		std::string data( ) { return _unmaskdata; }
		bool disconnect( ) { return _disconnect; }
	private:
		void _extract( )
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
			
			// I have no idea what to do if the payloadlen is either 126 or 127 inregards of the interpereting length...
			if( _payloadlen == 126 )
			{
				datastart += 2;
			}
			else if( _payloadlen == 127 )
			{
				datastart += 8;
			}
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
		
		void _unmask( )
		{
			unsigned char* maskptr = (unsigned char*)(&_mask);
			
			_unmaskdata = "";
			// _unmaskdata.reserve( _payloadlen );
			for( int i = 0; i < _payloadlen; i++ )
			{
				size_t j = i % 4;
				
				_unmaskdata += (unsigned char)(_rawdata[ i ] ^ maskptr[ j ]);
			}
		}
	
		unsigned char* _rawframe;
		size_t _size;
	
		bool _finflag;
	
		bool _rsv1flag;
		bool _rsv2flag;
		bool _rsv3flag;
		
		unsigned char _opcode;
		
		bool _maskflag;
	
		unsigned char _payloadlen;
		unsigned int _payloadlen1;
		unsigned long long _payloadlen2;
	
		unsigned int _mask;
		
		unsigned char* _rawdata;
		std::string _unmaskdata;
		
		bool _disconnect;
	};
	
}