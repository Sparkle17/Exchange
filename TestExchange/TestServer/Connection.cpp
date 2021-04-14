#include "Connection.h"
#include "Logger.h"
#include "NumServer.h"

Connection::Connection(tcp::socket&& socket, weak_ptr<NumServer> server)
	: m_socket(move(socket)),
    m_server(move(server))
{
}

void Connection::connect()
{
    process();
}

void Connection::disconnect()
{
	m_socket.close();
}

void Connection::process() 
{
    auto self = shared_from_this();
    boost::asio::async_read(m_socket, boost::asio::buffer(&m_request, sizeof(m_request)), [this, self](error_code ec, size_t size)
        {
            if (!ec || ec.value() == boost::asio::error::eof) {
                auto server = m_server.lock();
                m_answer = server->processRequest(m_request);
                boost::asio::async_write(m_socket, boost::asio::buffer(&m_answer, sizeof(m_answer)), [this, self](error_code ec, size_t size)
                    {
                        Logger::i("answer sent");
                        if (ec)
                            disconnect();
                    });
                process();
            }
            else
                disconnect();
        });
}
