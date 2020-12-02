#ifndef UriParser_HPP
# define UriParser_HPP

# include <iostream>
# include <iostream>
# include <vector>
# include <map>
# include "utils.hpp"

class UriParser
{
private:
    UriParser& operator=(const UriParser& rhs);
    UriParser(const UriParser& other);
    /* data */
    size_t _index;
    std::string _uri;
    std::string _scheme;
    std::string _host;
    std::string _port;
    std::string _path;
    std::string _query;
        
public:
    /* Constructor */
    UriParser();

    /* Destructor */
    virtual ~UriParser();

    /* Overload */

    /* Getter */
    int getIndex() const;
    const std::string& getScheme() const;
    const std::string& getHost() const;
    const std::string& getPort() const;
    const std::string& getPath() const;
    const std::string& getQuery() const;
    /* Setter */
    void setIndex(int index);
    void setUri(const std::string& uri);
    void setScheme(const std::string& scheme);
    void setHost(const std::string& host);
    void setPort(const std::string& port);
    void setPath(const std::string& path);
    void setHostAndPort(const std::string& host_port);
    void setQuery(const std::string& query);
    void setPaths();
    /* Exception */
    /* Util */
    void init();
    bool decodingUri(std::string& uri);
    bool parseUri(const std::string& uri);
    std::string findScheme();
    std::string findPort();
    std::string findHostAndPort();
    std::string findPath();
    void print();
    void findAndSetQuery(const std::string& path);

};

#endif
