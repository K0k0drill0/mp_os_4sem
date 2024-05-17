#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H

#include <logger.h>
#include "server_logger_builder.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <set>
#include <map>

// #define LINUX_MSG_QUEUE_KEY 100
#define MAX_MSG_TEXT_SIZE 1024
// #define LOG_PRIOR 2

class server_logger final:
    public logger
{

friend class server_logger_builder;

private:
    // struct msg_t {
    //     long type;
    //     pid_t pid;
    //     size_t packet_id;
    //     size_t packet_cnt;
    //     char file_path[256];
    //     int severity;
    //     char text[MAX_MSG_TEXT_SIZE];
    // };
    pid_t _process_id;

    size_t mutable _request;

private:
    #ifdef __linux__
        std::map<std::string, std::pair<mqd_t, std::set<logger::severity>>> _queues;

        static std::map<std::string, std::pair<mqd_t, unsigned int>> _queues_users;
    #endif

    #ifdef _WIN32
        std::map<std::string, std::pair<HANDLE, std::set<logger::severity>>> _queues;

        static std::map<std::string, std::pair<HANDLE, unsigned int>> _queues_users;
    #endif

    // std::map<std::string, std::set<logger::severity>> _configuration;

    void close_streams();

private:
    server_logger(std::map<std::string, std::set<logger::severity>> const configuration);

public:

    server_logger(
        server_logger const &other);

    server_logger &operator=(
        server_logger const &other);

    server_logger(
        server_logger &&other) noexcept;

    server_logger &operator=(
        server_logger &&other) noexcept;

    ~server_logger() noexcept final;

public:

    [[nodiscard]] logger const *log(
        const std::string &message,
        logger::severity severity) const noexcept override;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H