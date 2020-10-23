#include <map>
#include <iostream>
#include <string>

std::string _location;
std::map<std::string, std::string> one = { {"root", "/user/iwoo"}, {"index", "html;"}, {"limit_except", "PUT"} };
std::map<std::string, std::string> two = { {"root", "/user/yohai"}};
std::map<std::string, std::string> three = { {"root", "/user/sanam"}};
std::map<std::string, std::map<std::string, std::string>> location_config = { {"/one", one }, {"/one/two", two }, {"/", three} };

bool isLimitExceptInLocation(void)
{
    // std::map<std::string, location_info> location_config = server->getLocationConfig();
    // return (location_config[this->getLocation()].find("limit_except") != location_config.end());
    return (location_config[_location].find("limit_except") != location_config[_location].end());
}

int main()
{
    _location = "/one";
    std::cout<<"case: /one"<<std::endl;
    std::cout<<"result: "<<isLimitExceptInLocation()<<std::endl;
    _location = "/one/two";
    std::cout<<"case: /one/two"<<std::endl;
    std::cout<<"result: "<<isLimitExceptInLocation()<<std::endl;
    _location = "/three";
    std::cout<<"case: /three"<<std::endl;
    std::cout<<"result: "<<isLimitExceptInLocation()<<std::endl;

    return (0);
}