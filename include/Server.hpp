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
# include <Python/Python.h>
# include <algorithm>
# include <vector>
# include <fcntl.h>
# include <errno.h>
# include "types.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Exception.hpp"

const int BUFFER_SIZE = 65536;
const int SHOULD_RECEIVE_MORE = -3;
const int RECEIVE_SOCKET_STREAM_SIZE = 65536;
const int SEND_PIPE_STREAM_SIZE = 65536;
const int CHUNKED_LINE_LENGTH = 65536;
const int DEFAULT_LIMIT_CLIENT_BODY_LENGTH = 2147483647;
const int DEFAULT_TARGET_CHUNK_SIZE = -2;
const int DEFAULT_INDEX_OF_CRLF = -1;
const int CRLF_SIZE = 2;
const int NUM_OF_META_VARIABLES = 18;
const int DEFAULT_FD = -1;
const int LIMIT_HEADERS_LENGTH = 8192;
const std::string PHP_CGI_PATH("./php-mac/bin/php-cgi");

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
    std::string _host;
    std::string _port;
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
    const std::map<std::string, std::string> getServerConfig();
    const std::map<std::string, location_info>& getLocationConfig();
    int getServerSocket() const;
    Request& getRequest(int fd);
    Response& getResponse(int fd);
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
    bool isCgiPipe(int fd) const;

    /* Server function */
    void init();
    void run(int fd);
    void receiveRequest(int fd);
    void receiveRequestLine(int fd);
    void receiveRequestHeaders(int fd);
    void readBufferUntilRequestLine(int fd);
    bool readBufferUntilHeaders(int fd, char* buf, size_t read_target);
    void receiveRequestNormalBody(int fd);
    void receiveRequestChunkedBody(int fd);
    void makeResponseMessage(int fd);
    void setResponseMessageAccordingToTheParseProgress(int client_fd, std::string& status_line, std::string& headers);
    void sendResponse(int fd);
    bool isClientOfServer(int fd) const;
    bool isIndexFileExist(int fd);
    void parseUriAndSetResponse(int fd);
    bool isAutoIndexOn(int fd);
    bool isCgiUri(int fd, const std::string& extension);
    void checkAndSetResourceType(int fd);
    void checkValidOfCgiMethod(int fd);
    void openStaticResource(int fd);
    void setResourceAbsPathAsIndex(int fd);
    void processResponseBody(int fd);
    void preprocessResponseBody(int fd, ResType& res_type);
    void sendDataToCgi(int fd);
    void receiveDataFromCgi(int fd);

    void readStaticResource(int fd);

    void checkAuthenticate(int fd);

    void setAuthenticateRealm();

    void processIfHeadersNotFound(int fd, const std::string& readed);
    void putFileOnServer(int fd);
    void deleteResourceOfUri(int fd,const std::string& path);

    /* Server run function */
    void acceptClient();
    void openCgiPipe(int fd);
    void forkAndExecuteCgi(int fd);
    char** makeCgiArgv(int fd);
    char** makeCgiEnvp(int fd);
    bool makeEnvpUsingRequest(char** envp, int fd, int* idx);
    bool makeEnvpUsingResponse(char** envp, int fd, int* idx);
    bool makeEnvpUsingHeaders(char** envp, int fd, int* idx);
    bool makeEnvpUsingEtc(char** envp, int* idx);
    bool isResponseAllSended(int fd) const;

    bool isCgiReadPipe(int fd) const;
    bool isCgiWritePipe(int fd) const;

    void receiveChunkSize(int fd);
    void receiveChunkData(int client_fd, int receive_size);
    void receiveLastChunkData(int fd);
    bool isExistCrlf(int fd, const RecvRequest recv_request);
    bool isNotYetSetTargetChunkSize(int fd);
    void findCrlfAndSetIndexOfCrlf(int fd, const std::string& buf, const RecvRequest recv_request);
    bool isLastSequenceOfParsingChunk(int fd);
    int calculateReceiveTargetSizeOfChunkData(int fd);
    bool isChunkDataAllReceived(int fd);
    void prepareToReceiveHeaders(int fd);
    void prepareToReceiveNextChunkSize(int fd);
    void prepareToReceiveNextChunkData(int fd);
    void finishChunkSequence(int fd);

    void finishPutFileOnServer(int resource_fd);
    void finishReadStaticResource(int resource_fd);
    void finishSendDataToCgiPipe(int write_fd_to_cgi);
    void finishReceiveDataFromCgiPipe(int read_fd_from_cgi);

