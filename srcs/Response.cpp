#include "Response.hpp"
#include "Server.hpp"
#include "PageGenerator.hpp"
#include "Log.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

/*
    ResType _resource_type;
*/

Response::Response()
: _status_code("200"), _headers(), _transfer_type(""), _clients(""),
_location_info(), _resource_abs_path(""), _route(""),
_directory_entry(""), _resource_type(ResType::NOT_YET_CHECKED), _body(""),
_stdin_of_cgi(0), _stdout_of_cgi(0), _read_fd_from_cgi(0), _write_fd_to_cgi(0), 
_cgi_pid(0), _uri_path(""), _uri_extension(""), _transmitting_body(""),
_already_encoded_size(0), _send_progress(SendProgress::DEFAULT),
_receive_progress(ReceiveProgress::DEFAULT), _resoure_fd(0)
{
    ft::memset(&this->_file_info, 0, sizeof(this->_file_info));
    this->initStatusCodeTable();
    this->initMimeTypeTable();
}

Response::Response(const Response& other)
: _status_code(other._status_code), _headers(other._headers),
_transfer_type(other._transfer_type), _clients(other._clients),
_status_code_table(other._status_code_table), _mime_type_table(other._mime_type_table),
_location_info(other._location_info), _resource_abs_path(other._resource_abs_path),
_route(other._route), _directory_entry(other._directory_entry),
_file_info(other._file_info), _resource_type(other._resource_type),
_body(other._body), _stdin_of_cgi(other._stdout_of_cgi),
_stdout_of_cgi(other._stdout_of_cgi), _read_fd_from_cgi(other._read_fd_from_cgi),
_write_fd_to_cgi(other._write_fd_to_cgi), _cgi_pid(other._cgi_pid),
_uri_path(other._uri_path), _uri_extension(other._uri_extension), _transmitting_body(other._transmitting_body),
_already_encoded_size(other._already_encoded_size), _send_progress(other._send_progress),
_receive_progress(other._receive_progress), _resoure_fd(other._resoure_fd)
{}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

Response::~Response()
{
}

/*============================================================================*/
/*******************************  Overload  ***********************************/
/*============================================================================*/

Response&
Response::operator=(const Response& rhs)
{
    this->_status_code = rhs._status_code;
    this->_headers = rhs._headers;
    this->_transfer_type = rhs._transfer_type;
    this->_clients = rhs._clients;
    this->_status_code_table = rhs._status_code_table;
    this->_mime_type_table = rhs._mime_type_table;
    this->_location_info = rhs._location_info;
    this->_resource_abs_path = rhs._resource_abs_path;
    this->_route = rhs._route;
    this->_directory_entry = rhs._directory_entry;
    this->_file_info = rhs._file_info;
    this->_resource_type = rhs._resource_type;
    this->_body = rhs._body;
    this->_stdin_of_cgi = rhs._stdin_of_cgi;
    this->_stdout_of_cgi = rhs._stdout_of_cgi;
    this->_read_fd_from_cgi = rhs._read_fd_from_cgi;
    this->_write_fd_to_cgi = rhs._write_fd_to_cgi;
    this->_cgi_pid = rhs._cgi_pid;
    this->_uri_path = rhs._uri_path;
    this->_uri_extension = rhs._uri_extension;
    this->_transmitting_body = rhs._transmitting_body;
    this->_already_encoded_size = rhs._already_encoded_size;
    this->_send_progress = rhs._send_progress;
    this->_receive_progress = rhs._receive_progress;
    this->_resoure_fd = rhs._resoure_fd;
    return (*this);
}

/*============================================================================*/
/********************************  Getter  ************************************/
/*============================================================================*/

const std::string&
Response::getStatusCode() const
{
    return (this->_status_code);
}

const std::string&
Response::getStatusMessage(const std::string& code)
{
    return (this->_status_code_table[code]);
}

const location_info&
Response::getLocationInfo() const
{
    return (this->_location_info);
}

const std::string&
Response::getRoute() const
{
    return (this->_route);
}

