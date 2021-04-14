#include "NumServer.h"
#include "Logger.h"

NumServer::NumServer(unsigned short port)
    : m_workguard(m_context.get_executor()),
    m_acceptor(m_context, tcp::endpoint{ {}, port }),
    m_socket(m_context)
{
    auto maxThreads = thread::hardware_concurrency();
    for (unsigned int i = 0; i < maxThreads; i++)
        m_threadpool.create_thread(boost::bind(&boost::asio::io_context::run, &m_context));
}

void NumServer::connect()
{
	m_acceptor.listen();
	internalAccept();
}

void NumServer::disconnect()
{
    auto self = shared_from_this();
    m_context.post([this, self] { m_acceptor.close(); });

    lock_guard<mutex> lock(m_clientsAccess);
    for (auto& client : m_clients)
        client->disconnect();

    m_workguard.reset();
    m_threadpool.join_all();
}

void NumServer::internalAccept()
{
    auto self = shared_from_this();
    m_acceptor.async_accept(m_socket, [this, self](error_code ec)
        {
            if (!ec) {
                auto client = make_shared<Connection>(move(m_socket), self);
                client->connect();
                unique_lock<mutex> lock(m_clientsAccess);
                m_clients.push_back(client);
                lock.unlock();

                Logger::i("client connected");
                internalAccept();
            }
        });
}

double NumServer::processRequest(int32_t data)
{
    double average = 0;
    lock_guard<mutex> lock(m_dataAccess);
    if (data >= 0 && data <= 1023) {
        m_numbers[data]++;
        m_sum += data * data;
        m_count++;
    }
    Logger::i("new data from client: " + to_string(data));
    return (double)m_sum / m_count;
}

void NumServer::storeDump()
{
    ofstream fdump;
    fdump.open("dump.bin", std::ios::binary);

    unique_lock<mutex> lock(m_dataAccess);
    int32_t count = m_numbers.size();
    fdump.write(reinterpret_cast<char*>(&count), sizeof(count));
    for (auto &pair: m_numbers)
        fdump.write(reinterpret_cast<char*>(&pair), sizeof(pair));
    lock.unlock();

    fdump.close();
}
