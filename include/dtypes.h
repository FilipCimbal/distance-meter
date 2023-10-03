#ifndef D_TYPES
#define D_TYPES

#include <nlohmann/json.hpp>
#include <vector>
#include <string>

using json = nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, float>;

enum class ResponseError
{
    OK = 200,
    ERROR = 450,
    BAD_ARGUMENT = 451,
    TIMEOUT = 452
};

NLOHMANN_JSON_SERIALIZE_ENUM(ResponseError, {
                                                {ResponseError::OK, "OK"},
                                                {ResponseError::ERROR, "ERROR"},
                                                {ResponseError::BAD_ARGUMENT, "BAD_ARGUMENT"},
                                                {ResponseError::TIMEOUT, "TIMEOUT"},
                                            })

enum class SystemError
{
    NONE,
    UART_BUS,
    RS485_BUS,
    WIFI,
    UNKNOWN
};

NLOHMANN_JSON_SERIALIZE_ENUM(SystemError, {
                                              {SystemError::NONE, "NONE"},
                                              {SystemError::UART_BUS, "UART_BUS"},
                                              {SystemError::RS485_BUS, "RS485_BUS"},
                                              {SystemError::WIFI, "WIFI"},
                                              {SystemError::UNKNOWN, "UNKNOWN"},
                                          })


struct ResponseApi
{
    uint32_t id = 0;
    ResponseError code = ResponseError::OK;
    std::string message = "OK";
    json args;
};

inline void to_json(json &j, const ResponseApi &tVar)
{
    j = json{
        {"id", tVar.id},
        {"code", tVar.code},
        {"message", tVar.message},
        {"args", tVar.args},
    };
}

inline void from_json(const json &j, ResponseApi &tVar)
{
    if (j.contains("id"))
    {
        j.at("id").get_to(tVar.id);
    }
    if (j.contains("code"))
    {
        j.at("code").get_to(tVar.code);
    }
    if (j.contains("message"))
    {
        j.at("message").get_to(tVar.message);
    }
    if (j.contains("args"))
    {
        j.at("args").get_to(tVar.args);
    }
}

#endif /* D_TYPES */