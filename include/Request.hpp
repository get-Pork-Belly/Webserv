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

public:
    /* Constructor */
    Request();
    Request(const Request& other);
    Request& operator=(const Request& rhs);

    /* Destructor */
    virtual ~Request();

    /* Getter */
    std::string getMethod() const;
    const std::string& getUri() const;
    std::string getVersion();
    std::map<std::string, std::string> getHeaders() const;
    std::string getProtocol();
    std::string getBodies();
    std::string getStatusCode();
    const ReqInfo& getReqInfo() const;
    bool getIsBufferLeft() const;
    const std::string& getIpAddress() const;
    int getContentLength();
    int getTransferedBodySize() const;

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

    /* Util */

    void clear();

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
    void parseNormalBodies(char* buf);
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

