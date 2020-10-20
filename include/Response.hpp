#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Request.hpp"

class Server;

class Response
{
private:
    std::string _status_code;
    std::string _status_description;
    std::string _headers;
    std::string _transfer_type;
    std::string _clients;
    std::string _message_body;

public:
    /* Constructor */
    Response();
    Response(const Response& other);
    Response(Request& request, Server* server); //TODO 인자를 const로.

    /* Destructor */
    virtual ~Response();

    /* Overload */
    Response& operator=(const Response& rhs);
    /* Getter */
    std::string getStatusCode() const;
    // std::string getStatusDescription() const;
    // std::string getHeaders() const;
    // std::string getTransferType() const;
    // std::string getClients() const;
    /* Setter */
    void setStatusCode(Request& request);
    void setStatusCode(std::string& status_code);
    // void setMessageBody();
    /* Exception */
    /* Util */
};

#endif
