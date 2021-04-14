#pragma once
#include <chrono>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"

using namespace std;

namespace pt = boost::posix_time;

class LogWriter
{
public:
    virtual ~LogWriter() {}
    virtual void flush() {}
    virtual void print(const string& message) = 0;
};

class ConsoleWriter : public LogWriter
{
public:
    virtual void print(const string& message) override
    {
        cout << message;
    }
};

class FileWriter : public LogWriter
{
private:
    ofstream m_file;

public:
    FileWriter(const string& fileName)
        : m_file(fileName, ios_base::app) {}
    virtual ~FileWriter()
    {
        m_file.close();
    }

    virtual void flush() override
    {
        m_file.flush();
    }
    virtual void print(const string& message) override
    {
        m_file << message;
    }
};


class Logger
{
public:
    enum class level { debug = 0, info = 1, warning = 2, error = 3 };

private:
    vector<pair<level, unique_ptr<LogWriter>>> m_writers;
    mutex m_mutex;

    Logger()
    {
        m_writers.push_back({ level::info, make_unique<ConsoleWriter>() });
    }
    Logger(Logger const&) = delete;
    Logger& operator=(Logger const&) = delete;

    void out(level logLevel, const string& msg)
    {
        lock_guard<mutex> lock(m_mutex);
        pt::ptime now = pt::microsec_clock::local_time();
        stringstream ss;
        ss << now << ": " << msg << endl;
        for (auto& p : m_writers)
            if (p.first <= logLevel)
                p.second->print(ss.str());
    }

public:
    static Logger& getInstance()
    {
        static Logger instance;
        return instance;
    }

    static void d(const string& msg)
    {
        Logger::getInstance().out(level::debug, msg);
    }
    static void e(const string& msg)
    {
        Logger::getInstance().out(level::error, msg);
    }
    static void i(const string& msg)
    {
        Logger::getInstance().out(level::info, msg);
    }
    static void w(const string& msg)
    {
        Logger::getInstance().out(level::warning, msg);
    }

    void addFile(level logLevel, const string& fileName)
    {
        lock_guard<mutex> lock(m_mutex);
        m_writers.push_back({ logLevel, make_unique<FileWriter>(fileName) });
    }

    void setConsoleLogLevel(level logLevel)
    {
        lock_guard<mutex> lock(m_mutex);
        m_writers[0].first = logLevel;
    }
};
