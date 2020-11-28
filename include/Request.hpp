#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "utils.hpp"
# include "types.hpp"
# include <map>
# include "Exception.hpp"

//NOTE: test용으로 ostream include함.
#include <iostream>

# define RECV_COUNT_NOT_REACHED -2

class Request
{
private:
    std::string _method;
    std::string _uri;
    std::string _version;
    std::map<std::string, std::string> _headers;
    std::string _protocol;
    std::string _body;
    std::string _status_code;
    RecvRequest _info;
    std::string _ip_address;
    int _transfered_body_size;
    std::string _remote_user;
    std::string _remote_ident;
    std::string _auth_type;
    int _target_chunk_size;
    int _received_chunk_data_size;

    int _recv_counts;
    bool _carriege_return_trimmed;
    std::string _temp_buffer;

public:
    /* Constructor */
    Request();
    Request(const Request& other);
    Request& operator=(const Request& rhs);

    /* Destructor */
    virtual ~Request();

    /* Getter */
    const std::string& getMethod() const;
    const std::string& getUri() const;
    const std::string& getVersion() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getProtocol() const;
    const std::string& getBody() const;
    const std::string& getStatusCode() const;
    const RecvRequest& getRecvRequest() const;
    const std::string& getIpAddress() const;
    int getContentLength() const;
    int getTransferedBodySize() const;
    const std::string& getAuthType() const;
    const std::string& getRemoteUser() const;
    const std::string& getRemoteIdent() const;
    int getTargetChunkSize() const;
    int getReceivedChunkDataSize() const;
    bool getCarriegeReturnTrimmed() const;

    int getReceiveCounts() const;
    const std::string& getTempBuffer() const;

    /* Setter */
    void setMethod(const std::string& method);
    void setUri(const std::string& uri);
    void setVersion(const std::string& version);
    void setHeaders(const std::string& key, const std::string& value);
    void setProtocol(const std::string& protocol);
    void setBody(const std::string& body);
    void setStatusCode(const std::string& code);
    void setRecvRequest(const RecvRequest& info);
    void setIpAddress(const std::string& ip_address);
    void setTransferedBodySize(const int transfered_body_size);
    void setAuthType(const std::string& auth_type);
    void setRemoteUser(const std::string& remote_user);
    void setRemoteIdent(const std::string& remote_ident);
    void setTargetChunkSize(const int target_size);
    void setReceivedChunkDataSize(const int received_chunk_data_size);

    void setReceiveCounts(const int recv_count);
    void setCarriegeReturnTrimmed(const bool trimmed);
    void setTempBuffer(const std::string& temp_buffer);

    /* Util */

    void init();

    void updateRecvRequest();
    bool updateStatusCodeAndReturn(const std::string& status_code, const bool& ret);

    bool isBodyUnnecessary() const;
    bool isNormalBody() const;
    bool isChunkedBody() const;

    int peekMessageFromClient(int client_fd, char* buf);
    void raiseRecvCounts();

    /* parser */
    void parseRequestLine(char* buf, int bytes);
    void parseRequestHeaders();
    void parseRequestWithoutBody(char* buf, int bytes);
    bool parseHeaders(std::string& req_message);

    /* valid check */
    bool isValidLine(std::vector<std::string>& request_line);
    bool isValidMethod(const std::string& method);
    bool isValidUri(const std::string& uri);
    bool isValidVersion(const std::string& version);

    bool isValidHeaders(std::string& key, std::string& value);
    bool isValidHeaderFields(std::string& key);
    bool isValidSP(std::string& str);
    bool isDuplicatedHeader(std::string& key);

    bool isCarriegeReturnTrimmed();

    void appendBody(char* buf, int bytes);
    void appendBody(const char* buf, int bytes);
    void appendTempBuffer(char* buf, int bytes);

    void parseTargetChunkSize(const std::string& chunk_size_line);
    void parseChunkDataAndSetChunkSize(char* buf, size_t bytes, int next_target_chunk_size);
    void parseChunkData(char* buf, size_t bytes);

    int calculateReadTargetSize(char* buf, int peeked_bytes);

    /* Exception */
public:
    class RequestFormatException : public std::exception
    {
    private:
        std::string _msg;
        Request& _req;
    public:
        RequestFormatException(Request& req, const std::string& status_code);
        virtual const char* what() const throw();
    };

    class UriTooLongException : public std::exception
    {
    private:
        Request& _req;
    public:
        UriTooLongException(Request& req);
        virtual const char* what() const throw();
    };

};

std::ostream& operator<<(std::ostream& out, Request& object);

#endif

