#include <application.h>
#include <boost/asio.hpp>

/*
 * Simple server example
 */
void service::work()
{
	using boost::asio::ip::tcp;

	std::shared_ptr<boost::application::status> st =
		context_.find<boost::application::status>();

	boost::asio::io_service io_service;
	tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 9512));

	while(st->state() != boost::application::status::stopped)
	{
		boost::system::error_code ec;
		tcp::socket socket(io_service);
		acceptor.accept(socket);

		char data[1024];
		size_t len = socket.read_some(boost::asio::buffer(data), ec);

		if (ec == boost::asio::error::eof)
		{
			break;
		}
		else if (ec)
		{
			throw boost::system::system_error(ec);
		}

		std::string msg(data, len);
		boost::asio::write(socket, boost::asio::buffer(msg),
				boost::asio::transfer_all(), ec);
	}
}

int main(int argc, char* argv[])
{
	application app;		// create object of application
	app.start(argc, argv);	// call start functaion from application.h
	return 0;
}
