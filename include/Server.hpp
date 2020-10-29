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

const int BUFFER_SIZE = 8192;

class ServerManager;
class Request;

class Server
{
private:
    Server();
    Server(const Server& other);
    Server& operator=(const Server& rhs);

private:
    ServerManager* _server_manager;
    std::map<std::string, std::string> _server_config;
    int _server_socket;
    std::string _server_name;
    std::string _host;
    std::string _port;
    int _request_uri_limit_size;
    int _request_header_limit_size;
    int _limit_client_body_size;
    std::string _default_error_page;
    struct sockaddr_in _server_address;
    std::vector<Request> _requests;
    std::map<std::string, location_info> _location_config;
    std::vector<Response> _responses;

public:
    /* Constructor */
    Server(ServerManager* server_manager, server_info& server_config, 
                        std::map<std::string, location_info>& location_config);
    /* Destructor */
    virtual ~Server();

    /* Overload */
    /* Getter */
    //TODO: 구현
    // Request getRequest();
    const std::map<std::string, std::string> getServerConfig();
    const std::map<std::string, location_info>& getLocationConfig();
    int getServerSocket() const;
    Request& getRequest(int fd);
    /* Setter */
    void setServerSocket();
    /* Exception */
    /* Util */
    bool closeClientSocket(int fd);
    bool isFdManagedByServer(int fd) const;
    bool isServerSocket(int fd) const;
    bool isClientSocket(int fd) const;
    bool isStaticResource(int fd) const;
    bool isCGIPipe(int fd) const;

    /* Server function */
    void init();
    void run(int fd);
    void receiveRequest(int fd);
    void receiveRequestWithoutBody(int fd);
    void readBufferUntilHeaders(int fd, char* buf, size_t header_end_pos);
    void receiveRequestNormalBody(int fd);
    void receiveRequestChunkedBody(int fd);
    void clearRequestBuffer(int fd);
    std::string makeResponseMessage(Request& request);
    bool sendResponse(std::string& response_meesage, int fd);
    bool isClientOfServer(int fd) const;

public:
public:
public:
public:
public:
    class PayloadTooLargeException : public std::exception
    {
    private:
        Request& _request;
    public:
        PayloadTooLargeException(Request& request);
        virtual const char* what() const throw();
    };
    class ReadErrorException : public std::exception
    {
    public:
        virtual const char* what() const throw();
    };

// public:
//     class ResponseException : public std::exception
//     {
//     public:
//         virtual const char* what() const throw();
//     };
};

#endif
