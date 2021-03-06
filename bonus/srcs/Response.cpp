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
: _status_code("200"), _headers(),
_location_info(), _resource_abs_path(""), _route(""),
_directory_entry(), _resource_type(ResType::NOT_YET_CHECKED), _body(""),
_stdin_of_cgi(DEFAULT_FD), _stdout_of_cgi(DEFAULT_FD), _read_fd_from_cgi(DEFAULT_FD),
_write_fd_to_cgi(DEFAULT_FD),  _cgi_pid(0), _uri_path(""), _uri_extension(""), _transmitting_body(""),
_query(""), _already_encoded_size(0), _parse_progress(ParseProgress::DEFAULT),
_receive_progress(ReceiveProgress::DEFAULT), _resource_fd(DEFAULT_FD),
_sended_response_size(0), _response_message(""), _send_progress(SendProgress::READY),
_temp_buffer(""), _path_info(""), _script_name(""), _path_translated(""),
_request_uri_for_cgi("")
{
    ft::memset(&this->_file_info, 0, sizeof(this->_file_info));
    this->initStatusCodeTable();
    this->initMimeTypeTable();
}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

Response::~Response()
{
}

/*============================================================================*/
/*******************************  Overload  ***********************************/
/*============================================================================*/

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

const std::vector<std::string>&
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
Response::getCgiPid() const
{
    return (this->_cgi_pid);
}

int
Response::getStdinOfCgi() const
{
    return (this->_stdin_of_cgi);
}

int
Response::getStdoutOfCgi() const
{
    return (this->_stdout_of_cgi);
}

int
Response::getReadFdFromCgi() const
{
    return (this->_read_fd_from_cgi);
}

int
Response::getWriteFdToCgi() const
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

const std::string&
Response::getQuery() const
{
    return (this->_query);
}

const ParseProgress&
Response::getParseProgress() const
{
    return (this->_parse_progress);
}

const ReceiveProgress&
Response::getReceiveProgress() const
{
    return (this->_receive_progress);
}

int
Response::getResourceFd() const
{
    return (this->_resource_fd);
}

const std::map<std::string, std::string>&
Response::getHeaders() const
{
    return (this->_headers);
}

int
Response::getSendedResponseSize() const
{
    return (this->_sended_response_size);
}

const std::string&
Response::getResponseMessage() const
{
    return (this->_response_message);
}

const SendProgress&
Response::getSendProgress() const
{
    return (this->_send_progress);
}

const std::string&
Response::getTempBuffer() const
{
    return (this->_temp_buffer);
}

const std::string&
Response::getScriptName() const
{
    return (this->_script_name);
}

const std::string&
Response::getPathInfo() const
{
    return (this->_path_info);
}

const std::string&
Response::getPathTranslated() const
{
    return (this->_path_translated);
}

const std::string&
Response::getRequestUriForCgi() const
{
    return (this->_request_uri_for_cgi);
}

const std::string&
Response::getContentLanguage() const
{
    return (this->_content_language);
}

