#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <dirent.h>
# include <sys/stat.h>
# include "Request.hpp"
# include "types.hpp"
# include "Exception.hpp"

class Server;

class Response
{
private:
    std::string _status_code;
    std::map<std::string, std::string> _headers;
    std::string _transfer_type;
    std::string _clients;
    std::map<std::string, std::string> _status_code_table;
    std::map<std::string, std::string> _mime_type_table;
    location_info _location_info;
    std::string _resource_abs_path;
    std::string _route;
    std::string _directory_entry;
    struct stat _file_info;
    ResType _resource_type;
    std::string _body;

    int _stdin_of_cgi;
    int _stdout_of_cgi;
    int _read_fd_from_cgi;
    int _write_fd_to_cgi;

    int _cgi_pid;

    std::string _uri_path;
    std::string _uri_extension;
    std::string _transmitting_body;

    //TODO: 생성자에 추가하기.
    size_t _already_encoded_size;

    SendProgress _send_progress;

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
    const std::string& getStatusCode() const;
    const std::string& getStatusMessage(const std::string& code);
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
    const std::string& getUriPath() const;
    // int getCGIPipeFd() const;
    const std::map<std::string, std::string>& getMimeTypeTable() const;
    const std::string& getUriExtension() const;
    int getStdinOfCGI() const;
    int getStdoutOfCGI() const;
    int getReadFdFromCGI() const;
    int getWriteFdToCGI() const;
    int getCGIPid() const;
    const std::string& getTransmittingBody() const;

    size_t getAlreadyEncodedSize() const;
    const SendProgress& getSendProgress() const;

    /* Setter */
    void setStatusCode(const std::string& status_code);
    void setResourceAbsPath(const std::string& path);
    void setDirectoryEntry(DIR* dir_ptr);
    void setFileInfo(const struct stat& file_info);
    void setResourceType(const ResType& resource_type);
    void setBody(const std::string& body);
    void setUriPath(const std::string& path);
    void setUriExtension(const std::string& extension);
    // void setMessageBody();

    void setStdinOfCGI(const int fd);
    void setStdoutOfCGI(const int fd);
    void setReadFdFromCGI(const int fd);
    void setWriteFdToCGI(const int fd);

    void setCGIPid(const int pid);

    void setAlreadyEncodedSize(const size_t already_encoded_size);
    void setSendProgress(const SendProgress send_progress);

    /* Exception */
public:
    class CannotOpenCGIPipeException : public SendErrorCodeToClientException
    {
    private:
        Response& _response;
    public:
        CannotOpenCGIPipeException(Response& response);
        virtual const char* what() const throw();
    };
    /* Util */
    // bool isLocationUri(const std::string& uri, Server* server);
    bool setRouteAndLocationInfo(const std::string& uri, Server* server);
    bool isLimitExceptInLocation();
    bool isAllowedMethod(const std::string& method);
    bool isExtensionExist(const std::string& extension) const;
    bool isExtensionInMimeTypeTable(const std::string& extension) const;
    void findAndSetUriExtension();
    bool isNeedToBeChunkedBody(const Request& request) const;
    bool isRedirection(const std::string& status_code) const;
    bool isLocationToBeRedirected() const;
    std::string getRedirectStatusCode() const;
    std::string getRedirectUri(const Request& request) const;
    std::string getLastModifiedDateTimeOfResource() const;
    std::string getHtmlLangMetaData() const;

    void setTransmittingBody(const std::string& chunked_body);
    void encodeChunkedBody();

    void init();
    void initStatusCodeTable();
    void initMimeTypeTable();
    void  makeBody(Request& request);
    std::string makeHeaders(Request& request);
    std::string makeStatusLine();

    void appendBody(char* buf);

    /* General header */
    void appendDateHeader(std::string& headers);
    void appendServerHeader(std::string& headers);

    /* Entity header */
    void appendAllowHeader(std::string& headers);
    void appendContentLanguageHeader(std::string& headers);
    void appendContentLengthHeader(std::string& headers);
    void appendContentLocationHeader(std::string& headers);
    void appendContentTypeHeader(std::string& headers);
    void appendLastModifiedHeader(std::string& headers);
    void appendLocationHeader(std::string& headers, const Request& request);
    void appendRetryAfterHeader(std::string& headers, const std::string& status_code);
    void appendTransferEncodingHeader(std::string& headers);



};

#endif
