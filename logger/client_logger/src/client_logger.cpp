#include <not_implemented.h>
#include <fstream>
#include <unordered_map>

#include "../include/client_logger.h"

std::unordered_map<std::string, std::pair<std::ostream *, int>> client_logger::_streams_users;

client_logger::client_logger(
    client_logger const &other) : _format_string(other._format_string), _streams(other._streams) 
{
    for (auto record: _streams) {
        _streams_users[record.first].second++;
    }
}

client_logger &client_logger::operator=(
    client_logger const &other) 
{
    if (this == &other) {
        return *this;
    }

    for (auto record : _streams)
    {
        decrement_stream_users(record.first);
    }

    _format_string = other._format_string;
    _streams = other._streams;

    for (auto record: _streams) {
        _streams_users[record.first].second++;
    }

    return *this;
}

client_logger::client_logger(
    client_logger &&other) noexcept : _format_string(std::move(other._format_string)), _streams(std::move(other._streams))
{ }

client_logger &client_logger::operator=(
    client_logger &&other) noexcept
{
    if (this == &other) {
        return *this;
    }

    for (auto iter : _streams) {
        decrement_stream_users(iter.first);
    }

    _format_string = std::move(other._format_string);
    _streams = std::move(other._streams);

    return *this;
}

client_logger::~client_logger() noexcept
{
    for (auto record : _streams) {
        decrement_stream_users(record.first);
    }
}

void replace_substring(std::string &str, const std::string &to_replace, const std::string &replace_with)
{
    size_t pos = 0;
    while ((pos = str.find(to_replace, pos)) != std::string::npos)
    {
        str.replace(pos, to_replace.size(), replace_with);
        pos += replace_with.size();
    }
}

void format_to_message(std::string &replaced_format, const std::string &message, const std::string &string_severity) {
    time_t time_now = time(NULL);
    char tmp_date_time[20];

    std::string msg = "%m";
    std::string sev = "%s";
    std::string dt = "%d";
    std::string tm = "%t";

    replace_substring(replaced_format, msg, message);
    replace_substring(replaced_format, sev, string_severity);
    strftime(tmp_date_time, sizeof(tmp_date_time), "%T", localtime(&time_now));
    replace_substring(replaced_format, tm, tmp_date_time);
    strftime(tmp_date_time, sizeof(tmp_date_time), "%F", localtime(&time_now));
    replace_substring(replaced_format, dt, tmp_date_time);

}

logger const *client_logger::log(
    const std::string &text,
    logger::severity severity) const noexcept
{
    std::string string_severity = severity_to_string(severity);
    std::string message = _format_string;
    format_to_message(message, text, string_severity);

    for (auto iter : _streams) {
        std::ostream &stream = *(iter.second.first);
        auto &severities = iter.second.second;

        if (severities.count(severity)) {
            stream << message << std::endl;
        }
    }

    return this;
}


client_logger::client_logger(
        std::string const &format_string, 
        std::map<std::string, std::set<logger::severity>> const &configuration) : _format_string(format_string) {
    for (auto iter = configuration.begin(); iter != configuration.end(); iter++) {
        std::string file_path = iter->first;
        std::set<logger::severity> severities = iter->second;

        if (file_path.size() == 0) {
            _streams[""] = std::make_pair(&std::cout, severities);
            continue; 
        }

        if (!_streams_users.count(file_path)) {
            std::ofstream *stream = nullptr;

            try {
                stream = new std::ofstream(file_path);
                // std::cout << file_path << std::endl;
                if (!stream->is_open()) {
                    throw std::runtime_error("File " + file_path + " can not be opened");
                }
            }
            catch (const std::exception&) {
                delete stream;

                for (auto del_iter = configuration.begin(); del_iter != iter; del_iter++) {
                    decrement_stream_users(del_iter->first);
                }

                throw;
            } 

            _streams_users[file_path] = std::make_pair(stream, 0);
        }
        _streams[file_path] = std::make_pair(_streams_users[file_path].first, severities);
        _streams_users[file_path].second++;
    }
}

void client_logger::decrement_stream_users(std::string const &file_path) const noexcept {
    if (file_path.size() == 0) {
        return;
    }

    auto iter = _streams_users.find(file_path);

    auto &stream = iter->second.first;
    auto &counter = iter->second.second;

    counter--;

    if (counter == 0) {
        stream->flush();
        delete stream;
        _streams_users.erase(iter);
    }
}