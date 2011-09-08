#ifndef FRAME_00_H
#define FRAME_00_H

#include <string>
#include "Frame.h"

namespace libwebsock
{
	class Frame_00 : public Frame
	{
	public:
		size_t packFrame( std::string data, unsigned char*& frame, const bool maskit = false );
		size_t unpackFrame( unsigned char* rawdata, size_t size );
		
		Frame_00( unsigned char* rawdata, size_t size );
		Frame_00( ) { }
	};
}

#endif
