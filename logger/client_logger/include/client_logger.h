#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_CLIENT_LOGGER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_CLIENT_LOGGER_H

#include <logger.h>
#include <map>
#include <unordered_map>
#include <set>
#include "client_logger_builder.h"

class client_logger final:
    public logger
{

private:
    static std::unordered_map<std::string, std::pair<std::ostream *, int>> _streams_users;
    std::string _format_string;
    std::map<std::string, std::pair<std::ostream *, std::set<logger::severity>>> _streams;

public:
    client_logger(
        std::string const &format_string, 
        std::map<std::string, std::set<logger::severity>> const &configuration);

public:

    client_logger(
        client_logger const &other);

    client_logger &operator=(
        client_logger const &other);

    client_logger(
        client_logger &&other) noexcept;

    client_logger &operator=(
        client_logger &&other) noexcept;

    ~client_logger() noexcept final;

public:

    logger::severity string_to_severity(std::string);

    [[nodiscard]] logger const *log(
        const std::string &message,
        logger::severity severity) const noexcept override;

private:
    void decrement_stream_users(std::string const &file_path) const noexcept;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_CLIENT_LOGGER_H