#include <map>
#include <iostream>
#include <string>

// std::map<std::string, std::string> _location_info = { {"root", "/user/iwoo"}, {"index", "html;"}, {"limit_except", "PUT GET"} };
// std::map<std::string, std::string> _location_info = { {"root", "/user/yohai"}, {"limit_except", "HEAD"}};
std::map<std::string, std::string> _location_info = { {"root", "/user/sanam"}};
// std::map<std::string, std::map<std::string, std::string>> location_info = { {"/one", one }, {"/one/two", two }, {"/", three} };

bool
isAllowedMethod(const std::string& method)
{
    return (_location_info["limit_except"].find(method) != std::string::npos);
}

int main()
{
    std::cout<<"case: /one"<<std::endl;
    std::cout<<"result: "<<isAllowedMethod("GET")<<std::endl;
    std::cout<<"case: /one/two"<<std::endl;
    std::cout<<"result: "<<isAllowedMethod("POST")<<std::endl;
    std::cout<<"case: /three"<<std::endl;
    std::cout<<"result: "<<isAllowedMethod("HEAD")<<std::endl;

    return (0);
}