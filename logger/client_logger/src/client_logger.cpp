#include <ctime> 
#include <fstream> 
#include <iostream> 
#include <sstream> 
#include "../include/client_logger.h"

std::map<std::string, std::pair<std::ofstream, size_t>> client_logger::user_streams =  std::map<std::string, std::pair<std::ofstream, size_t>>();


client_logger::client_logger(client_logger const &other) 
    : _log_structure(other._log_structure),
    size_struct_of_log(other.size_struct_of_log),
    streams(other.streams) 
{
    for (auto it = streams.begin(); it != streams.end(); ++it) {
        auto& path = it->first;
        auto& severities = it->second;
        user_streams[path].second++;
    }
}

void client_logger::end_streams() 
{
    for (auto it = streams.begin(); it != streams.end(); ++it) {
        auto& path = it->first;
        auto& severities = it->second;
        if (!(--user_streams[path].second)) {
            user_streams[path].first.close();
            std::cout << path << "\n" << "File is closed \n";
            user_streams.erase(path);
        }
    }
}


client_logger::client_logger(
    client_logger &&other) noexcept
    :streams(std::move(other.streams)),
    _log_structure(std::move(other._log_structure)),
    size_struct_of_log(std::move(other.size_struct_of_log)) {}

client_logger &client_logger::operator=(
    client_logger &&other) noexcept {
    if (this == &other) return *this;

    end_streams();

    streams = std::move(other.streams);
    _log_structure = std::move(other._log_structure);
    size_struct_of_log = std::move(other.size_struct_of_log);

    return *this;
}

client_logger::~client_logger() noexcept
{
    std::cout << "Destructor\n";
    end_streams();
}

client_logger::client_logger(
    const std::map<std::string, std::set<logger::severity>> &paths, const std::string &struct_of_log)
: _log_structure(struct_of_log), size_struct_of_log(struct_of_log.size())
{
    std::cout << "Client logger builder\n";
    for (auto it = paths.begin(); it != paths.end(); ++it) {
        std::cout << it->first << "Iterator \n";
        auto& path = it->first;
        auto& severities = it->second;
        if (user_streams.find(path) == user_streams.end()) {
            if (path != "/console") {
                user_streams[path].first.open(path);
                std::cout << path <<"File is opened\n";
                if (!user_streams[path].first.is_open()) {
                    throw std::runtime_error("Unable to open " + path);
                }
            }
            user_streams[path].second = 1;
        } else  {
            user_streams[path].second++;
        }
        streams[path] = severities;
    }
    std::cout << "Client logger end \n";
}

client_logger &client_logger::operator=(client_logger const &other) {
    if (this == &other) return *this;
    end_streams();
    streams = other.streams;
    _log_structure = other._log_structure;
    size_struct_of_log = other.size_struct_of_log;

    for (auto it = streams.begin(); it != streams.end(); ++it) {
        auto& path = it->first;
        auto& pair = it->second;
        user_streams[path].second++;
    }
    return *this;
}

void client_logger::message_format(std::string &to_format, std::string const &flag, std::string const &replace) const noexcept {
    for (auto pos = 0; pos != std::string::npos;){
        pos = to_format.find(flag, pos);
        if (pos != std::string::npos) {
            to_format.replace(pos, flag.size(), replace);
            pos += replace.size();
        }
    }
}

logger const *client_logger::log(
    const std::string &text,
    logger::severity severity) const noexcept
{    
    // Log file
    std::string datetime_str = current_datetime_to_string();
    auto sep = datetime_str.find(' ');
    std::string date = datetime_str.substr(0, sep);
    std::string time = datetime_str.substr(sep);
    for (auto it = streams.begin(); it != streams.end(); ++it) {
        auto& path = it->first;
        auto& severities = it->second;
        if (severities.find(severity) == severities.end()) continue;

        std::string message_log = _log_structure;
        message_format(message_log, "%d", date);
        message_format(message_log, "%t", time);
        message_format(message_log, "%s", severity_to_string(severity));
        message_format(message_log, "%m", text);
        if (user_streams[path].first.is_open()) {
            user_streams[path].first << message_log;
        } else {
            std::cout << message_log;
        }

    }
    return this;
     
}