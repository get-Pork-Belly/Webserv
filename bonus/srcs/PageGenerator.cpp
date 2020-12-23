#include <string>
#include "PageGenerator.hpp"

void
PageGenerator::makeErrorPage(Response& res)
{
    std::string body;
    const std::string& code = res.getStatusCode();
    const std::string& message = res.getStatusMessage(code);

    body.reserve(200);
    body += "<html>\n\t<head>\n\t\t<title>" + code + " " + message +
       "</title>\n\t</head>" + "\n\t<body>\n\t\t<center>\n\t\t\t<h1>" +
       code + " " + message + "</h1>\n\t\t</center>" +
        "\n\t\t<hr>\n\t<center> ft_nginx </center>\n\t</body>\n</html>";
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
