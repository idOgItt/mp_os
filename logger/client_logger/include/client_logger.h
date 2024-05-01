#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_CLIENT_LOGGER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_CLIENT_LOGGER_H


#include <map>
#include <set>
#include <fstream>
#include <logger.h>
#include "client_logger_builder.h"

class client_logger final:
    public logger
{
private:

    std::map<std::string, std::set<logger::severity>> streams;
    static std::map<std::string, std::pair<std::ofstream, size_t>> user_streams;

    std::string _log_structure;
    size_t size_struct_of_log;

public:

    client_logger(
        client_logger const &other);

    client_logger &operator=(
        client_logger const &other);

    client_logger(
        const std::map<std::string, std::set<logger::severity>> &_paths, const std::string &_log_structure);

    void message_format(
        std::string &to_format, const std::string &flag, const std::string &replace_with) const noexcept;

    void end_streams();

    client_logger(
        client_logger &&other) noexcept;

    client_logger &operator=(
        client_logger &&other) noexcept;

    ~client_logger() noexcept final;

public:

    [[nodiscard]] logger const *log(
        const std::string &message,
        logger::severity severity) const noexcept override;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_CLIENT_LOGGER_H