#pragma once
#include <memory>

#include <boost/asio.hpp>

using namespace std;

class NumServer;

class Connection: public enable_shared_from_this<Connection>
{
	using tcp = boost::asio::ip::tcp;

private:
	tcp::socket m_socket;
	int32_t m_request;
	double m_answer;
	weak_ptr<NumServer> m_server;

	void process();

public:
	Connection(tcp::socket&& socket, weak_ptr<NumServer> server);

	void disconnect();
	void connect();
};
