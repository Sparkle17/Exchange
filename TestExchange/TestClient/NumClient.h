#pragma once
#include <memory>
#include <random>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread/thread.hpp>

using namespace std;

class NumClient : public enable_shared_from_this<NumClient>
{
	using tcp = boost::asio::ip::tcp;

private:
	boost::asio::io_context m_context;
	boost::thread_group m_threadpool;
	boost::asio::executor_work_guard<decltype(m_context.get_executor())> m_workguard;

	tcp::socket m_socket;
	atomic<bool> m_connected;

	random_device m_rd;
	mt19937 m_rng;
	uniform_int_distribution<int> m_uni;

	int32_t m_request;
	double m_answer;

	void process();
	void processAnswer(const boost::system::error_code& ec, size_t size);

public:
	NumClient(unsigned short port);

	void connect();
	void disconnect();
};
