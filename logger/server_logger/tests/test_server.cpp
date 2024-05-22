#include <fstream>
#include <string>
#include <map>
#include <thread>
#include <iostream>

#define LINUX_MSG_QUEUE_KEY 100
#define WIN32_MAILSLOT_NAME "\\\\.\\mailslot\\mp_os_srvr_lgr"
#define MAX_MSG_TEXT_SIZE 1024
#define SHUTDOWN_PRIOR 1
#define LOG_PRIOR 2

struct msg_t
{
    long mtype;
    pid_t pid;
    size_t packet_id;
    size_t packet_cnt;
    char file_path[256];
    int severity;
    char mtext[MAX_MSG_TEXT_SIZE];
};

int run_flag = 1;

#ifdef __linux__

#include <sys/msg.h>

int mq_descriptor = -1;

#endif

#ifdef _WIN32

#include <windows.h>

bool read_win32_slot(HANDLE hSlot, msg_t &msg);

#endif

std::string decode_severity(int severity_code);
void terminal_reader();

int main() {
    msg_t msg;
    size_t const max_msg_size = sizeof(msg_t);
    std::map<std::string, std::ofstream> streams;
    
    #ifdef __linux__
    mq_descriptor = msgget(LINUX_MSG_QUEUE_KEY, IPC_CREAT | 0666);
    if (mq_descriptor == -1)
    {
        std::cout << "Cannot create the queue. Shut down." << std::endl;
        return 1;
    }
    #endif
    
    #ifdef _WIN32
    HANDLE hSlot = CreateMailslot(WIN32_MAILSLOT_NAME, sizeof(msg_t),
            MAILSLOT_WAIT_FOREVER, (LPSECURITY_ATTRIBUTES) NULL);
    
    if (hSlot == INVALID_HANDLE_VALUE) 
    {
        std::cout << "Cannot create the mail slot. Shut down." << std::endl;
        return 1;
    }
    #endif

    std::thread cmd_thread(terminal_reader);

    while (run_flag) {
        #ifdef __linux__
        ssize_t rcv_cnt = msgrcv(mq_descriptor, &msg, max_msg_size, 0, MSG_NOERROR);
        if (rcv_cnt == -1) {
            std::cout << "An error occured while receiving the message" << std::endl;
        }

        break;
        #endif

        #ifdef _WIN32

        bool read_flag = false;
        
        try
        {
            read_flag = read_win32_slot(hSlot, msg);
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
        
        if (!read_flag)
        {
            Sleep(200);
            continue;
        }

        #endif

        if (msg.mtype == SHUTDOWN_PRIOR)
        {
            break;
        }
        
        if (msg.file_path[0] && !streams.count(msg.file_path))
        {	
            std::ofstream fstream = std::ofstream(msg.file_path, std::ios::app);
            
            if (!fstream.is_open())
            {
                std::cerr << std::runtime_error("File can not be opened").what() << std::endl;
            }
            
            streams[msg.file_path] = std::move(fstream);
        }
        
        if (msg.packet_cnt == 1)
        {
            std::ostream *log_stream_ptr = &std::cout;
            if (msg.file_path[0])
            {
                log_stream_ptr = &streams[msg.file_path];
            }
            
            std::ostream &log_stream = *log_stream_ptr;
            std::string severity = decode_severity(msg.severity);
            
            log_stream << "[" << severity << "] " << msg.mtext << std::endl;
            log_stream.flush();
        }
        else
        {
            std::string tmp_file = "tmp_file_" + std::to_string(msg.pid);
            
            std::ofstream &stream = streams[tmp_file];
            
            if (msg.packet_id == 1)
            {
                stream.close();
                stream.open(tmp_file, std::ios::out | std::ios::trunc);
            }
            
            stream << msg.mtext;
            
            if (msg.packet_id == msg.packet_cnt)
            {
                stream.flush();
                
                std::ifstream istream(tmp_file);
                
                if (istream.is_open())
                {
                    std::ostream *log_stream_ptr = &std::cout;
                    if (msg.file_path[0])
                    {
                        log_stream_ptr = &streams[msg.file_path];
                    }
                    
                    std::ostream &log_stream = *log_stream_ptr;
                    std::string severity = decode_severity(msg.severity);
                    
                    log_stream << "[" << severity << "] ";
                    
                    char ch;
                    while (istream >> std::noskipws >> ch)
                    {
                        log_stream << ch;
                    }
                    
                    log_stream << std::endl;
                    log_stream.flush();
                    
                    istream.close();
                }
            }
        }
    }
    
    for (auto &record : streams)
    {
        std::ofstream &stream = record.second;
        stream.flush();
        stream.close();
    }
    
    #ifdef __linux__
    msgctl(mq_descriptor, IPC_RMID, nullptr);
    #endif
    
    #ifdef _WIN32
    CloseHandle(hSlot);
    #endif
    
    std::cout << "Server shut down" << std::endl;
    
    cmd_thread.detach();
}

std::string decode_severity(int severity_code)
{
    switch (severity_code)
    {
    case 0:
        return "TRACE";
    case 1:
        return "DEBUG";
    case 2:
        return "INFORMATION";
    case 3:
        return "WARNING";
    case 4:
        return "ERROR";
    case 5:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}

void terminal_reader()
{
    msg_t msg;
    std::string cmd;
    
    while (std::cin >> cmd)
    {
        if (cmd == "shutdown")
        {
            msg.mtype = SHUTDOWN_PRIOR;
            
            #ifdef __linux__
            msgsnd(mq_descriptor, &msg, sizeof(msg_t), 0);
            #endif
            
            run_flag = 0;
            
            break;
        }
    }
}

#ifdef _WIN32
bool read_win32_slot(HANDLE hSlot, msg_t &msg)
{
    DWORD msg_cnt, next_msg_size;
    bool fResult; 
    HANDLE hEvent;
    OVERLAPPED ov;
    
    hEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("mp_os_srvr_lgr_slot"));
    if(hEvent == NULL)
    {
        throw std::runtime_error("Cannot create event for overlapped read");
    }
    
    ov.Offset = 0;
    ov.OffsetHigh = 0;
    ov.hEvent = hEvent;
    
    // slot descriptor, max msg size, next msg size, msg cnt, read timeout
    fResult = GetMailslotInfo(hSlot, (LPDWORD) NULL, &next_msg_size, &msg_cnt, (LPDWORD) NULL);
    
    if (!fResult)
    {
        CloseHandle(hEvent);
        throw std::runtime_error("An error occur while getting mail slot info");
    }
    
    if (msg_cnt == 0)
    {
        CloseHandle(hEvent);
        return false;
    }
    
    // slot descriptor, buffer, size, byte cnt, overlapped obj
    fResult = ReadFile(hSlot, &msg, next_msg_size, (LPDWORD) NULL, &ov);
    
    if (!fResult) 
    {
        CloseHandle(hEvent);
        throw std::runtime_error("An error occur whilee reading the mail slot");
    } 
    
    if (next_msg_size != sizeof(msg_t))
    {
        CloseHandle(hEvent);
        throw std::runtime_error("Message of wrong size was read from mail slot");
    }
    
    CloseHandle(hEvent);
    return true;
}
#endif
