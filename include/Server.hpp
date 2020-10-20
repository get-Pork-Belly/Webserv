#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <string>
# include <map>
# include <vector>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <algorithm>
# include <vector>
# include <fcntl.h>
# include "types.hpp"
# include "Request.hpp"
# include "Response.hpp"

const int BUFFER_SIZE = 1;

class ServerManager;
class Request;

class Server
{
private:
    Server();
    Server(const Server& other);
    Server& operator=(const Server& rhs);

private:
    std::map<std::string, std::string> _server_config;
    // int _fd; 
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
    struct sockaddr_in _server_address;
    std::vector<Response> _response;

public:
    /* Constructor */
    Server(server_info& server_config, std::map<std::string, location_info> location_config);
        /* Destructor */
    virtual ~Server();

    /* Overload */
    /* Getter */
    //TODO: 구현
    // Request getRequest();
    const std::map<std::string, std::string> getServerConfig();
    int getServerSocket();
    /* Setter */
    void setServerSocket();
    /* Exception */
    /* Util */

    /* Server function */
    void init();
    void run(ServerManager *server_manager, int fd);
    Request receiveRequest(int fd);
    void makeResponse(Request& request, int fd);
    bool sendResponse(int fd);
    bool isClientOfServer(int fd);
};

#endif
