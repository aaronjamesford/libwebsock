#include "WebSocket.h"

#include <boost/asio.hpp>

class ChatBot : public libwebsock::WebSocket
{
public:
	ChatBot( boost::asio::io_service& io_service, int port ) : libwebsock::WebSocket( io_service, port ) { }
protected:
	void _process( libwebsock::User& u, std::string req )
	{
		std::string response;
		
		if( req == "hello" )
		{
			response = "Why, hello there!";
		}
		else if( req == "weather" )
		{
			response = "Weather sucks here!";
		}
		else
		{
			response = "I have no idea what you just said, but I'm sure it was itneresting";
		}
		
		_send( u.sock, char( 0x00 ) + response + char( 0xFF ) );
	}
};

int main( )
{
	boost::asio::io_service io_stream;
	ChatBot cb( io_stream, 2738 );
	cb.start( );
}