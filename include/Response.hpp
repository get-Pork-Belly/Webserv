#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Request.hpp"
# include "types.hpp"

class Server;

class Response
{
private:
    std::string _status_code;
    std::map<std::string, std::string> _headers;
    std::string _transfer_type;
    std::string _clients;
    std::string _message_body;
    std::map<std::string, std::string> _status_code_table;
    location_info _location_info;

public:
    /* Constructor */
    Response();
    Response(const Response& other);
    // Response(Request& request, Server* server); //TODO 인자를 const로.

    /* Destructor */
    virtual ~Response();

    /* Overload */
    Response& operator=(const Response& rhs);
    /* Getter */
    std::string getStatusCode() const;
    std::string getStatusMessage(const std::string& code);
    // std::string getHeaders() const;
    // std::string getTransferType() const;
    // std::string getClients() const;
    const location_info& getLocationInfo() const;

    /* Setter */
    void setStatusCode(const std::string& status_code);
    // void setMessageBody();
    /* Exception */
    /* Util */
    // bool isLocationUri(const std::string& uri, Server* server);
    bool checkAndSetLocation(const std::string& uri, Server* server);
    bool isLimitExceptInLocation();
    bool isAllowedMethod(const std::string& method);

    void init();
    void initStatusCodeTable();
    // std::string makeBody(Request& request);
    // std::string makeHeaders(Request& request);
    std::string makeStatusLine();

    void applyAndCheckRequest(Request& request, Server* server);
};

#endif
