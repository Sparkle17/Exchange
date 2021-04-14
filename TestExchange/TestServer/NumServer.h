#pragma once
#include <map>
#include <memory>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread/thread.hpp>

#include "Connection.h"

using namespace std;

class NumServer: public enable_shared_from_this<NumServer>
{
	using tcp = boost::asio::ip::tcp;

private:
	boost::asio::io_context m_context;
	boost::thread_group m_threadpool;
	boost::asio::executor_work_guard<decltype(m_context.get_executor())> m_workguard;

	tcp::acceptor m_acceptor;
	tcp::socket m_socket;

	mutex m_clientsAccess;
	vector<shared_ptr<Connection>> m_clients;

	mutex m_dataAccess;
	unsigned long long m_sum;
	unsigned long long m_count;
	map<int32_t, unsigned long> m_numbers;

	void internalAccept();

public:
	NumServer(unsigned short port);

	void connect();
	void disconnect();

	double processRequest(int32_t data);
	void storeDump();
};
