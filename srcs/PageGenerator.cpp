#include "PageGenerator.hpp"

void
PageGenerator::makeErrorPage(Response& res)
{
    std::string body;
    std::string code = res.getStatusCode();
    std::string message = res.getStatusMessage(code);

    body += "<html>\n\t<head>\n\t\t<title>" + code + " " + message +
       "</title>\n\t</head>" + "\n\t<body>\n\t\t<center>\n\t\t\t<h1>" +
       code + " " + message + "</h1>\n\t\t</center>" +
        "\n\t\t<hr>\n\t<center> ft_nginx </center>\n\t</body>\n</html>";
    res.setBody(body);
}

void
PageGenerator::makeAutoIndex(const std::string& error_code,
                const std::string& abs_path, std::string& body)
{
    (void)error_code;
    (void) abs_path;
    (void) body;
}
