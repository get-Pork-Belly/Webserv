#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <map>
# include <vector>
# include <errno.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <algorithm>
# include <vector>
# include <fcntl.h>
# include <errno.h>
# include "types.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Exception.hpp"

const int BUFFER_SIZE = 6553600;
const int RECEIVE_SOCKET_STREAM_SIZE = 254560;
const int SEND_PIPE_STREAM_SIZE = 65536;
const int CHUNKED_LINE_LENGTH = 65536;
const int DEFAULT_TARGET_CHUNK_SIZE = -1;
const int CRLF_SIZE = 2;
const int NUM_OF_META_VARIABLES = 18;

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
    std::string _default_error_page; //TODO: delete 고려
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
    const std::string& getHost() const;
    const std::string& getPort() const;
    /* Setter */
    void setServerSocket();
    void setAuthBasic(const std::string& auth_basic, const std::string& route);
    void setAuthBasicUserFile(const std::string& decoded_id_password, const std::string& route);

    /* Exception */

    /* Util */
    void closeClientSocket(int fd);
    void closeFdAndSetClientOnWriteFdSet(int fd);
    void closeFdAndUpdateFdTable(int fd, FdSet fd_set);
    void closeFdAndSetFd(int clear_fd, FdSet clear_fd_set, int set_fd, FdSet set_fd_set);
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
    std::string makeResponseMessage(int fd);
    bool sendResponse(const std::string& response_meesage, int fd);
    bool isClientOfServer(int fd) const;
    bool isIndexFileExist(int fd);
    void findResourceAbsPath(int fd);
    bool isAutoIndexOn(int fd);
    bool isCGIUri(int fd, const std::string& extension);
    void checkAndSetResourceType(int fd);
    void checkValidOfCgiMethod(int fd);
    void openStaticResource(int fd);
    void setResourceAbsPathAsIndex(int fd);
    void processResponseBody(int fd);
    void preprocessResponseBody(int fd, ResType& res_type);
    void sendDataToCGI(int fd);
    void receiveDataFromCGI(int fd);

    void readStaticResource(int fd);

    void checkAuthenticate(int fd);

    void setAuthenticateRealm();

    void processIfHeadersNotFound(int fd, const std::string& readed);
    void putFileOnServer(int fd);
    void deleteResourceOfUri(int fd,const std::string& path);

    /* Server run function */
    void acceptClient();
    void openCGIPipe(int fd);
    void forkAndExecuteCGI(int fd);
    char** makeCGIArgv(int fd);
    char** makeCGIEnvp(int fd);
    bool makeEnvpUsingRequest(char** envp, int fd, int* idx);
    bool makeEnvpUsingResponse(char** envp, int fd, int* idx);
    bool makeEnvpUsingHeaders(char** envp, int fd, int* idx);
    bool makeEnvpUsingEtc(char** envp, int fd, int* idx);
    bool isResponseAllSended(int fd) const;

    bool isCGIReadPipe(int fd) const;
    bool isCGIWritePipe(int fd) const;

    void receiveChunkSize(int fd, size_t index_of_crlf);
    void receiveChunkData(int client_fd, int receive_size, int target_chunk_size);
    void receiveLastChunkData(int fd);

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
public:
    class MustRedirectException : public SendErrorCodeToClientException
    {
    private:
        Response& _res;
        std::string _msg;
    public:
        MustRedirectException(Response& res);
        virtual const char* what() const throw();
    };
    class CannotOpenDirectoryException : public SendErrorCodeToClientException
    {
    private:
        Response& _res;
        int _error_num;
        std::string _msg;
    public:
        CannotOpenDirectoryException(Response& res, const std::string& status_code, int error_num);
        virtual const char* what() const throw();
    };
    class IndexNoExistException : public SendErrorCodeToClientException
    {
    private:
        Response& _response;
    public:
        IndexNoExistException(Response& response);
        virtual const char* what() const throw();
    };
    class OpenResourceErrorException : public std::exception
    {
    private:
        Response& _response;
        int _error_num;
        std::string _msg;
    public:
        OpenResourceErrorException(Response& response, int error_num);
        virtual const char* what() const throw();
    };
    class CgiMethodErrorException : public SendErrorCodeToClientException
    {
    private:
        Response& _response;
    public:
        CgiMethodErrorException(Response& response);
        virtual const char* what() const throw();
    };
    class InternalServerException : public SendErrorCodeToClientException
    {
    private:
        Response& _response;
    public:
        InternalServerException(Response& response);
        virtual const char* what() const throw();
    };
    class AuthenticateErrorException : public SendErrorCodeToClientException
    {
    private:
        Response& _res;
        std::string _status_code;
    public:
        AuthenticateErrorException(Response& res, const std::string& status_code);
        virtual const char* what() const throw();
    };
    class CannotPutOnDirectoryException : public SendErrorCodeToClientException
    {
    private:
        Response& _response;
    public:
        CannotPutOnDirectoryException(Response& response);
        virtual const char* what() const throw();
    };
    class TargetResourceConflictException : public SendErrorCodeToClientException
    {
    private:
        Response& _response;
    public:
        TargetResourceConflictException(Response& response);
        virtual const char* what() const throw();
    };
    class UnchunkedErrorException : public SendErrorCodeToClientException
    {
    public:
        virtual const char* what() const throw();
    };
};

#endif
