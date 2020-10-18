#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Server.hpp"
# include "utils.hpp"

//TODO: 테스트용
# include <iostream>

class Request
{
private:
    std::string _request_method;
    std::string _request_uri;
    std::string _request_version;
    std::map<std::string, std::string> _request_headers;
    // std::string _request_protocol;
    std::string _request_bodies;
    // std::string _request_transfer_type;
    // std::string _status_code;

public:
    /* Constructor */
    Request(/* args*/);
    Request(const Request& other);

    /* Destructor */
    virtual ~Request();

    /* Overload */
    Request& operator=(const Request& rhs);

    /* Getter */
    std::string getRequestMethod();
    std::string getRequestUri();
    std::string getRequestVersion();

    // std::string getRequestHeaders();
    std::map<std::string, std::string> getRequestHeaders();
    // std::string getRequestProtocol();
    std::string getRequestBodies();
    // std::string getRequestTransferType();
    // std::string getStatusCode();

    /* Setter */
    void setRequestMethod(std::string method);
    void setRequestUri(std::string uri);
    void setRequestVersion(std::string version);
    void setRequestHeaders(std::map<std::string, std::string> request_headers);
    // void setRequestProtocol(std::vector<std::string> request_line);
    void setRequestBodies(std::string body);
    // void setRequestTransferType(std::vector<std::string> request_line);
    // void setStatusCode(std::string code);

    /* Exception */

    /* Util */

    // void initMembers(std::string req_message);

    void parseRequest(std::string &req_message);
    void parseRequestLine(std::string &req_message);
    void parseRequestHeaders(std::string &req_message);
    void parseRequestBodies(std::string &req_message);

};

#endif