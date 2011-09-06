#ifndef FRAME_H
#define FRAME_H

#include <string>

namespace libwebsock
{
	class Frame
	{
	public:
		virtual size_t packFrame( std::string data, unsigned char*& frame, const bool maskit ) = 0;
		virtual void unpackFrame( unsigned char* rawdata, size_t size ) = 0;
		
		bool disconnect( ) { return _disconnect; }
		std::string data( ) { return _data; }
	protected:
		bool _disconnect;

		std::string _data;
	};
}

#endif
