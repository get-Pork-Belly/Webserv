#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "utils.hpp"
# include <map>

//TODO: 테스트용
# include <iostream>

class Request
{
private:
    std::string _request_method;
    std::string _request_uri;
    std::string _request_version;
    std::map<std::string, std::string> _request_headers;
    std::string _request_protocol;
    std::string _request_bodies;
    std::string _request_transfer_type;
    std::string _status_code;

public:
    /* Constructor */
    Request();
    Request(const Request& other);
    Request& operator=(const Request& rhs);

    /* Destructor */
    virtual ~Request();

    /* Getter */
    std::string getRequestMethod();
    std::string getRequestUri();
    std::string getRequestVersion();
    std::map<std::string, std::string> getRequestHeaders();
    std::string getRequestProtocol();
    std::string getRequestBodies();
    std::string getRequestTransferType();
    std::string getStatusCode();

    /* Setter */
    void setRequestMethod(const std::string& method);
    void setRequestUri(const std::string& uri);
    void setRequestVersion(const std::string& version);
    void setRequestHeaders(const std::string& key, const std::string& value);
    void setRequestProtocol(const std::string& protocol);
    void setRequestBodies(const std::string& body);
    void setRequestTransferType(const std::string& transfer_type);
    void setStatusCode(const std::string& code);

    /* Exception */

    /* Util */

    // void initMembers(std::string req_message);

    /* parser */
    bool parseRequest(std::string& req_message);
    bool parseRequestLine(std::string& req_message);
    bool parseRequestHeaders(std::string& req_message);
    bool parseRequestBodies(std::string& req_message);

    bool parseChunkedBody(std::string &req_message);

    /* valid check */
    bool isValidRequestLine(std::vector<std::string>& request_line);
    bool isValidRequestMethod(const std::string& method);
    bool isValidRequestUri(const std::string& uri);
    bool isValidRequestVersion(const std::string& version);

    bool isValidRequestHeaders(std::string& key, std::string& value);
    bool isValidRequestHeaderFields(std::string& key);
    bool isValidSP(std::string& str);
    bool isDuplicated(std::string& key);

    bool isValidRequestBodies();
};

#endif
