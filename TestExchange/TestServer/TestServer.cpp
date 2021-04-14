#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include "Logger.h"
#include "NumServer.h"

static constexpr chrono::seconds dumpFrequencyInSec(100);
static constexpr unsigned short connectionPort(4369);

timed_mutex g_exitFlag;
unique_lock<timed_mutex> g_exitLock(g_exitFlag);

BOOL WINAPI Handler(DWORD dwCtrlType) {
    switch (dwCtrlType)
    {
    case CTRL_C_EVENT:
        g_exitLock.unlock();
        return TRUE;
    }
    return FALSE;
}

int main()
{
    Logger::getInstance().addFile(Logger::level::info, "server_log.txt");
    Logger::getInstance().setConsoleLogLevel(Logger::level::info);
    SetConsoleCtrlHandler(Handler, TRUE);
    
    auto server = make_shared<NumServer>(connectionPort);
	server->connect();
    Logger::i("server started");

    // main thread also used as store dump thread
    while (!g_exitFlag.try_lock_for(dumpFrequencyInSec))
        server->storeDump();

    server->disconnect();
    Logger::i("server stopped");

    return 0;
}