const std::string&
Response::getResourceAbsPath() const
{
    return (this->_resource_abs_path);
}

const std::string&
Response::getDirectoryEntry() const
{
    return (this->_directory_entry);
}

const struct stat&
Response::getFileInfo() const
{
    return (this->_file_info);
}

const ResType&
Response::getResourceType() const
{
    return (this->_resource_type);
}

const std::string&
Response::getBody() const
{
    return (this->_body);
}

const std::string&
Response::getUriPath() const
{
    return (this->_uri_path);
}

const std::map<std::string, std::string>&
Response::getMimeTypeTable() const
{
    return (this->_mime_type_table);
}

const std::string&
Response::getUriExtension() const
{
    return (this->_uri_extension);
}

int
Response::getCGIPid() const
{
    return (this->_cgi_pid);
}

int
Response::getStdinOfCGI() const
{
    return (this->_stdin_of_cgi);
}

int
Response::getStdoutOfCGI() const
{
    return (this->_stdout_of_cgi);
}

int
Response::getReadFdFromCGI() const
{
    return (this->_read_fd_from_cgi);
}

int
Response::getWriteFdToCGI() const
{
    return (this->_write_fd_to_cgi);
}

size_t
Response::getAlreadyEncodedSize() const
{
    return (this->_already_encoded_size);
}

const std::string&
Response::getTransmittingBody() const
{
    return (this->_transmitting_body);
}

const SendProgress&
Response::getSendProgress() const
{
    return (this->_send_progress);
}

const ReceiveProgress&
Response::getReceiveProgress() const
{
    return (this->_receive_progress);
}

int
Response::getResourceFd() const
{
    return (this->_resoure_fd);
}

const std::map<std::string, std::string>&
Response::getHeaders() const
{
    return (this->_headers);
}

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void
Response::setStatusCode(const std::string& status_code)
{
    this->_status_code = status_code;
}

void
Response::setResourceAbsPath(const std::string& path)
{
    this->_resource_abs_path = path;
}

void
Response::setDirectoryEntry(DIR* dir_ptr)
{
    this->_directory_entry = "";
    struct dirent* entry = NULL;
    while ((entry = readdir(dir_ptr)) != NULL)
    {
        this->_directory_entry += entry->d_name;
        if (entry->d_type == 4)
            this->_directory_entry += "/";
        this->_directory_entry += " ";
    }
}

void
Response::setFileInfo(const struct stat& file_info)
{
    this->_file_info = file_info;
}

void
Response::setResourceType(const ResType& resource_type)
{
    this->_resource_type = resource_type;
}

void
Response::setBody(const std::string& body)
{
    this->_body = body;
}

void
Response::setUriPath(const std::string& uri_path)
{
    this->_uri_path = uri_path;
}

void
Response::setUriExtension(const std::string& extension)
{
    this->_uri_extension = extension;
}

void
Response::setCGIPid(const int pid)
{
    this->_cgi_pid = pid;
}

void
Response::setStdinOfCGI(const int fd)
{
    this->_stdin_of_cgi = fd;
}

void
Response::setStdoutOfCGI(const int fd)
{
    this->_stdout_of_cgi = fd;
}

void
Response::setReadFdFromCGI(const int fd)
{
    this->_read_fd_from_cgi = fd;
}

void
Response::setWriteFdToCGI(const int fd)
{
    this->_write_fd_to_cgi = fd;
}

void
Response::setTransmittingBody(const std::string& transmitting_body)
{
    this->_transmitting_body = transmitting_body;
}

void
Response::setAlreadyEncodedSize(const size_t already_encoded_size)
{
    this->_already_encoded_size = already_encoded_size;
}

void
Response::setSendProgress(const SendProgress send_progress)
{
    this->_send_progress = send_progress;
}

void
Response::setReceiveProgress(const ReceiveProgress receive_progress)
{
    this->_receive_progress = receive_progress;
}

void
Response::setResourceFd(const int resource_fd)
{
    this->_resoure_fd = resource_fd;
}

