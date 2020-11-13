#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "utils.hpp"
# include "types.hpp"
# include <map>

//NOTE: test용으로 ostream include함.
#include <iostream>

class Request
{
private:
    std::string _method;
    std::string _uri;
    std::string _version;
    std::map<std::string, std::string> _headers;
    std::string _protocol;
    std::string _bodies;
    std::string _status_code;
    ReqInfo _info;
    bool _is_buffer_left;
    std::string _ip_address;
    int _transfered_body_size;
    std::string _remote_user;
    std::string _remote_ident;
    std::string _auth_type;

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
    const std::string& getBodies() const;
    const std::string& getStatusCode() const;
    const ReqInfo& getReqInfo() const;
    bool getIsBufferLeft() const;
    const std::string& getIpAddress() const;
    int getContentLength() const;
    int getTransferedBodySize() const;
    const std::string& getAuthType() const;
    const std::string& getRemoteUser() const;
    const std::string& getRemoteIdent() const;


    /* Setter */
    void setMethod(const std::string& method);
    void setUri(const std::string& uri);
    void setVersion(const std::string& version);
    void setHeaders(const std::string& key, const std::string& value);
    void setProtocol(const std::string& protocol);
    void setBodies(const std::string& body);
    void setStatusCode(const std::string& code);
    void setReqInfo(const ReqInfo& info);
    void setIsBufferLeft(const bool& is_left_buffer);
    void setIpAddress(const std::string& ip_address);
    void setTransferedBodySize(const int transfered_body_size);
    void setAuthType(const std::string& auth_type);
    void setRemoteUser(const std::string& remote_user);
    void setRemoteIdent(const std::string& remote_ident);


    /* Util */

    void init();

    void updateReqInfo();
    bool updateStatusCodeAndReturn(const std::string& status_code, const bool& ret);

    bool isBodyUnnecessary() const;
    bool isNormalBody() const;
    bool isChunkedBody() const;
    bool isContentLeftInBuffer() const;


    // void initMembers(std::string req_message);

    /* parser */
    void parseRequestWithoutBody(char* buf);
    bool parseRequestLine(std::string& req_message);
    bool parseHeaders(std::string& req_message);
    void parseChunkedBody(char* buf);

    /* valid check */
    bool isValidLine(std::vector<std::string>& request_line);
    bool isValidMethod(const std::string& method);
    bool isValidUri(const std::string& uri);
    bool isValidVersion(const std::string& version);

    bool isValidHeaders(std::string& key, std::string& value);
    bool isValidHeaderFields(std::string& key);
    bool isValidSP(std::string& str);
    bool isDuplicatedHeader(std::string& key);

    void appendBody(char* buf, int bytes);

    /* Exception */
public:
    class RequestFormatException : public std::exception
    {
    private:
        std::string _msg;
        Request& _req;
    public:
        RequestFormatException(Request& req, const std::string& status_code);
        RequestFormatException(Request& req);
        virtual const char* what() const throw();
    };
};

std::ostream& operator<<(std::ostream& out, Request& object);

#endif

