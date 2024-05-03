#include <fstream> 
#include <iostream> 
#include <sstream> 

#include "../include/client_logger_builder.h"

client_logger_builder::client_logger_builder() : _log_structure{"%d %t %s %m\n"} {}

client_logger_builder::client_logger_builder(
    client_logger_builder const &other) = default;

client_logger_builder &client_logger_builder::operator=(
    client_logger_builder const &other)
{
    if (this == &other) return *this;
    _paths = other._paths;
    _log_structure = other._log_structure;
    return *this;
}

client_logger_builder::client_logger_builder(
    client_logger_builder &&other) noexcept = default;

client_logger_builder &client_logger_builder::operator=(
    client_logger_builder &&other) noexcept
{
    if (this == &other) return *this;
    _log_structure = std::move(other._log_structure);
    _paths = std::move(other._paths);
    return *this;
}

client_logger_builder::~client_logger_builder() noexcept = default;

logger_builder *client_logger_builder::add_file_stream(
    std::string const &stream_file_path,
    logger::severity severity)
{
    char new_path[456];
    realpath(stream_file_path.c_str(), new_path);
    std::string new_path_str = new_path;

    _paths[new_path_str].insert(severity);
    return this;
}

logger_builder *client_logger_builder::add_console_stream(
    logger::severity severity)
{
    _paths["/console"].insert(severity);
    return this;
}

logger_builder* client_logger_builder::transform_with_configuration(
    std::string const &configuration_file_path,
    std::string const &configuration_path)
{
    // JSon
    std::runtime_error file_error("Configuration file doesn't exist\n");
    std::runtime_error configuration_error("Can't find configuration path\n");
    nlohmann::json configuration;
    std::ifstream configuration_file(configuration_file_path, std::ios::binary);
    if (configuration_file.is_open() == false) throw file_error;
    if (configuration_file.peek() == EOF) throw configuration_error;

    configuration_file >> configuration;
    if (configuration.find(configuration_path) == configuration.end()) throw file_error;

    std::string filename;
    std::string severity_string;
    logger::severity severity_logger;

    for (auto &file : configuration[configuration_path]) {
        filename = file[0];
        for (auto &severity : file[1]) {
            severity_string = severity;
            severity_logger = string_to_severity(severity_string);
            _paths[filename].insert(severity_logger);
        }
    }
    return this;
}

logger_builder * client_logger_builder::change_log_structure(const std::string &log_structure) noexcept {
    _log_structure = log_structure;
    return this;
}

logger_builder *client_logger_builder::clear()
{
    _paths.clear();
    return this;
}

logger *client_logger_builder::build() const
{
    return new client_logger(_paths, _log_structure);
}