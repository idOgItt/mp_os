#include "server_logger.h"

#ifdef _WIN32
std::map<std::string, std::pair<HANDLE, size_t>> server_logger::queues_users = std::map<std::string, std::pair<HANDLE, size_t>>();
#else
std::map<std::string, std::pair<mqd_t, size_t>> server_logger::queues_users = std::map<std::string, std::pair<mqd_t, size_t>>();
#endif

server_logger::server_logger(server_logger const &other) = default;

server_logger::server_logger(std::map<std::string, std::set<logger::severity>> const keys) {
    session_id = 0;
    for (auto it = keys.begin(); it != keys.end(); ++it) {
        auto &key = it->first;
        auto &set = it->second;
        if (queues_users.find(key) == queues_users.end()) {
#ifdef _WIN32
            HANDLE q = CreateFileA(key.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (q == INVALID_HANDLE_VALUE) {
                throw std::runtime_error("Can`t open queue");
            }

#else
            mqd_t q = mq_open(key.c_str(), O_WRONLY, 0644);
            if (q == (mqd_t) - 1) {
                throw std::runtime_error("Can`t open queue");
            }

#endif
            queues_users[key].first = q;
            queues_users[key].second = 1;
            queues[key].first = q;

        } else {
            queues_users[key].second++;
            queues[key].first = queues_users[key].first;
        }
        queues[key].second = set;
    }
#ifdef _WIN32
    process_id = GetCurrentProcessId();
#else
    process_id = getpid();
#endif
    session_id = 0;
}

server_logger &server_logger::operator=(server_logger const &other) = default;

server_logger::server_logger(server_logger &&other) noexcept = default;

server_logger &server_logger::operator=(server_logger &&other) noexcept = default;

server_logger::~server_logger() noexcept {
    for (auto it = queues.begin(); it != queues.end(); ++it) {
        auto &key = it->first;
        auto &pr = it->second;
        if (--queues_users[key].second == 0) {
#ifdef _WIN32
            CloseHandle(pr.first);
#else
            mq_close(pr.first);
#endif
            queues_users.erase(key);
        }
    }
}


logger const *server_logger::log(const std::string &text, logger::severity severity) const noexcept {
    size_t meta_size = sizeof(bool) + sizeof(size_t) + sizeof(pid_t);
    size_t msg_size = MESSAGE_SIZE - meta_size;
    size_t msg_count = text.size() / msg_size + 1;
    for (auto it = queues.begin(); it != queues.end(); ++it) {
        auto &path = it->first;
        auto &pr = it->second;
        if (pr.second.find(severity) == pr.second.end()) continue;
        const char* severity_str = severity_to_string(severity).c_str();
        char buffer[MESSAGE_SIZE];
        char* ptr = buffer;

        *reinterpret_cast<bool*>(ptr) = false;
        ptr += sizeof(bool);
        *reinterpret_cast<pid_t*>(ptr) = process_id;
        ptr += sizeof(pid_t);
        *reinterpret_cast<size_t*>(ptr) = session_id;
        ptr += sizeof(size_t);
        // Преобразование msg_count в сетевой порядок байтов
        size_t network_msg_count = htonl(msg_count);
        memcpy(ptr, &network_msg_count, sizeof(size_t));
        ptr += sizeof(size_t);

        strcpy(ptr, severity_str);
        ptr += strlen(severity_str) + 1;

#ifdef _WIN32
        DWORD bytes_count;
        WriteFile(pr.first, buffer, ptr - buffer, &bytes_count, nullptr);
#else
        mq_send(pr.first, buffer, ptr - buffer, 0);
#endif

        for (size_t i = 0; i < msg_count; i++) {
            char tmp_msg[MESSAGE_SIZE];
            char* ptr_tmp = tmp_msg;
            *reinterpret_cast<bool*>(ptr_tmp) = true;
            ptr_tmp += sizeof(bool);
            *reinterpret_cast<pid_t*>(ptr_tmp) = process_id;
            ptr_tmp += sizeof(pid_t);
            *reinterpret_cast<size_t*>(ptr_tmp) = session_id;
            ptr_tmp += sizeof(size_t);

            size_t position = msg_size * i;
            size_t left_size = text.size() - position;
            size_t substr_size;
            if (left_size < msg_size) {
                substr_size = left_size;
            } else {
                substr_size = msg_size;
            }

            memcpy(ptr_tmp, text.substr(position, substr_size).c_str(), substr_size);
            ptr_tmp += substr_size;
            *ptr_tmp = '\0';

#ifdef _WIN32
            WriteFile(pr.first, tmp_msg, ptr_tmp - tmp_msg, &bytes_count, nullptr);
#else
            mq_send(pr.first, tmp_msg, ptr_tmp - tmp_msg, 0);
#endif
        }
    }
    session_id++;
    return this;
}
