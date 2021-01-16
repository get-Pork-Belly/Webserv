#include <string>
#include "PageGenerator.hpp"

void
PageGenerator::makeErrorPage(Response& res)
{
    std::string body;
    const std::string& code = res.getStatusCode();
    const std::string& error_message = res.getStatusMessage(code);

    const std::string target_code = "<- status_code ->";
    const std::string target_message = "<- error_message ->";
    
    body = res.getErrorPage();

    size_t pos = 0;
    size_t offset = 0;
    while ((pos = body.find(target_code, offset)) != std::string::npos)
    {
        body.replace(body.begin() + pos, body.begin() + pos + target_code.length(), code);
        offset = pos + code.length();
    }
    pos = 0;
    offset = 0;
    while ((pos = body.find(target_message, offset)) != std::string::npos)
    {
        body.replace(body.begin() + pos, body.begin() + pos + target_message.length(), error_message);
        offset = pos + error_message.length();
    }

    res.setTransmittingBody(body);
}

void
PageGenerator::makeAutoIndex(Response& res)
{
    std::string temp;
    std::string body;
    const std::vector<std::string>& files = res.getDirectoryEntry();

    body.reserve(300);
    body += "<html>\n\t<head>\n\t\t<title>Index of " + res.getRoute() +
        "</title>\n\t</head>\n\t<body>\n\t\t<h1>Index of" + res.getRoute() +
        "</h1>\n\t\t<hr>\n\t\t<pre>";
    for (auto& file : files)
        body += "\n\t\t\t<a href=\""+ res.getUriPath() + file +"\">" + file + "</a>";
    body += "\n\t\t</pre>\n\t\t<hr>\n\t</body>\n</html>";
    res.setTransmittingBody(body);
}
