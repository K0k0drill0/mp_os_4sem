#include <cmath>
#include <string.h>
#include <set>
#include <map>
#include <unistd.h>

#ifdef __linux__
#include <sys/msg.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <not_implemented.h>

#include "../include/server_logger.h"

std::mutex server_logger::mutex;

server_logger::server_logger(std::map<std::string, std::set<logger::severity>> const &log_dest) {
    #ifdef __linux__
    _mq_descriptor = msgget(LINUX_MSG_QUEUE_KEY, 0666);
	
	if (_mq_descriptor == -1)
	{
        throw std::runtime_error("Cannot connect to the server");
	}
    #endif
    
    #ifdef _WIN32
    _hFile = CreateFile(WIN32_MAILSLOT_NAME, GENERIC_WRITE, FILE_SHARE_READ,
            (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
    
    if (_hFile == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("Cannot connect to the server");
    }
    #endif
}

logger const *server_logger::log(
    const std::string &text, logger::severity severity) const noexcept {

    for (auto &[file_name, severities] : _configuration) {
        if (severities.count(severity)) {
            mutex.lock();

            msg_t msg;
            msg.mtype = LOG_PRIOR;
            msg.pid = getpid();
            msg.packet_cnt = std::ceil(1.0 * text.size() / MAX_MSG_TEXT_SIZE);
            msg.severity = static_cast<int>(severity);

            strcpy(msg.file_path, file_name.c_str());

            for (size_t i = 0; i < msg.packet_cnt; i++) {
                msg.packet_id = i + 1;

                strcpy(msg.mtext, text.substr(i * MAX_MSG_TEXT_SIZE, MAX_MSG_TEXT_SIZE).c_str());

                #ifdef __linux__
	            msgsnd(_mq_descriptor, &msg, sizeof(msg_t), 0);
                #endif
                
                #ifdef _WIN32
                WriteFile(_hFile, &msg, sizeof(msg_t), (LPDWORD) NULL, (LPOVERLAPPED) NULL);
                #endif
            }
            mutex.unlock();
        }
    }

    return this;
}