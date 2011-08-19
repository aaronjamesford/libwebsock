#include "WebSocket.h"

#include <boost/asio.hpp>

class ChatBot : public libwebsock::WebSocket
{
public:
	ChatBot( boost::asio::io_service& io_service, int port ) : libwebsock::WebSocket( io_service, port ) { }
protected:
	libwebsock::ResponseType process( std::string& request, std::string& response )
	{
		if( request == "hello" )
		{
			response = "Why, hello there!";
		}
		else if( request == "weather" )
		{
			response = "Weather sucks here!";
		}
		else
		{
			response = "I have no idea what you just said, but I'm sure it was interesting";
		}
		
		return libwebsock::RESPOND;
	}
};

int main( )
{
	boost::asio::io_service io_stream;
	ChatBot cb( io_stream, 2738 );
	cb.start( );
	io_stream.run( );
}