void
Response::setHeaders(const std::string& key, const std::string& value)
{
    this->_headers[key] = value;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

Response::CannotOpenCGIPipeException::CannotOpenCGIPipeException(Response& response)
: _response(response)
{
    this->_response.setStatusCode("500");
}

const char*
Response::CannotOpenCGIPipeException::what() const throw()
{
    return ("[CODE 500] Cannot Open CGI Pipe.");
}

Response::InvalidCGIMessageException::InvalidCGIMessageException(Response& response)
: _msg("InvalidCGIMessageException: Invalid Response Format: "), _response(response)
{
    this->_response.setStatusCode("500");
}

const char*
Response::InvalidCGIMessageException::what() const throw()
{
    return (this->_msg.c_str());
}

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

void
Response::init()
{
    *this = Response();
}

void
Response::initStatusCodeTable()
{
    this->_status_code_table = {
        {"100", "Continue"},
        {"101", "Switching Protocols"},
        {"102", "Processing"},
        {"200", "OK"},
        {"201", "Created"},
        {"202", "Accepted"},
        {"203", "Non-authoritative Information"},
        {"204", "No Content"},
        {"205", "Reset Content"},
        {"206", "Partial Content"},
        {"207", "Multi-Status"},
        {"208", "Already Reported"},
        {"226", "IM Used"},
        {"300", "Multiple Choices"},
        {"301", "Moved Permanently"},
        {"302", "Found"},
        {"303", "See Other"},
        {"304", "Not Modified"},
        {"305", "Use Proxy"},
        {"307", "Temporary Redirect"},
        {"308", "Permanent Redirect"},
        {"400", "Bad Request"},
        {"401", "Unauthorized"},
        {"402", "Payment Required"},
        {"403", "Forbidden"},
        {"404", "Not found"},
        {"405", "Method Not Allowed"},
        {"406", "Not Acceptable"},
        {"407", "Proxy Authentication Required"},
        {"408", "Required Timeout"},
        {"409", "Conflict"},
        {"410", "Gone"},
        {"411", "Length Required"},
        {"412", "Precondition Failed"},
        {"413", "Request Entity Too Large"},
        {"414", "Request URI Too Long"},
        {"415", "Unsupported Media Type"},
        {"416", "Requested Range Not Satisfiable"},
        {"417", "Expectation Failed"},
        {"418", "IM_A_TEAPOT"},
        {"500", "Internal Server Error"},
        {"501", "Not Implemented"},
        {"502", "Bad Gateway"},
        {"503", "Service Unavailable"},
        {"504", "Gateway Timeout"},
        {"505", "HTTP Version Not Supported"},
        {"506", "Variant Also Negotiates"},
        {"507", "Insufficient Storage"},
        {"508", "Loop Detected"},
        {"510", "Not Extened"},
        {"511", "Network Authentication Required"},
        {"599", "Network Connect Timeout Error"},
    };
}

void
Response::initMimeTypeTable()
{
    this->_mime_type_table = {
        {".aac", "audio/aac"},
        {".abw", "application/x-abiword"},
        {".arc", "application/octet-stream"},
        {".avi", "video/x-msvideo"},
        {".azw", "application/vnd.amazon.ebook"},
        {".bin", "application/octet-stream"},
        {".bz", "application/x-bzip"},
        {".bz2", "application/x-bzip2"},
        {".csh", "application/x-csh"},
        {".css", "text/css"},
        {".csv", "text/csv"},
        {".doc", "application/msword"},
        {".epub", "application/epub+zip"},
        {".gif", "image/gif"},
        {".htm", "text/html"},
        {".html", "text/html"},
        {".ico", "image/x-icon"},
        {".ics", "text/calendar"},
        {".jar", "Temporary Redirect"},
        {".jpeg", "image/jpeg"},
        {".jpg", "image/jpeg"},
        {".js", "application/js"},
        {".json", "application/json"},
        {".mid", "audio/midi"},
        {".midi", "audio/midi"},
        {".mpeg", "video/mpeg"},
        {".mpkg", "application/vnd.apple.installer+xml"},
        {".odp", "application/vnd.oasis.opendocument.presentation"},
        {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
        {".odt", "application/vnd.oasis.opendocument.text"},
        {".oga", "audio/ogg"},
        {".ogv", "video/ogg"},
        {".ogx", "application/ogg"},
        {".pdf", "application/pdf"},
        {".ppt", "application/vnd.ms-powerpoint"},
        {".rar", "application/x-rar-compressed"},
        {".rtf", "application/rtf"},
        {".sh", "application/x-sh"},
        {".svg", "image/svg+xml"},
        {".swf", "application/x-shockwave-flash"},
        {".tar", "application/x-tar"},
        {".tif", "image/tiff"},
        {".tiff", "image/tiff"},
        {".ttf", "application/x-font-ttf"},
        {".vsd", " application/vnd.visio"},
        {".wav", "audio/x-wav"},
        {".weba", "audio/webm"},
        {".webm", "video/webm"},
        {".webp", "image/webp"},
        {".woff", "application/x-font-woff"},
        {".xhtml", "application/xhtml+xml"},
        {".xls", "application/vnd.ms-excel"},
        {".xml", "application/xml"},
        {".xul", "application/vnd.mozilla.xul+xml"},
        {".zip", "application/zip"},
        {".3gp", "video/3gpp audio/3gpp"},
        {".3g2", "video/3gpp2 audio/3gpp2"},
        {".7z", "application/x-7z-compressed"},
    };
}

//NOTE
bool
Response::setRouteAndLocationInfo(const std::string& uri, Server* server)
{
    Log::trace("> setRouteAndLocationInfo");
    timeval from;
    gettimeofday(&from, NULL);

    std::map<std::string, location_info> location_config = server->getLocationConfig();
    std::string route;

    // Log::printLocationConfig(location_config);

    if (uri.length() == 1)
    {
        if (location_config.find("/") != location_config.end())
        {
            this->_route = uri;
            this->_location_info = location_config["/"];
            return (true);
        }
        return (false);
    }
    size_t index = (uri[uri.length() - 1] == '/') ? uri.length() : uri.length() + 1;
    while ((index = uri.find_last_of("/", index - 1)) != std::string::npos)
    {
        route = uri.substr(0, index);
        if (location_config.find(route) != location_config.end())
        {
            this->_route = route;
            this->_location_info = location_config[route];
            return (true);
        }
        if (index == 0)
        {
            if (location_config.find(uri) != location_config.end())
            {
                this->_route = uri;
                this->_location_info = location_config[this->_route];
                break ;
            }
            else
            {
                this->_route = "/";
                this->_location_info = location_config["/"];
                break ;
            }
        }
    }

    Log::printTimeDiff(from);
    Log::trace("< setRouteAndLocationInfo");
    return (false);
}

std::string
Response::makeStatusLine()
{
    Log::trace("> makeStatusLine");
    timeval from;
    gettimeofday(&from, NULL);

    std::string status_line;

    status_line = "HTTP/1.1 ";
    status_line += this->getStatusCode();
    status_line += " ";
    status_line += this->getStatusMessage(this->getStatusCode());
    status_line += "\r\n";

    Log::printTimeDiff(from);
    Log::trace("< makeStatusLine");
    return (status_line);
}

void
Response::appendDateHeader(std::string& headers)
{
    headers += "Date: ";
    headers += ft::getCurrentDateTime();
    headers += "\r\n";
}

void
Response::appendServerHeader(std::string& headers)
{
    headers += "Server: gbp_nginx/0.4\r\n";
}

void
Response::appendAllowHeader(std::string& headers)
{
    const std::vector<const std::string> implemented_methods = {
        "GET",
        "POST",
        "HEAD",
        "PUT",
        "DELETE",
        "CONNECT",
        "OPTIONS",
        "TRACE",
        "PATCH",
    };

    headers += "Allow:";
    if (this->isLimitExceptInLocation())
    {
        for (const std::string& method : implemented_methods)
        {
            if (this->isAllowedMethod(method))
            {
                headers += " ";
                headers += method;
            }
        }
    }
    else
    {
        for (const std::string& method : implemented_methods)
        {
            headers += " ";
            headers += method;
        }
    }
    headers += "\r\n";
}

void
Response::appendContentLanguageHeader(std::string& headers)
{
    //TODO html 외 다른 파일들의 메타데이터는 어찌 처리할지 결정할 것.
    //NOTE: 만약 요청된 resource가 html, htm 확장자가 있는 파일이 아니면 생략한다.
    std::string extension = this->getUriExtension();
    if (!(this->isExtensionExist(extension) 
            && this->isExtensionInMimeTypeTable(extension)
            && this->getMimeTypeTable().at(extension).compare("text/html") == 0))
        return ;

    std::string lang_meta_data = this->getHtmlLangMetaData();
    if (lang_meta_data != "")
    {
        headers += "Content-Language: ";
        headers += lang_meta_data;
        headers += "\r\n";
    }
}

void
Response::appendContentLengthHeader(std::string& headers)
{
    headers += "Content-Length: ";
    headers += std::to_string(this->getTransmittingBody().length());
    headers += "\r\n";
}

void
Response::appendContentLocationHeader(std::string& headers)
{
    headers += "Content-Location: ";
    headers += this->_resource_abs_path;
    headers += "\r\n";
}

void
Response::appendContentTypeHeader(std::string& headers)
{
    Log::trace("> appendContentTypeHeader");
    timeval from;
    gettimeofday(&from, NULL);

    headers += "Content-Type: ";

    if (this->getHeaders().find("Content-Type") != this->getHeaders().end())
        headers += this->getHeaders().at("Content-Type");
    else
    {
        std::string extension = this->getUriExtension();
        if (this->isExtensionExist(extension) && this->isExtensionInMimeTypeTable(extension))
            headers += this->getMimeTypeTable().at(extension);
        else if (this->getResourceType() == ResType::AUTO_INDEX || this->getResourceType() == ResType::INDEX_HTML || this->getResourceType() == ResType::ERROR_HTML)
            headers += "text/html";
        else
            headers += "application/octet-stream";
    }
    headers += "\r\n";

    Log::printTimeDiff(from);
    Log::trace("< appendContentTypeHeader");
}

std::string
Response::getLastModifiedDateTimeOfResource() const
{
    struct tm time;
    time_t mtime = this->_file_info.st_mtime;
    char buf[64];
    const char* fmt = "%a, %d %b %Y %X GMT";

    ft::memset(buf, 0, sizeof(buf));
    strptime(std::to_string(mtime).c_str(), "%s", &time);
    mtime -= ft::getTimeDiffBetweenGMT(time.tm_zone);
    strptime(std::to_string(this->_file_info.st_mtime).c_str(), "%s", &time);
    strftime(buf, sizeof(buf), fmt, &time);
    return (buf);
}

void
Response::appendLastModifiedHeader(std::string& headers)
{
    headers += "Last-Modified: ";
    headers += this->getLastModifiedDateTimeOfResource();
    headers += "\r\n";
}

void
Response::appendLocationHeader(std::string& headers, const Request& request)
{
    headers += "Location: ";
    if (this->getStatusCode() == "201")
        headers += this->getResourceAbsPath();
    else if (this->getHeaders().find("Location") != this->getHeaders().end())
        headers += this->getHeaders().at("Location");
    else
        headers += this->getRedirectUri(request);
    headers += "\r\n";
}

void
Response::appendRetryAfterHeader(std::string& headers, const std::string& status_code)
{
    Log::trace("> appendRetryAfterHeader");
    timeval from;
    gettimeofday(&from, NULL);

    headers += "Retry-After: ";
    if (status_code.compare("503") == 0)
    {
        //NOTE: nginx는 perl script 등을 이용하여 예상복구시간을 동적으로 계산한다. 오버디벨롭이라 판단하여 제외함.
        headers += ft::getEstimatedUnavailableTime();
    }
    else
    {
        //TODO: throw 되지 않도록 예외처리.
        headers += this->getLocationInfo().at("retry_after_sec");
    }
    headers += "\r\n";

    Log::printTimeDiff(from);
    Log::trace("< appendRetryAfterHeader");
}

void
Response::appendTransferEncodingHeader(std::string& headers)
{
    Log::trace("> appendTransferEncodingHeader");
    timeval from;
    gettimeofday(&from, NULL);

    headers += "Transfer-Encoding: chunked\r\n";

    Log::printTimeDiff(from);
    Log::trace("< appendTransferEncodingHeader");
}

void
Response::appendAuthenticateHeader(std::string& headers)
{
    const location_info& location = this->getLocationInfo();
    const std::string& realm = location.at("auth_basic");
    const std::string auth_type = "Basic ";

    headers += "WWW-Authenticate: ";
    headers += auth_type;
    headers += "realm=";
    headers += realm;
    headers += "\r\n";
}

std::string
Response::makeHeaders(Request& request)
{
    Log::trace("> makeHeaders");
    timeval from;
    gettimeofday(&from, NULL);

    std::string headers;
    const std::string& method = request.getMethod();
    
    //TODO 적정 reserve size 구하기
    headers.reserve(200);
    // General headers
    this->appendDateHeader(headers);
    this->appendServerHeader(headers);

    // Entity headers
    if (method == "PUT" && this->getStatusCode().front() == '2')
        headers += "Content-Length: 0\r\n";
    else
    {
        this->appendContentLanguageHeader(headers);
        this->appendContentTypeHeader(headers);
        if (this->isNeedToBeChunkedBody(request))
            this->appendTransferEncodingHeader(headers);
        else
            this->appendContentLengthHeader(headers);
    }

    // Log::printLocationInfo(this->_location_info);

    //TODO switch 문 고려
    std::string status_code = this->getStatusCode();
    if (status_code.compare("200") == 0)
    {
        this->appendContentLocationHeader(headers);
        if (this->getResourceType() == ResType::STATIC_RESOURCE || 
            this->getResourceType() == ResType::INDEX_HTML)
            this->appendLastModifiedHeader(headers);
        if (method == "OPTIONS")
            this->appendAllowHeader(headers);
    }
    else if (status_code.compare("405") == 0)
        this->appendAllowHeader(headers);
    else if (status_code.compare("401") == 0)
    {
        this->appendAuthenticateHeader(headers);
    }
    // else if (status_code.compare("403") == 0)
    // {

    // }
    else if (status_code.compare("201") == 0 || this->isRedirection(status_code))
        this->appendLocationHeader(headers, request);
    else if (status_code.compare("503") == 0 || status_code.compare("429") == 0 
            || status_code.compare("301") == 0)
    {
        this->appendRetryAfterHeader(headers, status_code);
    }
    
    //TODO: connection close 타이밍 확인
    headers += "Connection: close\r\n";
    headers += "\r\n";

    Log::printTimeDiff(from);
    Log::trace("< makeHeaders");
    return (headers);
}

void
Response::makeTraceBody(const Request& request)
{
    std::string body;
    body += request.getMethod();
    body += " ";
    body += request.getUri();
    body += " ";
    body += request.getVersion();
    body += "\r\n";
    const std::map<std::string, std::string>& headers = request.getHeaders();
    for (auto& s : headers)
    {
        const std::string& key = s.first;
        const std::string& value = s.second;
        body += key;
        body += ": ";
        body += value;
        body += "\r\n";
    }
    this->setTransmittingBody(body);
}

void
Response::makeOptionBody()
{
    std::string body;

    body = "OPTIONS!";
    this->setTransmittingBody(body);
}

void
Response::makeBody(Request& request)
{
    Log::trace("> makeBody");
    timeval from;
    gettimeofday(&from, NULL);

    const std::string& method = request.getMethod();

    if (method == "TRACE" && this->getStatusCode() == "200")
        this->makeTraceBody(request);
    else if (method == "OPTIONS" && this->getStatusCode() == "200")
        this->makeOptionBody();
    else if (this->getResourceType() == ResType::AUTO_INDEX)
        PageGenerator::makeAutoIndex(*this);
    else if (this->getStatusCode().front() != '2')
        PageGenerator::makeErrorPage(*this);
    else if (this->isNeedToBeChunkedBody(request))
        this->encodeChunkedBody();
    else
        this->setTransmittingBody(this->getBody());

    Log::printTimeDiff(from);
    Log::trace("< makeBody");
}

bool
Response::isLimitExceptInLocation()
{
    return (this->getLocationInfo().find("limit_except") != this->getLocationInfo().end());
}

bool
Response::isAllowedMethod(const std::string& method)
{
    return (this->_location_info["limit_except"].find(method) != std::string::npos);
}

bool
Response::isExtensionExist(const std::string& extension) const
{
    return (extension != "");
}

bool
Response::isExtensionInMimeTypeTable(const std::string& extension) const
{
    const std::map<std::string, std::string>& mime_type_table = this->getMimeTypeTable();
    // std::cout<<"in isExtensionInMimeTypeTable extension:"<<extension<<std::endl;
    return (mime_type_table.find(extension) != mime_type_table.end());
}


void 
Response::findAndSetUriExtension()
{
    size_t dot = this->getResourceAbsPath().rfind(".");
    if (dot == std::string::npos)
        return ;
    std::string extension = this->getResourceAbsPath().substr(dot);
    this->setUriExtension(extension);
}

bool
Response::isNeedToBeChunkedBody(const Request& request) const
{
    if (request.getVersion().compare("HTTP/1.1") != 0 && request.getVersion().compare("HTTP/2.0") != 0)
        return (false);

    //NOTE: 아래 기준은 임의로 정한 것임.
    if (this->_file_info.st_size > BUFFER_SIZE)
        return (true);
    if (this->getResourceType() == ResType::CGI)
        return (true);
    return (false);
}

bool
Response::isRedirection(const std::string& status_code) const
{
    return (status_code[0] == '3');
}

bool
Response::isLocationToBeRedirected() const
{
    return (this->_location_info.find("return") != this->_location_info.end());
}

std::string
Response::getRedirectStatusCode() const
{
    const std::string& redirection_info = this->_location_info.at("return");

    size_t index = redirection_info.find(" ");
    return (redirection_info.substr(0, index));
}

std::string
Response::getRedirectUri(const Request& request) const
{
    Log::trace("> getRedirectUri");
    timeval from;
    gettimeofday(&from, NULL);
    //TODO: find 실패하지 않도록 invalid 여부는 처음 서버 만들 때 잘 확인할 것.

    const std::string& redirection_info = this->_location_info.at("return");
    std::string redirect_route = redirection_info.substr(redirection_info.find(" "));
    std::string requested_uri = request.getUri();
    size_t offset = requested_uri.find(this->getRoute());
    requested_uri.replace(offset, this->getRoute().length(), redirect_route);

    Log::printTimeDiff(from);
    Log::trace("< getRedirectUri");
    return (requested_uri);
}

std::string
Response::getHtmlLangMetaData() const
{
    const std::string& body = this->getBody();

    size_t html_tag_start;
    size_t html_tag_end;
    if ((html_tag_start = body.find("<html ")) == std::string::npos)
        return ("");
    if ((html_tag_end = body.find(">", html_tag_start)) == std::string::npos)
        return ("");

    std::string html_tag_block = body.substr(html_tag_start, html_tag_end - html_tag_start + 1);

    size_t lang_meta_data_start;
    size_t lang_meta_data_end;
    if ((lang_meta_data_start  = html_tag_block.find("lang=\"")) == std::string::npos)
        return ("");
    if ((lang_meta_data_end = html_tag_block.find("\"", lang_meta_data_start + 6)) == std::string::npos)
        return ("");

    return (html_tag_block.substr(lang_meta_data_start + 6, lang_meta_data_end - (lang_meta_data_start + 6)));
}

void
Response::preparseCGIMessage()
{
    Log::trace("> preparseCGIMessage");
    timeval from;
    gettimeofday(&from, NULL);

    std::string line;
    std::string cgi_message(this->getBody());

    if (ft::substr(line, cgi_message, "\r\n\r\n") == false)
        throw (InvalidCGIMessageException(*this));
    else
    {
        if (this->parseCGIHeaders(line) == false)
            throw (InvalidCGIMessageException(*this));
    }
    this->setBody(cgi_message);
    if (this->getHeaders().find("Status") == this->getHeaders().end())
        throw (InvalidCGIMessageException(*this));
    this->setStatusCode(this->_headers.at("Status").substr(0, 3));

    Log::printTimeDiff(from);
    Log::trace("< preparseCGIMessage");
}

bool
Response::parseCGIHeaders(std::string& cgi_message)
{
    Log::trace("> parseHeaders");
    timeval from;
    gettimeofday(&from, NULL);

    std::string key;
    std::string value;
    std::string line;

    while (ft::substr(line, cgi_message, "\r\n") && !cgi_message.empty())
    {
        if (ft::substr(key, line, ":") == false)
            return (false);
        value = ft::ltrim(line, " ");
        if (this->isValidHeaders(key, value) == false)
            return (false);
        this->setHeaders(key, value);
    }
    if (ft::substr(key, line, ":") == false)
        return (false);
    value = ft::ltrim(line, " ");
    if (this->isValidHeaders(key, value) == false)
        return (false);
    this->setHeaders(key, value);

    Log::printTimeDiff(from);
    Log::trace("< parseHeaders");
    return (true);
}

bool
Response::isValidHeaders(std::string& key, std::string& value)
{
    if (key.empty() || value.empty() ||
        this->isValidSP(key) == false ||
        this->isDuplicatedHeader(key) == false)
        return (false);
    return (true);
}

bool
Response::isValidSP(std::string& str)
{
    if (str.find(" ") == std::string::npos)
        return (true);
    return (false);
}

bool
Response::isDuplicatedHeader(std::string& key)
{
    if (this->_headers.find(key) == this->_headers.end())
        return (true);
    return (false);
}

void
Response::encodeChunkedBody()
{
    Log::trace("> encodeChunkedBody");
    timeval from;
    gettimeofday(&from, NULL);

    const std::string& raw_body = this->getBody(); // raw_body
    size_t already_encoded_size = this->getAlreadyEncodedSize(); // 지금까지 인코딩한 사이즈
    std::string chunked_body; // 청크처리되어 이번에 송신될 body.
    size_t target_size; // 이번 청크처리에서 청크처리할 사이즈.
    size_t raw_body_size = raw_body.length(); // 전체 Body의 사이즈
    size_t substring_size;
    if (raw_body_size - already_encoded_size > BUFFER_SIZE)
        target_size = already_encoded_size + BUFFER_SIZE;
    else
        target_size = raw_body_size;
    while (already_encoded_size < target_size)
    {
        if (already_encoded_size + CHUNKED_LINE_LENGTH > target_size)
            substring_size = target_size - already_encoded_size;
        else
            substring_size = CHUNKED_LINE_LENGTH ;
        chunked_body += ft::itosHex(substring_size);
        chunked_body += "\r\n";
        chunked_body += raw_body.substr(already_encoded_size, substring_size);
        chunked_body += "\r\n";
        already_encoded_size += substring_size;
    }
    if (this->getSendProgress() == SendProgress::DEFAULT)
        this->setSendProgress(SendProgress::CHUNK_START);
    else if (this->getSendProgress() == SendProgress::CHUNK_START)
        this->setSendProgress(SendProgress::CHUNK_PROGRESS);
    if (already_encoded_size == raw_body_size && 
                this->getReceiveProgress() == ReceiveProgress::FINISH)
    {
        chunked_body += "0\r\n\r\n";
        this->setSendProgress(SendProgress::FINISH);
    }

    this->setAlreadyEncodedSize(already_encoded_size);
    this->setTransmittingBody(chunked_body);

    Log::printTimeDiff(from);
    Log::trace("< encodeChunkedBody");
}

void
Response::appendBody(char* buf, int bytes)
{
    this->setBody(this->getBody() + std::string(buf, bytes));
}
