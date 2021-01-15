#include <string>
#include "PageGenerator.hpp"
#include <algorithm>

void
PageGenerator::makeErrorPage(Response& res)
{
    std::string body;
    const std::string& code = res.getStatusCode();
    const std::string& error_message = res.getStatusMessage(code);

    body.reserve(200);
    // body += "<html>\n\t<head>\n\t\t<title>" + code + " " + message +
    //    "</title>\n\t</head>" + "\n\t<body>\n\t\t<center>\n\t\t\t<h1>" +
    //    code + " " + message + "</h1>\n\t\t</center>" +
    //     "\n\t\t<hr>\n\t<center> ft_nginx </center>\n\t</body>\n</html>";

    const std::string target_code = "<- status_code ->";
    const std::string target_message = "<- error_message ->";
    body += res.getErrorPage();

    size_t index;
    while ((index = body.find(target_code.c_str()) == std::string::npos))
        body.replace(index, target_code.length(), code.c_str());

    while ((index = body.find(target_message.c_str())) == std::string::npos)
        body.replace(index, target_message.length(), error_message.c_str());

    res.setTransmittingBody(body);
    std::cout << "\033[31m\033[01m";
    std::cout << "===============================================" << std::endl;
    std::cout << body << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << "\033[0m";
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
