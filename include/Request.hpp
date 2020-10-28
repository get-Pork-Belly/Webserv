#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "utils.hpp"
# include "types.hpp"
# include <map>

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
    bool _is_left_buffer;

public:
    /* Constructor */
    Request();
    Request(const Request& other);
    Request& operator=(const Request& rhs);

    /* Destructor */
    virtual ~Request();

    /* Getter */
    std::string getMethod() const;
    const std::string& getUri();
    std::string getVersion();
    std::map<std::string, std::string> getHeaders() const;
    std::string getProtocol();
    std::string getBodies();
    std::string getTransferType();
    std::string getStatusCode();
    const ReqInfo& getReqInfo() const;
    bool getIsLeftBuffer() const;

    /* Setter */
    void setMethod(const std::string& method);
    void setUri(const std::string& uri);
    void setVersion(const std::string& version);
    void setHeaders(const std::string& key, const std::string& value);
    void setProtocol(const std::string& protocol);
    void setBodies(const std::string& body);
    void setTransferType(const std::string& transfer_type);
    void setStatusCode(const std::string& code);
    void setReqInfo(const ReqInfo& info);
    void setIsLeftBuffer(const bool& is_left_buffer);

    /* Util */

    void clear();

    void updateReqInfo();
    bool updateStatusCodeAndReturn(const std::string& status_code, const bool& ret);

    bool isBodyUnnecessary() const;
    bool isNormalBody() const;
    bool isChunkedBody() const;
    bool isContentLeftInBuffer() const;

    int getContentLength();

    // void initMembers(std::string req_message);

    /* parser */
    void parseRequestWithoutBody(std::string& buf);
    bool parseRequestLine(std::string& req_message);
    bool parseHeaders(std::string& req_message);
    void parseNormalBodies(char* buf);
    void parseChunkedBody(std::string &req_message);

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

#endif

