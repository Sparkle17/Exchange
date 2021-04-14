#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>

#include "NumClient.h"
#include "..\TestServer\Logger.h"

using namespace std;

static constexpr unsigned short connectionPort(7777);

atomic<bool> exitFlag = false;

BOOL WINAPI Handler(DWORD dwCtrlType) {
    switch (dwCtrlType)
    {
    case CTRL_C_EVENT:
        exitFlag = true;
        return TRUE;
    }
    return FALSE;
}

int main()
{
    int i = 0;
    string logFile;
    // unique log files for multiple clients
    while (1) {
        i++;
        logFile = "client_" + to_string(i) + ".txt";
        ifstream f(logFile);
        if (!f.good())
            break;
    };

    Logger::getInstance().addFile(Logger::level::info, logFile);
    Logger::getInstance().setConsoleLogLevel(Logger::level::info);
    SetConsoleCtrlHandler(Handler, TRUE);

    boost::asio::io_context context;
    auto client = make_shared<NumClient>(connectionPort);
    client->connect();
    while (!exitFlag.load())
        this_thread::sleep_for(100ms);
    client->disconnect();
    Logger::i("disconnected from server");

    return 0;
}
