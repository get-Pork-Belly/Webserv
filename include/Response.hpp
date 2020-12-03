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
    std::vector<std::string> _directory_entry;
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
    std::string _query;

    size_t _already_encoded_size;

    ParseProgress _parse_progress;
    ReceiveProgress _receive_progress;

    int _resource_fd;
    int _sended_response_size;
    std::string _response_message;
    SendProgress _send_progress;

    std::string _temp_buffer;

public:
    /* Constructor */
    Response();
    Response(const Response& other);

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
    const std::vector<std::string>& getDirectoryEntry() const;
    const struct stat& getFileInfo() const;
    const ResType& getResourceType() const;
    const std::string& getBody() const;
    const std::string& getUriPath() const;
    // int getCgiPipeFd() const;
    const std::map<std::string, std::string>& getMimeTypeTable() const;
    const std::string& getUriExtension() const;
    int getStdinOfCgi() const;
    int getStdoutOfCgi() const;
    int getReadFdFromCgi() const;
    int getWriteFdToCgi() const;
    int getCgiPid() const;
    const std::string& getTransmittingBody() const;

    size_t getAlreadyEncodedSize() const;
    const ParseProgress& getParseProgress() const;
    const ReceiveProgress& getReceiveProgress() const;

    int getResourceFd() const;
    const std::map<std::string, std::string>& getHeaders() const;
    int getSendedResponseSize() const;
    const std::string& getResponseMessage() const;
    const SendProgress& getSendProgress() const;
    const std::string& getTempBuffer() const;
    const std::string& getQuery() const;

    /* Setter */
    void setStatusCode(const std::string& status_code);
    void setResourceAbsPath(const std::string& path);
    void setDirectoryEntry(DIR* dir_ptr);
    void setFileInfo(const struct stat& file_info);
    void setResourceType(const ResType& resource_type);
    void setBody(const std::string& body);
    void setUriPath(const std::string& path);
    void setQuery(const std::string& query);
    void setUriExtension(const std::string& extension);
    void setHeaders(const std::string& key, const std::string& value);

    void setStdinOfCgi(const int fd);
    void setStdoutOfCgi(const int fd);
    void setReadFdFromCgi(const int fd);
    void setWriteFdToCgi(const int fd);

    void setCgiPid(const int pid);

    void setAlreadyEncodedSize(const size_t already_encoded_size);
    void setParseProgress(const ParseProgress sparseprogress);
    void setReceiveProgress(const ReceiveProgress rececive_progress);
    void setResourceFd(const int resource_fd);
    void setSendedResponseSize(const int sended_response_size);
    void setResponseMessage(const std::string& response_message);
    void setSendProgress(const SendProgress& send_progress);
    void setTempBuffer(const std::string& temp_buffer);

    bool isCgiWritePipeNotClosed() const;
    bool isCgiReadPipeNotClosed() const;
    bool isResourceNotClosed() const;


    /* Exception */
public:
    class CannotOpenCgiPipeException : public SendErrorCodeToClientException
    {
    private:
        Response& _response;
    public:
        CannotOpenCgiPipeException(Response& response);
        virtual const char* what() const throw();
    };
    class InvalidCgiMessageException: public SendErrorCodeToClientException
    {
    private:
        std::string _msg;
        Response& _response;
    public:
        InvalidCgiMessageException(Response& response);
        InvalidCgiMessageException(Response& response, const std::string& status_code);
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
    bool findEndOfHeaders();
    void preparseCgiMessage();
    bool parseCgiHeaders(std::string& cgi_message);
    bool isValidHeaders(std::string& key, std::string& value);
    bool isValidSP(std::string& str);
    bool isDuplicatedHeader(std::string& key);
    bool isFileInDirEntry(std::string& index);

    void setTransmittingBody(const std::string& chunked_body);
    void encodeChunkedBody();

    void init();
    void initStatusCodeTable();
    void initMimeTypeTable();
    void makeBody(Request& request);
    void makeTraceBody(const Request& request);
    void makeOptionBody();
    std::string makeHeaders(Request& request);
    std::string makeStatusLine();

    void appendBody(char* buf, int bytes);
    void appendTempBuffer(char* buf, int bytes);

    /* General header */
    void appendDateHeader(std::string& headers);
    void appendServerHeader(std::string& headers);

    /* Entity header */
    void appendAllowHeader(std::string& headers);
    void appendContentLanguageHeader(std::string& headers);
    void appendContentLengthHeader(std::string& headers, const std::string& method);
    void appendContentLocationHeader(std::string& headers);
    void appendContentTypeHeader(std::string& headers);
    void appendLastModifiedHeader(std::string& headers);
    void appendLocationHeader(std::string& headers, const Request& request);
    void appendRetryAfterHeader(std::string& headers, const std::string& status_code);
    void appendTransferEncodingHeader(std::string& headers, const std::string& method);
    void appendAuthenticateHeader(std::string& headers);

};

#endif
