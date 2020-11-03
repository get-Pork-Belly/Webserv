#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <dirent.h>
# include <sys/stat.h>
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
    std::map<std::string, std::string> _status_code_table;
    location_info _location_info;
    std::string _resource_abs_path;
    std::string _route;
    std::string _directory_entry;
    struct stat _file_info;
    ResType _resource_type;
    std::string _body;

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
    const std::string& getDirectoryEntry() const;
    const struct stat& getFileInfo() const;
    const ResType& getResourceType() const;
    const std::string& getBody() const;

    /* Setter */
    void setStatusCode(const std::string& status_code);
    void setResourceAbsPath(const std::string& path);
    void setDirectoryEntry(DIR* dir_ptr);
    void setFileInfo(const struct stat& file_info);
    void setResourceType(const ResType& resource_type);
    void setBody(const std::string& body);
    // void setMessageBody();
    /* Exception */
    /* Util */
    // bool isLocationUri(const std::string& uri, Server* server);
    bool setRouteAndLocationInfo(const std::string& uri, Server* server);
    bool isLimitExceptInLocation();
    bool isAllowedMethod(const std::string& method);

    void init();
    void initStatusCodeTable();
    void  makeBody(Request& request);
    std::string makeHeaders(Request& request);
    std::string makeStatusLine();

    void applyAndCheckRequest(Request& request, Server* server);
    void appendBody(char *buf);
};

#endif