public:
    class PayloadTooLargeException : public SendErrorCodeToClientException
    {
    public:
        PayloadTooLargeException(Server& server, int client_fd);
        virtual const char* what() const throw();
    };
    class ReadErrorException : public SendErrorCodeToClientException
    {
    public:
        ReadErrorException(Server& server, int client_fd);
        virtual const char* what() const throw();
    };
public:
    class MustRedirectException : public SendErrorCodeToClientException
    {
    private:
        std::string _msg;
    public:
        MustRedirectException(Server& server, int client_fd);
        virtual const char* what() const throw();
    };
    class CannotOpenDirectoryException : public SendErrorCodeToClientException
    {
    private:
        std::string _msg;
    public:
        CannotOpenDirectoryException(Server& server, int client_fd, const std::string& status_code, int error_num);
        virtual const char* what() const throw();
    };
    class IndexNoExistException : public SendErrorCodeToClientException
    {
    public:
        IndexNoExistException(Server& server, int client_fd);
        virtual const char* what() const throw();
    };
    class OpenResourceErrorException : public SendErrorCodeToClientException
    {
    private:
        std::string _msg;
    public:
        OpenResourceErrorException(Server& server, int client_fd, int error_num);
        virtual const char* what() const throw();
    };
    class CgiMethodErrorException : public SendErrorCodeToClientException
    {
    public:
        CgiMethodErrorException(Server& server, int client_fd);
        virtual const char* what() const throw();
    };
    class InternalServerException : public SendErrorCodeToClientException
    {
    public:
        InternalServerException(Server& server, int client_fd);
        virtual const char* what() const throw();
    };
    class AuthenticateErrorException : public SendErrorCodeToClientException
    {
    public:
        AuthenticateErrorException(Server& server, int client_fd, const std::string& status_code);
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
    public:
        TargetResourceConflictException(Server& server, int client_fd);
        virtual const char* what() const throw();
    };
    class UnchunkedErrorException : public SendErrorCodeToClientException
    {
    public:
        UnchunkedErrorException(Server& server, int client_fd);
        virtual const char* what() const throw();
    };
    class NotAllowedMethodException : public SendErrorCodeToClientException
    {
    public:
        NotAllowedMethodException(Server& server, int client_fd);
        virtual const char* what() const throw();
    };
    class CannotWriteToClientException : public CannotSendErrorCodeToClientException
    {
    public:
        CannotWriteToClientException(Server& server, int client_fd);
        virtual const char* what() const throw();
    };
   class ReadStaticResourceErrorException: public SendErrorCodeToClientException
    {
    public:
        ReadStaticResourceErrorException(Server& server, int resource_fd);
        virtual const char* what() const throw();
    };
   class ReceiveDataFromCgiPipeErrorException: public SendErrorCodeToClientException
    {
    public:
        ReceiveDataFromCgiPipeErrorException(Server& server, int read_fd_from_cgi);
        virtual const char* what() const throw();
    };
   class SendDataToCgiPipeErrorException: public SendErrorCodeToClientException
    {
    public:
        SendDataToCgiPipeErrorException(Server& server, int write_fd_to_cgi);
        virtual const char* what() const throw();
    };
   class PutFileOnServerErrorException: public SendErrorCodeToClientException
    {
    public:
        PutFileOnServerErrorException(Server& server, int resource_fd);
        virtual const char* what() const throw();
    };
};

#endif
