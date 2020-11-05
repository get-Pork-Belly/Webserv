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

Response::Response()
: _status_code("200"), _transfer_type(""), _clients(""), _body(""), 
_uri_extension("")
{
    this->_headers = { {"", ""} };
    ft::memset(&this->_file_info, 0, sizeof(this->_file_info));
    this->initStatusCodeTable();
    this->initMimeTypeTable();
}

Response::Response(const Response& other)
: _status_code(other._status_code),  _headers(other._headers),
_transfer_type(other._transfer_type), _clients(other._clients),
_status_code_table(other._status_code_table), 
_mime_type_table(other._mime_type_table), _body(other._body),
_uri_extension(other._uri_extension) {}

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
    this->_body= rhs._body;
    this->_status_code_table = rhs._status_code_table;
    this->_mime_type_table = rhs._mime_type_table;
    this->_uri_extension = rhs._uri_extension;
    return (*this);
}

/*============================================================================*/
/********************************  Getter  ************************************/
/*============================================================================*/

std::string 
Response::getStatusCode() const
{
    return (this->_status_code);
}

std::string
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
Response::setUriExtension(const std::string& extension)
{
    this->_uri_extension = extension;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

void
Response::init()
{
    this->_status_code = "200";
    this->_headers = { {"", ""} };
    this->_transfer_type = "";
    this->_clients = "";
    this->_body = "";
    this->_location_info = { {"", ""} };
    this->_resource_abs_path = "";
    this->_route = "";
    this->_directory_entry = "";
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

//TODO: Response에 상태코드 세팅하게 변경하기.
void
Response::applyAndCheckRequest(Request& request, Server* server)
{
    Log::trace("> applyAndCheckRequest");
    if (this->setRouteAndLocationInfo(request.getUri(), server))
    {
        if (this->isLimitExceptInLocation() && this->isAllowedMethod(request.getMethod()) == false)
            this->setStatusCode("405");
    }
    Log::trace("< applyAndCheckRequest");
}

//NOTE
bool
Response::setRouteAndLocationInfo(const std::string& uri, Server* server)
{
    Log::trace("> setRouteAndLocationInfo");
    std::map<std::string, location_info> location_config = server->getLocationConfig();
    std::string route;

    Log::printLocationConfig(location_config);

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
            this->_route = "/";
            this->_location_info = location_config["/"];
            break ;
        }
    }
    Log::trace("< setRouteAndLocationInfo");
    return (false);
}

std::string
Response::makeStatusLine()
{
    Log::trace("> makeStatusLine");
    std::string status_line;

    status_line = "HTTP/1.1 ";
    status_line += this->getStatusCode();
    status_line += " ";
    status_line += this->getStatusMessage(this->getStatusCode());
    status_line += "\r\n";
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

// std::string
// Response::appendAllowHeader(std::string& headers)
// {
//     if (this->isLimitExceptInLocation())
//     {
//         std::string header = "Allow:";

//         for (auto& method : this->_implemented_methods)
//         {
//             if (this->isAllowedMethod(method))
//             {
//                 header += " ";
//                 header += method;
//             }
//             header += "\r\n";
//             return (header);
//         }
//     }
//     else
//         //NOTE: default method
//         return ("Allow: GET HEAD");
// }

void
Response::appendContentLengthHeader(std::string& headers)
{
    headers += "Content-Length: ";
    headers += std::to_string(this->getBody().length());
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
    headers += "Content-Type: ";
    std::string extension = this->getUriExtension();
    if (this->isExtensionExist(extension) && this->isExtensionInMimeTypeTable(extension))
        headers += this->getMimeTypeTable().at(extension);
    else
        headers += "application/octet-stream";
    headers += "\r\n";
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

std::string
Response::makeHeaders(Request& request)
{
    (void)request;
    std::string headers;
    
    //TODO 적정 reserve size 구하기
    headers.reserve(200);
    this->appendDateHeader(headers);
    this->appendServerHeader(headers);
    // if chunked 
    // headers += this->makeTransferEncodingHeader();
    // if not chunked
    this->appendContentLengthHeader(headers);
    this->appendContentLocationHeader(headers);
    this->appendContentTypeHeader(headers);

    //TODO switch 문 고려
    std::string status_code = this->getStatusCode();
    if (status_code.compare("200") == 0)
        this->appendLastModifiedHeader(headers);
    else if (status_code.compare("405") == 0)
    {
        // this->appendAllowHeader(headers);
    }
    else if (status_code.compare("401") == 0)
    {
        // this->appendAuthenticateHeader(headers);
    }
    else if (status_code.compare("201") == 0 || this->isRedirection(status_code))
    {
        // this->appendLocationHeader(headers);
    }
    else if (status_code.compare("503") == 0 || status_code.compare("429") == 0 
            || status_code.compare("301") == 0)
    {
        // this->appendRetryAfterHeader(headers);
    }
    
    headers += "\r\n";
    return (headers);
}

void
Response::makeBody(Request& request)
{
    Log::trace("> makeBody");
    (void)request;
    if (this->getResourceType() == ResType::AUTO_INDEX)
        PageGenerator::makeAutoIndex(*this);
    else if (this->getStatusCode().front() != '2')
        PageGenerator::makeErrorPage(*this);
    else // 일반적인 body
    {
    }
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
    std::cout<<"in isExtensionInMimeTypeTable extension:"<<extension<<std::endl;
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
Response::isRedirection(const std::string& status_code) const
{
    return (status_code[0] == '3');
}

void
Response::appendBody(char* buf)
{
    std::string tmp = this->getBody();
    std::string tmp2(buf);

    std::string ret = tmp + tmp2;
    this->setBody(ret);
}