const std::string&
Response::getErrorPage() const
{
    return (this->_error_page);
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
    this->_directory_entry = ft::makeDirectoryEntry(dir_ptr);
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
Response::setCgiPid(const int pid)
{
    this->_cgi_pid = pid;
}

void
Response::setStdinOfCgi(const int fd)
{
    this->_stdin_of_cgi = fd;
}

void
Response::setStdoutOfCgi(const int fd)
{
    this->_stdout_of_cgi = fd;
}

void
Response::setReadFdFromCgi(const int fd)
{
    this->_read_fd_from_cgi = fd;
}

void
Response::setWriteFdToCgi(const int fd)
{
    this->_write_fd_to_cgi = fd;
}

void
Response::setTransmittingBody(const std::string& transmitting_body)
{
    this->_transmitting_body = transmitting_body;
}

void
Response::setQuery(const std::string& query)
{
    this->_query = query;
}

void
Response::setAlreadyEncodedSize(const size_t already_encoded_size)
{
    this->_already_encoded_size = already_encoded_size;
}

void
Response::setParseProgress(const ParseProgress parse_progress)
{
    this->_parse_progress = parse_progress;
}

void
Response::setReceiveProgress(const ReceiveProgress receive_progress)
{
    this->_receive_progress = receive_progress;
}

void
Response::setResourceFd(const int resource_fd)
{
    this->_resource_fd = resource_fd;
}

void
Response::setHeaders(const std::string& key, const std::string& value)
{
    this->_headers[key] = value;
}

void
Response::setContentLanguage(const std::string& content_language)
{
    this->_content_language = content_language;
}

void
Response::setSendedResponseSize(const int sended_response_size)
{
    this->_sended_response_size = sended_response_size;
}

void
Response::setResponseMessage(const std::string& response_message)
{
    this->_response_message = response_message;
}

void
Response::setSendProgress(const SendProgress& send_progress)
{
    this->_send_progress = send_progress;
}

void
Response::setTempBuffer(const std::string& temp_buffer)
{
    this->_temp_buffer = temp_buffer;
}

void
Response::setScriptName(const std::string& script_name)
{
    this->_script_name = script_name;
}

void
Response::setPathInfo(const std::string& path_info)
{
    this->_path_info = path_info;
}

void
Response::setPathTranslated(const std::string& path_translated)
{
    this->_path_translated = path_translated;
}

void
Response::setRequestUriForCgi(const std::string& request_uri_for_cgi)
{
    this->_request_uri_for_cgi = request_uri_for_cgi;
}

void
Response::setCgiEnvpValues()
{
    const std::string& abs_path = this->getResourceAbsPath();
    const std::string& cgi_extension = this->getUriExtension();
    size_t index;
    if ((index = abs_path.find(cgi_extension)) != std::string::npos)
    {
        std::string script_name = this->getResourceAbsPath().substr(0, index + cgi_extension.length());
        if (ft::fileExists(script_name))
        {
            this->setScriptName(script_name);
            this->setPathInfo(abs_path.substr(index + cgi_extension.length(), abs_path.length()));
            this->setPathTranslated(this->getLocationInfo().at("root") + this->getPathInfo());
            this->setRequestUriForCgi(this->getUriPath());
        }
        else
        {
            if (this->_location_info.find("cgi_path") != this->_location_info.end())
                script_name = this->_location_info.at("cgi_path");
            else
                throw (CannotSetCgiScriptNameException(*this));
            //NOTE: to deal with 42 cgi tester
            this->setScriptName(script_name);
            this->setPathInfo(script_name);
            this->setPathTranslated(script_name);
            this->setRequestUriForCgi(script_name);
        }
    }
}

void
Response::setErrorPage(const std::string& error_page)
{
    this->_error_page = error_page;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

Response::CannotSetCgiScriptNameException::CannotSetCgiScriptNameException(Response& response)
: _response(response)
{
    this->_response.setStatusCode("500");
}

const char*
Response::CannotSetCgiScriptNameException::what() const throw()
{
    return ("[CODE 500] Cannot Set script name. check config file's 'cgi_path'!");
}

Response::CannotOpenCgiPipeException::CannotOpenCgiPipeException(Response& response)
: _response(response)
{
    this->_response.setStatusCode("500");
}

const char*
Response::CannotOpenCgiPipeException::what() const throw()
{
    return ("[CODE 500] Cannot Open CGI Pipe.");
}

Response::InvalidCgiMessageException::InvalidCgiMessageException(Response& response)
: _msg("InvalidCgiMessageException: Invalid Response Format: "), _response(response)
{
    this->_response.setStatusCode("500");
}

const char*
Response::InvalidCgiMessageException::what() const throw()
{
    return (this->_msg.c_str());
}

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

void
Response::init()
{
    this->_status_code = "200";
    this->_headers = {};
    this->_location_info = {};
    this->_resource_abs_path = "";
    this->_route = "";
    this->_directory_entry = {};
    this->_resource_type = ResType::NOT_YET_CHECKED;
    ft::memset(&this->_file_info, 0, sizeof(this->_file_info));
    this->_body = "";
    this->_stdin_of_cgi = DEFAULT_FD;
    this->_stdout_of_cgi = DEFAULT_FD;
    this->_read_fd_from_cgi = DEFAULT_FD;
    this->_write_fd_to_cgi = DEFAULT_FD;
    this->_cgi_pid = 0;
    this->_uri_path = "";
    this->_uri_extension = "";
    this->_transmitting_body = "";
    this->_query = "";
    this->_already_encoded_size = 0;
    this->_parse_progress = ParseProgress::DEFAULT;
    this->_receive_progress = ReceiveProgress::DEFAULT;
    this->_resource_fd = DEFAULT_FD;
    this->_sended_response_size = 0;
    this->_response_message = "";
    this->_send_progress = SendProgress::READY;
    this->_temp_buffer = "";
    this->_script_name = "";
    this->_path_info = "";
    this->_path_translated = "";
    this->_request_uri_for_cgi = "";
    //NOTE: _status_code_table, _mime_type_table은 초기화 대상 아님. 값이 바뀌지 않으며 초기화시 성능저하 우려되기 때문.
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

bool
Response::isMatchedRegexRoute(Server* server, const std::vector<std::pair<std::regex, std::string> >& regex_vector, const std::string& route)
{
    const std::map<std::string, location_info>& location_config = server->getLocationConfig();

    for (auto& regex_pair : regex_vector)
    {
        if (std::regex_match(route, regex_pair.first))
        {
            this->_route = route;
            this->_location_info = location_config.at(regex_pair.second);
            return (true);
        }
    }
    return (false);
}

bool
Response::setRouteAndLocationInfo(const std::string& uri, Server* server)
{
    Log::trace("> setRouteAndLocationInfo", 2);
    timeval from;
    gettimeofday(&from, NULL);

    std::map<std::string, location_info> location_config = server->getLocationConfig();
    std::string route;

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

    std::vector<std::pair<std::regex, std::string> > regex_vector;
    for (auto& kv : location_config)
        regex_vector.push_back(std::make_pair(std::regex(kv.first), kv.first));

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
            if (this->isMatchedRegexRoute(server, regex_vector, uri))
                return (true);
            this->_route = "/";
            this->_location_info = location_config["/"];
            return (true);
        }
        else
        {
            if (this->isMatchedRegexRoute(server, regex_vector, route))
                return (true);
        }
    }

    Log::printTimeDiff(from, 2);
    Log::trace("< setRouteAndLocationInfo", 2);
    return (false);
}

std::string
Response::makeStatusLine()
{
    Log::trace("> makeStatusLine", 2);
    timeval from;
    gettimeofday(&from, NULL);

    std::string status_line;

    status_line = "HTTP/1.1 ";
    status_line += this->getStatusCode();
    status_line += " ";
    status_line += this->getStatusMessage(this->getStatusCode());
    status_line += "\r\n";

    Log::printTimeDiff(from, 2);
    Log::trace("< makeStatusLine", 2);
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
Response::appendConnectionHeader(std::string& headers, Request& request)
{
    //NOTE: 아래 경우를 제외하고는 keep-alive
    //      1. status code가 200이 아닌 경우.
    //      2. Request에 포함된 Connection header 값이 close인 경우.
    if (this->getStatusCode() != "200" || request.isConnectionHeaderClose())
        headers += "Connection: close\r\n";
    else
        headers += "Connection: keep-alive\r\n";
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
        // "CONNECT", not implemented
        "OPTIONS",
        "TRACE",
        // "PATCH", not implemented
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

bool
Response::isContentTypeTextHtml() const
{
    if ((this->ExtensionExists(this->_uri_extension) 
            && this->isExtensionInMimeTypeTable(this->_uri_extension)
            && this->getMimeTypeTable().at(this->_uri_extension).compare("text/html") == 0))
        return (true);
    return (false);
}

void
Response::appendContentLanguageHeader(std::string& headers)
{
    // NOTE: 만약 요청된 resource가 html, htm 확장자가 있는 파일이 아니면, 
    //       굳이 메타데이터를 추출하지 않도록 구현되어있음.
    if (this->isContentTypeTextHtml())
    {
        std::string lang_meta_data = this->getHtmlLangMetaData();
        if (lang_meta_data != "")
        {
            headers += "Content-Language: ";
            headers += lang_meta_data;
            headers += "\r\n";
            return ;
        }
    }
    headers += "Content-Language: ";
    headers += this->getContentLanguage();
    headers += "\r\n";
}

void
Response::appendContentLengthHeader(std::string& headers, const std::string& method)
{
    const std::string& status_code = this->getStatusCode();
    if (status_code == "204" ||
        status_code.front() == '1' ||
        (method == "CONNECT" && status_code.front() == '2'))
        return ;
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
    Log::trace("> appendContentTypeHeader", 2);
    timeval from;
    gettimeofday(&from, NULL);

    headers += "Content-Type: ";

    if (this->getHeaders().find("Content-Type") != this->getHeaders().end())
        headers += this->getHeaders().at("Content-Type");
    else
    {
        std::string extension = this->getUriExtension();
        if (this->getResourceType() == ResType::AUTO_INDEX || this->getResourceType() == ResType::ERROR_HTML)
            headers += "text/html";
        else if (this->ExtensionExists(extension) && this->isExtensionInMimeTypeTable(extension))
            headers += this->getMimeTypeTable().at(extension);
        else
            headers += "application/octet-stream";
    }
    headers += "\r\n";

    Log::printTimeDiff(from, 2);
    Log::trace("< appendContentTypeHeader", 2);
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
    Log::trace("> appendRetryAfterHeader", 2);
    timeval from;
    gettimeofday(&from, NULL);

    headers += "Retry-After: ";
    if (status_code.compare("503") == 0)
        headers += ft::getEstimatedUnavailableTime();
    else
        headers += this->getLocationInfo().at("retry_after_sec");
    headers += "\r\n";

    Log::printTimeDiff(from, 2);
    Log::trace("< appendRetryAfterHeader", 2);
}

void
Response::appendTransferEncodingHeader(std::string& headers, const std::string& method)
{
    Log::trace("> appendTransferEncodingHeader", 2);
    timeval from;
    gettimeofday(&from, NULL);

    if (this->getStatusCode() == "204" ||
        this->getStatusCode().front() == '1' ||
        (method == "CONNECT" && this->getStatusCode().front() == '2'))
        return ;
    headers += "Transfer-Encoding: chunked\r\n";

    Log::printTimeDiff(from, 2);
    Log::trace("< appendTransferEncodingHeader", 2);
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

void
Response::appendGeneralHeaders(std::string& headers, Request& request)
{
    this->appendDateHeader(headers);
    this->appendServerHeader(headers);
    if (request.getVersion() != "HTTP/2.0")
        this->appendConnectionHeader(headers, request);
}

void
Response::appendEntityHeaders(std::string& headers, Request& request)
{
    const std::string& method = request.getMethod();
    if ((method == "PUT" || method == "DELETE") && this->getStatusCode().front() == '2')
    {
        headers += "Content-Length: 0\r\n";
        return ;
    }

    if (this->isNeedToBeChunkedBody(request))
        this->appendTransferEncodingHeader(headers, method);
    else
        this->appendContentLengthHeader(headers, method);
    this->appendContentLanguageHeader(headers);
    this->appendContentTypeHeader(headers);

    const std::string& status_code = this->getStatusCode();
    if (status_code == "200")
    {
        if (method == "OPTIONS")
            this->appendAllowHeader(headers);
        this->appendContentLocationHeader(headers);
    }
    else if (status_code == "405")
        this->appendAllowHeader(headers);
}

void
Response::appendResponseHeaders(std::string& headers, Request& request)
{
    const std::string& status_code = this->getStatusCode();
    if (status_code == "200" && this->getResourceType() == ResType::STATIC_RESOURCE)
        this->appendLastModifiedHeader(headers);
    else if (status_code == "401")
        this->appendAuthenticateHeader(headers);
    else if (status_code == "201" || this->isRedirection(status_code))
        this->appendLocationHeader(headers, request);
    else if (status_code == "503" || status_code == "429" || status_code == "301")
        this->appendRetryAfterHeader(headers, status_code);
}

std::string
Response::makeHeaders(Request& request)
{
    Log::trace("> makeHeaders", 2);
    timeval from;
    gettimeofday(&from, NULL);

    std::string headers;
    
    headers.reserve(400);
    this->appendGeneralHeaders(headers, request);
    this->appendEntityHeaders(headers, request);
    this->appendResponseHeaders(headers, request);
    headers += "\r\n";

    Log::printTimeDiff(from, 2);
    Log::trace("< makeHeaders", 2);
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
    Log::trace("> makeBody", 2);
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

    Log::printTimeDiff(from, 2);
    Log::trace("< makeBody", 2);
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
Response::ExtensionExists(const std::string& extension) const
{
    return (extension != "");
}

bool
Response::isExtensionInMimeTypeTable(const std::string& extension) const
{
    const std::map<std::string, std::string>& mime_type_table = this->getMimeTypeTable();
    return (mime_type_table.find(extension) != mime_type_table.end());
}


void 
Response::findAndSetUriExtension()
{
    if (this->_location_info.find("cgi") != this->_location_info.end())
    {
        std::vector<std::string> cgi_extensions =  ft::split(this->_location_info.at("cgi"), " ");

        const std::string& abs_path = this->getResourceAbsPath();
        size_t index = std::string::npos;
        for (std::string& cgi_extension : cgi_extensions)
        {
            if ((index = abs_path.find(cgi_extension)) != std::string::npos)
            {
                this->setUriExtension(cgi_extension);
                return ;
            }
        }
    }
    size_t dot = this->getResourceAbsPath().rfind(".");
    if (dot == std::string::npos)
        return ;
    std::string extension = this->getResourceAbsPath().substr(dot);
    if (extension == ".")
        return ;
    this->setUriExtension(extension);
}

std::vector<std::string>
Response::makeLanguageWeightTable(const std::string& accept_languages)
{
    std::vector<std::pair<double, std::string>> weight_and_languages;
    std::vector<std::string> tmp = ft::split(accept_languages, ",");
    size_t count = tmp.size();
    size_t index;
    for (std::string& s : tmp)
    {
        s = ft::ltrim(s);
        if ((index = s.find(";q=")) != std::string::npos)
        {
            double q = std::stod(s.substr(index + 3));
            weight_and_languages.push_back(make_pair(q, s.substr(0, index)));
        }
        else
            weight_and_languages.push_back(make_pair(count--, s));
        
    }

    sort(weight_and_languages.begin(), weight_and_languages.end());

    std::vector<std::string> language_weight_table;
    size_t i = weight_and_languages.size();
    while (i--)
        language_weight_table.push_back(weight_and_languages[i].second);
    
    return (language_weight_table);
}

void
Response::negotiateContent(const std::string& accept_language)
{
    std::vector<std::string> language_weight_table = this->makeLanguageWeightTable(accept_language);

    size_t index;
    for (std::string& language : language_weight_table)
    {
        //NOTE: en is default.
        if (language == "en")
            return ;

        std::string resource_abs_path = this->getResourceAbsPath();
        if (this->getUriExtension() != "")
            index = resource_abs_path.rfind(this->getUriExtension());
        else
            index = resource_abs_path.length();
        
        std::string target_content_path = resource_abs_path.substr(0, index) + "_" + language + this->getUriExtension();
        
        if (ft::fileExists(target_content_path))
        {
            this->setResourceAbsPath(target_content_path);
            this->setContentLanguage(language);
            return ;
        }
    }
}

bool
Response::isNeedToBeChunkedBody(const Request& request) const
{
    if (request.getVersion().compare("HTTP/1.1") != 0 && request.getVersion().compare("HTTP/2.0") != 0)
        return (false);

    if (this->_file_info.st_size > BUFFER_SIZE && request.getMethod() != "PUT")
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
    Log::trace("> getRedirectUri", 2);
    timeval from;
    gettimeofday(&from, NULL);

    if (this->_location_info.find("return") == this->_location_info.end())
        return ("/");
    const std::string& redirection_info = this->_location_info.at("return");
    std::string redirect_route = redirection_info.substr(redirection_info.find(" "));
    std::string requested_uri = request.getUri();
    size_t offset = requested_uri.find(this->getRoute());
    requested_uri.replace(offset, this->getRoute().length(), redirect_route);

    Log::printTimeDiff(from, 2);
    Log::trace("< getRedirectUri", 2);
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

bool
Response::findEndOfHeaders()
{
    const std::string& temp_buffer = this->getTempBuffer();

    if (temp_buffer.find("\r\n\r\n") != std::string::npos)
        return (true);
    return (false);
}

void
Response::preparseCgiMessage()
{
    Log::trace("> preparseCgiMessage", 1);
    timeval from;
    gettimeofday(&from, NULL);

    std::string line;
    std::string cgi_message(this->getTempBuffer());

    if (ft::substr(line, cgi_message, "\r\n\r\n") == false)
        throw (InvalidCgiMessageException(*this));
    else
    {
        if (this->parseCgiHeaders(line) == false)
            throw (InvalidCgiMessageException(*this));
    }
    this->setTempBuffer("");
    this->setBody(cgi_message);
    if (this->getHeaders().find("Status") == this->getHeaders().end())
        throw (InvalidCgiMessageException(*this));
    this->setStatusCode(this->_headers.at("Status").substr(0, 3));

    Log::printTimeDiff(from, 1);
    Log::trace("< preparseCgiMessage", 1);
}

bool
Response::parseCgiHeaders(std::string& cgi_message)
{
    Log::trace("> parseCgiHeaders", 1);
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

    Log::printTimeDiff(from, 1);
    Log::trace("< parseCgiHeaders", 1);
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

bool
Response::isFileInDirEntry(std::string& index)
{
    for (const std::string& entry : this->_directory_entry)
    {
        if (entry == index)
            return (true);
    }
    return (false);
}

void
Response::encodeChunkedBody()
{
    Log::trace("> encodeChunkedBody", 1);
    timeval from;
    gettimeofday(&from, NULL);

    const std::string& raw_body = this->getBody();
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
    if (this->getParseProgress() == ParseProgress::DEFAULT)
        this->setParseProgress(ParseProgress::CHUNK_START);
    else if (this->getParseProgress() == ParseProgress::CHUNK_START)
        this->setParseProgress(ParseProgress::CHUNK_PROGRESS);
    if (already_encoded_size == raw_body_size && 
                this->getReceiveProgress() == ReceiveProgress::FINISH)
    {
        chunked_body += "0\r\n\r\n";
        this->setParseProgress(ParseProgress::FINISH);
    }

    this->setAlreadyEncodedSize(already_encoded_size);
    this->setTransmittingBody(chunked_body);

    Log::printTimeDiff(from, 1);
    Log::trace("< encodeChunkedBody", 1);
}

void
Response::appendBody(char* buf, int bytes)
{
    this->_body.append(buf, bytes);
}

void
Response::appendTempBuffer(char* buf, int bytes)
{
    this->_temp_buffer.append(buf, bytes);
}

void
Response::trimPhpCgiFirstHeadersFromTempBuffer()
{
    size_t index = this->getTempBuffer().find("\r\n\r\n");
    this->setTempBuffer(this->getTempBuffer().substr(index + CRLF_SIZE + CRLF_SIZE));
}

bool
Response::isCgiWritePipeNotClosed() const
{
    return (this->_write_fd_to_cgi != DEFAULT_FD);
}

bool
Response::isCgiReadPipeNotClosed() const
{
    return (this->_read_fd_from_cgi != DEFAULT_FD);
}

bool
Response::isResourceNotClosed() const
{
    return (this->_resource_fd != DEFAULT_FD);
}
