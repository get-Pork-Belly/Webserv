#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Request.hpp"

class Server;

class Response
{
private:
    std::string _status_code;
    std::map<std::string, std::string> _headers;
    std::string _transfer_type;
    std::string _clients;
    std::string _message_body;
    std::map<std::string, std::string> _status_code_table;

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
    // std::string getHeaders() const;
    // std::string getTransferType() const;
    // std::string getClients() const;
    /* Setter */
    void setStatusCode(Request& request);
    void setStatusCode(const std::string& status_code);
    // void setMessageBody();
    /* Exception */
    /* Util */

    void init();
    void initStatusCodeTable();
    // std::string makeBody(Request& request);
    // std::string makeHeaders(Request& request);
    std::string makeStartLine();
};

#endif
