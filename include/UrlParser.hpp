#ifndef URLPARSER_HPP
# define URLPARSER_HPP

# include <iostream>
# include <iostream>
# include <vector>
# include <map>
# include "utils.hpp"

class UrlParser
{
private:
    UrlParser& operator=(const UrlParser& rhs);
    UrlParser(const UrlParser& other);
    /* data */
    size_t _index;
    std::string _url;
    std::string _scheme;
    std::string _host;
    std::string _port;
    std::string _path;
    std::vector<std::string> _paths;
        
public:
    /* Constructor */
    UrlParser();

    /* Destructor */
    virtual ~UrlParser();

    /* Overload */

    /* Getter */
    int getIndex() const;
    const std::string& getScheme() const;
    const std::string& getHost() const;
    const std::string& getPort() const;
    const std::string& getPath() const;
    const std::vector<std::string>& getPaths() const;
    /* Setter */
    void setIndex(int index);
    void setUrl(const std::string& url);
    void setScheme(const std::string& scheme);
    void setHost(const std::string& host);
    void setPort(const std::string& port);
    void setPath(const std::string& path);
    void setHostAndPort(const std::string& host_port);
    void setPaths();
    /* Exception */
    /* Util */
    void parseUrl(const std::string& url);
    std::string findScheme();
    std::string findPort();
    std::string findHostAndPort();
    std::string findPath();
    void clear();
    void print();
};

#endif
