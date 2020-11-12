#ifndef TYPES_HPP
# define TYPES_HPP

# include <string>
# include <vector>
# include <map>

using location_info = std::map<std::string, std::string>;
using server_info = std::map<std::string, std::string>;

//NOTE 임시
typedef struct GlobalConfig
{
    std::string _http_version; 
    std::string _os;
    std::string _cgi_version;
} GlobalConfig;

typedef struct ServerConfig
{
    std::string _server_name;
    std::string _host;
    std::string _port;
    int _request_uri_limit_size;
    int _request_header_limit_size;
    int _limit_client_body_size;
    std::string _default_error_page;
    std::vector<struct Location *> _locations;
    struct GlobalConfig *global_config; //NOTE: ServerManager 객체의 멤버변수의 주소
} ServerConfig;

typedef struct Location
{
    std::string _uri;
    std::string _root_path;
    std::string _allow_method;
    std::string _auth_basic_realm;
    std::string _auth_basic_file;
    std::string _index;
    std::string _cgi;
    std::string _autoindex;
} Location;

enum class FdSet
{
    ALL,
    READ,
    WRITE,
    EXCEPT,
};

enum class FdType
{
    SERVER_SOCKET,
    CLIENT_SOCKET,
    RESOURCE,
    PIPE,
    CLOSED,
};

enum class ReqInfo
{
    READY,
    COMPLETE,
    NORMAL_BODY,
    CHUNKED_BODY,
    MUST_CLEAR,
};

enum class ResType
{
    NOT_YET_CHECKED,
    STATIC_RESOURCE,
    CGI,
    AUTO_INDEX,
    INDEX_HTML,
    ERROR_HTML,
};

enum class SendProgress
{
    DEFAULT,
    CHUNK_START,
    CHUNK_PROGRESS,
    FINISH,
};

enum class OnRead
{
    READING,
    COMPLETE,
};

#endif
