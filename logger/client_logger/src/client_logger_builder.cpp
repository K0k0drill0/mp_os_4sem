#include <vector>
#include <fstream>
#include <filesystem>

#include <not_implemented.h>
#include <client_logger.h>

#include <nlohmann/json.hpp>


#include "../include/client_logger_builder.h"

client_logger_builder::client_logger_builder() : _format_string("[%s] %m\n") { }

client_logger_builder::client_logger_builder(
    std::string const &format_string) : _format_string(format_string)
{ }

client_logger_builder &client_logger_builder::operator=(
    client_logger_builder const &other)
{
    if (this == &other) return *this;
    _format_string = other._format_string;
    _configuration = other._configuration;
    return *this;
}

client_logger_builder::client_logger_builder(
    client_logger_builder &&other) noexcept :
    _configuration(std::move(other._configuration)), _format_string(std::move(other._format_string))
{ }

client_logger_builder &client_logger_builder::operator=(
    client_logger_builder &&other) noexcept
{
    if (this == &other) return *this;
    _format_string = std::move(other._format_string);
    _configuration = std::move(other._configuration);

    return *this;
}

client_logger_builder::~client_logger_builder() noexcept
{ }

logger_builder *client_logger_builder::add_file_stream(
    std::string const &stream_file_path,
    logger::severity severity)
{
    std::string abs_path = std::filesystem::weakly_canonical(stream_file_path).string();
    _configuration[stream_file_path].insert(severity);
    return this;
}

logger_builder *client_logger_builder::add_console_stream(
    logger::severity severity)
{   
    _configuration[""].insert(severity);

    return this;
}

logger_builder* client_logger_builder::transform_with_configuration(
    std::string const &configuration_file_path,
    std::string const &configuration_path)
{
    std::runtime_error nonexistent_file("Configuration file does not exist.\n");
    std::runtime_error empty_file("Can not find configuration path.\n");

    std::ifstream stream(configuration_file_path);

    if (!stream.is_open()) {
        throw nonexistent_file;
    }

    std::vector<std::string> data_path_components;

    for (int i = 0; i < configuration_file_path.size(); i++) {
        int tmp_ind = std::min(configuration_path.find(':', i), configuration_file_path.size());

        std::string component = configuration_path.substr(i, tmp_ind - i);
        if (component.size() == 0) {
            throw std::runtime_error("Invalid JSON data path");
        }

        data_path_components.push_back(std::move(component));
        i = tmp_ind + 1;
    }

    nlohmann::json json_obj = nlohmann::json::parse(stream);

    for (auto path_elem: data_path_components) {
        json_obj = json_obj[path_elem];
    }

    clear();

    _format_string = json_obj["format_string"];
    json_obj = json_obj["logger_files"];

    for (auto &[file_path, severities] : json_obj.items()) {
        for (std::string severity_str : severities) {
            logger::severity severity = string_to_severity(severity_str);

            if (file_path == "console") {
                add_console_stream(severity);
            }
            else {
                add_file_stream(file_path, severity);
            }
        }
    }

    return this;
}

logger_builder *client_logger_builder::clear()
{
    _format_string = "[%s]: %m";
    _configuration.clear();

    return this;
}

logger *client_logger_builder::build() const
{
    return new client_logger(_format_string, _configuration);
}