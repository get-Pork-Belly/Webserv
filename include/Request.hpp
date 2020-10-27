#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "utils.hpp"
# include "types.hpp"
# include <map>

//TODO: 테스트용
# include <iostream>

class Request
{
private:
    std::string _method;
    std::string _uri;
    std::string _version;
    std::map<std::string, std::string> _headers;
    std::string _protocol;
    std::string _bodies;
    std::string _transfer_type;
    std::string _status_code;
    ReqInfo _info;
    size_t _header_end_pos;

public:
    /* Constructor */
    Request();
    Request(const Request& other);
    Request& operator=(const Request& rhs);

    /* Destructor */
    virtual ~Request();

    /* Getter */
    std::string getMethod();
    const std::string& getUri();
    std::string getVersion();
    std::map<std::string, std::string> getHeaders();
    std::string getProtocol();
    std::string getBodies();
    std::string getTransferType();
    std::string getStatusCode();
    const ReqInfo& getReqInfo() const;
    const size_t& getHeaderEndPos() const;

    /* Setter */
    void setMethod(const std::string& method);
    void setUri(const std::string& uri);
    void setVersion(const std::string& version);
    void setHeaders(const std::string& key, const std::string& value);
    void setProtocol(const std::string& protocol);
    void setBodies(const std::string& body);
    void setTransferType(const std::string& transfer_type);
    void setStatusCode(const std::string& code);
    void setHeaderEndPos(const size_t& header_end_pos);
    void setReqInfo(const ReqInfo& info);

    /* Exception */

    /* Util */

    void clear();

    // void initMembers(std::string req_message);

    /* parser */
    bool parseRequestWithoutBody(std::string& buf);

    bool parseRequest(std::string& req_message);
    bool parseRequestLine(std::string& req_message);
    bool parseHeaders(std::string& req_message);
    bool parseBodies(std::string& req_message);
    bool parseChunkedBody(std::string &req_message);

    /* valid check */
    bool isValidLine(std::vector<std::string>& request_line);
    bool isValidMethod(const std::string& method);
    bool isValidUri(const std::string& uri);
    bool isValidVersion(const std::string& version);

    bool isValidHeaders(std::string& key, std::string& value);
    bool isValidHeaderFields(std::string& key);
    bool isValidSP(std::string& str);
    bool isDuplicatedHeader(std::string& key);

    bool isValidBodies();
};

#endif

