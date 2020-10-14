#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <map>
# include <vector>

class Server
{
private:
    Server(const Server& other);
    Server& operator=(const Server& rhs);

private:
    Server() {}; //쓰지않을 예정
    std::map<std::string, std::string> _server_config;
    int _server_socket;
    std::vector<int> _client_sockets;
    std::string _server_name;
    std::string _host;
    std::string _port;
    int _status_code;
    int _request_uri_limit_size;	
    int _request_header_limit_size;
    int _limit_client_body_size; 
    std::string _default_error_page;

public:
    /* Constructor */
    Server(std::map<std::string, std::string>& server_config);
        /* Destructor */
    virtual ~Server();

    /* Overload */
    /* Getter */
    //TODO: 구현
    // Request getRequest();
    const std::map<std::string, std::string> getServerConfig();
    int getServerSocket();
    /* Setter */
    /* Exception */
    /* Util */
    //TODO: 구현
    // bool isValidRequest(Request);
    // Response makeResponse(Request);
};

#endif