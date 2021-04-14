#include "NumClient.h"
#include "..\TestServer\Logger.h"

NumClient::NumClient(unsigned short port)
	: m_workguard(m_context.get_executor()),
	m_socket(m_context),

	m_rng(m_rd()),
	m_uni(0, 1023)
{
	auto maxThreads = thread::hardware_concurrency();
	for (unsigned int i = 0; i < maxThreads; i++)
		m_threadpool.create_thread(boost::bind(&boost::asio::io_context::run, &m_context));
}

void NumClient::connect()
{
	tcp::resolver resolver(m_context);
	auto addr = resolver.resolve("127.0.0.1", "4369");

	auto self = shared_from_this();
	boost::asio::async_connect(m_socket, addr, [this, self](const boost::system::error_code& ec, const tcp::endpoint&)
		{
			if (!ec) {
				Logger::i("connected to server");
				m_connected = true;
				process();
			}
			else {
				Logger::e("error while connecting to server");
				disconnect();
			}
		});
}

void NumClient::disconnect()
{
	m_connected = false;

	m_workguard.reset();
	m_threadpool.join_all();
}

void NumClient::process()
{
	if (m_connected.load()) {
		m_request = m_uni(m_rng);
		auto self = shared_from_this();
		boost::asio::async_write(m_socket, boost::asio::buffer(&m_request, sizeof(m_request)), [this, self](const boost::system::error_code& ec, size_t size)
			{
				if (!ec || ec.value() == boost::asio::error::eof) {
					Logger::i("request sent: " + to_string(m_request));
					boost::asio::async_read(m_socket, boost::asio::buffer(&m_answer, sizeof(m_answer)), bind(&NumClient::processAnswer, shared_from_this(), placeholders::_1, placeholders::_2));
				}
				else {
					Logger::e("error while sending request");
					disconnect();
				}
			});
	}
	else
		m_socket.close();
}

void NumClient::processAnswer(const boost::system::error_code& ec, size_t size)
{
	if (!ec || ec.value() == boost::asio::error::eof) {
		Logger::i("answer received: " + to_string(m_answer));
		process();
	}
	else {
		Logger::e("error while receiving answer");
		disconnect();
	}
}
