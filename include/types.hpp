#ifndef TYPES_HPP
# define TYPES_HPP

# include <string>
# include <vector>

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

#endif