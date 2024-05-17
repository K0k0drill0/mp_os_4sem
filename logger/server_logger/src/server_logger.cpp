#include <not_implemented.h>

#include "../include/server_logger.h"

#include <cmath>
#include <stdexcept>

#ifdef __linux__
#include <sys/msg.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
    std::map<std::string, std::pair<mqd_t, unsigned int>> server_logger::_queues_users = std::map<std::string, std::pair<mqd_t, unsigned int>>();
#endif

#ifdef _WIN32
    std::map<std::string, std::pair<HANDLE, unsigned int>> server_logger::_queues_users = std::map<std::string, std::pair<HANDLE, unsigned int>>();
#endif

#define MSG_SIZE 1024

server_logger::server_logger(server_logger const &other) : _queues(other._queues) {
    for (auto &[key, pair] : _queues_users) pair.second++;
}

server_logger &server_logger::operator=(server_logger const &other)
{
    if (this == &other) return *this;
    close_streams();
    _queues = other._queues;
    for (auto &[key, pair] : _queues) _queues_users[key].second++;
    return *this;
}

server_logger::server_logger(server_logger &&other) noexcept :
    _queues(std::move(other._queues)) {}

server_logger &server_logger::operator=(server_logger &&other) noexcept
{
    if (this == &other) return *this;
    close_streams();
    _queues = std::move(other._queues);
    return *this;
}

void server_logger::close_streams()
{
    for (auto & [file, pair] : _queues)
    {
        if (--_queues_users[file].second != 0) continue;
        #ifdef _WIN32
            CloseHandle(_queues_users[file].first);
        #elif __linux__
            mq_close(_queues_users[file].first);
        #endif
        _queues_users.erase(file);

    }
}

server_logger::~server_logger() noexcept
{
    close_streams();
}


server_logger::server_logger(std::map<std::string, std::set<logger::severity>> const configuration) {
    #ifdef __linux__
    _mq_descriptor = msgget(LINUX_MSG_QUEUE_KEY, 0666);

    if (_mq_descriptor == -1) {
        throw std::runtime error("Cannot connect to server");
    }
    #endif

    #ifdef _WIN32
    
    for (auto &[file, severities] : configuration) {
        if (!_queues_users.contains(file)) {
            HANDLE pipe = CreateFileA(file.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (pipe == INVALID_HANDLE_VALUE) {
                throw std::runtime_error("Can not access handle file");
            }
            _queues_users[file].first = pipe;
            _queues_users[file].second = 0;
        }
        
        _queues_users[file].second++;
        _queues[file].first = _queues_users[file].first;
        _queues[file].second = severities;
    }

    #endif
}

logger const *server_logger::log(
    const std::string &text,
    logger::severity severity) const noexcept
{
    size_t meta_size = sizeof(size_t) + sizeof(size_t) + sizeof(pid_t) + sizeof(const char*) + sizeof(bool);
    size_t message_size = MSG_SIZE - meta_size;
    size_t packets_count = text.size() / message_size + 1;

    char info_message[meta_size];
    char *ptr;

    ptr = info_message;
    *reinterpret_cast<bool*>(ptr) = false;
    ptr += sizeof(bool);
    *reinterpret_cast<size_t*>(ptr) = packets_count;
    ptr += sizeof(size_t);
    *reinterpret_cast<size_t*>(ptr) = _request;
    ptr += sizeof(size_t);
    *reinterpret_cast<pid_t*>(ptr) = _process_id;
    ptr += sizeof(pid_t);
    char const * severity_string = severity_to_string(severity).c_str();
    strcpy(ptr, severity_string);

    char message[MSG_SIZE];

    for (auto &[file, pair] : _queues) {
        if (!pair.second.contains(severity)) {
            continue;
        }

        #ifdef _WIN32
            DWORD bytes_written;
            WriteFile(pair.first, info_message, MSG_SIZE, &bytes_written, nullptr);
        #endif

        #ifdef __linux__
        //later
        #endif

        ptr = message;
        *reinterpret_cast<bool*>(ptr) = true;
        ptr += sizeof(bool);
        *reinterpret_cast<size_t*>(ptr) = _request;
        ptr += sizeof(size_t);
        *reinterpret_cast<pid_t*>(ptr) = _process_id;
        ptr += sizeof(pid_t);

        for (size_t i = 0; i < packets_count; i++) {
            size_t pos= i * message_size;
            size_t rest = text.size() - pos;
            size_t substr_size = (rest < message_size) ? rest : message_size;
            memcpy(ptr, text.substr(pos, substr_size).c_str(), substr_size);
            *(ptr + substr_size) = 0;
            #ifdef _WIN32
                WriteFile(pair.first, message, MSG_SIZE, &bytes_written, nullptr);
            #endif

            #ifdef __linux__
                //later
            #endif  
        }

    }

    _request++;

    return this;
}