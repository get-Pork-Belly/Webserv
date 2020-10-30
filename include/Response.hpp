#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Request.hpp"
# include "types.hpp"
# include <sys/stat.h>

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
    std::string _resource_abs_path;
    std::string _route;
    struct stat _file_info;

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
    const std::string& getRoute() const;
    // std::string getHeaders() const;
    // std::string getTransferType() const;
    // std::string getClients() const;
    const location_info& getLocationInfo() const;
    const std::string& getResourceAbsPath() const;
    struct stat getFileInfo() const;

    /* Setter */
    void setStatusCode(const std::string& status_code);
    void setResourceAbsPath(const std::string& path);
    // void setMessageBody();
    /* Exception */
    /* Util */
    // bool isLocationUri(const std::string& uri, Server* server);
    bool setRouteAndLocationInfo(const std::string& uri, Server* server);
    bool isLimitExceptInLocation();
    bool isAllowedMethod(const std::string& method);

    void init();
    void initStatusCodeTable();
    // std::string makeBody(Request& request);
    std::string makeHeaders(Request& request);
    std::string makeStatusLine();

    void applyAndCheckRequest(Request& request, Server* server);
};

#endif
