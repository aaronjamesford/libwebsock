#ifndef FRAME_H
#define FRAME_H

#include <string>

namespace libwebsock
{
	class Frame
	{
	public:
		static size_t createFrame( std::string data, unsigned char*& frame, const bool maskit = false );
		static void mask( unsigned char* unmasked, unsigned int mask, size_t payloadlen, unsigned char* masked );
	
		Frame( unsigned char* rawframe, size_t size );
		
		// ~Frame( ) { delete[] _rawframe; delete[] _rawdata; }
		
		std::string data( ) { return _unmaskdata; }
		bool disconnect( ) { return _disconnect; }
	private:
		void _extract( );
		void _unmask( );
		
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

#endif
