#include <map>
#include <iostream>
#include <string>

bool setRouteAndLocationInfo(const std::string &uri)
{

    std::map<std::string, std::string> one = { {"root", "/user/iwoo"}, {"index", "html;"}, {"limit_except", "PUT"} };
    std::map<std::string, std::string> two = { {"root", "/user/yohai"}};
    std::map<std::string, std::string> three = { {"root", "/user/sanam"}};
    std::map<std::string, std::map<std::string, std::string>> location_config = { {"/one", one }, {"/one/two", two }, {"/", three} };

    if (uri[0] != '/')
        return (false);

    std::string router;
    if (uri.length() == 1)
    {
        if (location_config.find("/") != location_config.end())
        {
            router = "/";
            std::cout << "True Router: " << router << std::endl;
            return (true);
        }
        else
            return (false);
    }
    size_t index = uri[uri.length() - 1] == '/' ? uri.length() : uri.length() + 1;
    while ((index = uri.find_last_of("/", index - 1)) != std::string::npos)
    {
        router = uri.substr(0, index);
        if (location_config.find(router) != location_config.end())
        {
            std::cout << "True Router: " << router << std::endl;
            return (true);
        }
        if (index == 0)
            break ;
    }
    return (false);
}

int main()
{
    // 1) /one.html          X 마지막에 슬래쉬가 없고, 중간에 슬래쉬가 없다. 
    // 2) /one.html/         X 마지막에 슬래쉬가 있지만, router table에 없다.
    // 3) /one/index.html    △ one이 로케이션에 있다면 true, 없으면 false
    // 4) /one/two/          △ one/two가 로케이션에 있다면 true, 없으면 one을 찾고 있으면 true 없으면 false
    // 5) /one/two           △ one이 로케이션에 있다면 true, 없으면 false
    // 6) /                  △ 로케이션에 / 블록이 있는지 먼저 탐색 후 없으면 location이 아니다.
    
    std::string uri1 = "/one"; // false
    std::string uri2 = "/one/"; // one루트가 있으면 true
    std::string uri3 = "/one/index.html"; // one루트가 있으면 true
    std::string uri4 = "/one/two/"; // one/two 혹은 one/ 루트가 있으면 true
    std::string uri5 = "/one/two"; // one/ 루트가 있으면 true
    std::string uri6 = "/"; // '/' 루트가 있으면 true
    bool ret;

    std::cout<<"==================="<<std::endl;
    std::cout <<"Case: " << uri1 <<std::endl;
    ret = setRouteAndLocationInfo(uri1);
    std::cout<<"--> Result1: " << ret << std::endl;

    std::cout<<"==================="<<std::endl;
    std::cout <<"Case: " << uri2 << std::endl;
    ret = setRouteAndLocationInfo(uri2);
    std::cout<<"--> Result2: " << ret << std::endl;

    std::cout<<"==================="<<std::endl;
    std::cout <<"Case: " << uri3 << std::endl;
    ret = setRouteAndLocationInfo(uri3);
    std::cout <<"--> Result3: " << ret << std::endl;

    std::cout<<"==================="<<std::endl;
    std::cout <<"Case: " << uri4 << std::endl;
    ret = setRouteAndLocationInfo(uri4);
    std::cout <<"--> Result4: " << ret << std::endl;

    std::cout<<"==================="<<std::endl;
    std::cout <<"Case: " << uri5 << std::endl;
    ret = setRouteAndLocationInfo(uri5);
    std::cout <<"--> Result5: " << ret << std::endl;

    std::cout<<"==================="<<std::endl;
    std::cout <<"Case: " << uri6 << std::endl;
    ret = setRouteAndLocationInfo(uri6);
    std::cout <<"--> Result6: " << ret << std::endl;
}