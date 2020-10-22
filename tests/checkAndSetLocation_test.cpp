#include <map>
#include <iostream>
#include <string>

// 1) /one.html          X 마지막에 슬래쉬가 없고, 중간에 슬래쉬가 없다. 
// 2) /one.html/         O 마지막에 슬래쉬가 있다.
// 3) /one/index.html    O
// 4) /one/two/          O
// 5) /one/two           O 마지막에 슬래쉬가 없지만, 중간에 슬래쉬가 있다.
// 6) /                  △ 로케이션에 / 블록이 있는지 먼저 탐색 후 없으면 location이 아니다.

bool checkAndSetLocation(const std::string &uri)
{

    std::map<std::string, std::string> one = { {"root", "/user/iwoo"}, {"index", "html;"}, {"limit_except", "PUT"} };
    std::map<std::string, std::string> two = { {"root", "/user/yohai"}};
    std::map<std::string, std::string> three = { {"root", "/user/sanam"}};
    std::map<std::string, std::map<std::string, std::string>> location_config = { {"/one", one }, {"/one/two", two }, {"/", three} };

    if (uri[0] != '/')
        return (false);

    // std::map<std::string, location_info> location_config = server->getLocationConfig();
    if (uri.length() == 1 && location_config.find("/") != location_config.end())
    {
        // this->_location = "/";
        std::cout<<"router: "<<"/"<<std::endl;
        return (true);
    }
    // size_t index = uri[uri.length() - 1] == '/' ? uri.length() : uri.length() + 1;
    size_t index = uri.length();
    std::string router;
    while ((index = uri.find_last_of("/", index - 1)) != std::string::npos)
    {
        router = uri.substr(0, index);
        if (location_config.find(router) != location_config.end())
        {
            // this->_location = router;
            std::cout<<"router: "<<router<<std::endl;
            return (true);
        }
        if (index == 0)
            break ;
    }
    return (false);
}

// bool isLocationUri(const std::string& uri)
// {
//     if (uri[0] != '/')
//         return (false);

//     std::map<std::string, std::string> one = { {"root", "/user/iwoo"}, {"index", "html;"}, {"limit_except", "PUT"} };
//     std::map<std::string, std::string> two = { {"root", "/user/yohai"}};
//     std::map<std::string, std::string> three = { {"root", "/user/sanam"}};
//     std::map<std::string, std::map<std::string, std::string>> location_config = { {"/one", one }, {"/one/two", two }, {"/", three} };
//     if (uri.length() == 1)
//     {
//         if (location_config.find("/") != location_config.end())
//             return (true);
//         else
//             return (false);
//     }
 
//     if (uri[uri.length() - 1] == '/')
//         return (true);
//     else if (uri.find("/", 1) != std::string::npos)
//         return (true);

//     return (false);
// }

int main()
{
    // 1) /one.html          X 마지막에 슬래쉬가 없고, 중간에 슬래쉬가 없다. 
    // 2) /one.html/         X 마지막에 슬래쉬가 있지만, router table에 없다.
    // 3) /one/index.html    O
    // 4) /one/two/          O
    // 5) /one/two           O 마지막에 슬래쉬가 없지만, 중간에 슬래쉬가 있다.
    // 6) /                  △ 로케이션에 / 블록이 있는지 먼저 탐색 후 없으면 location이 아니다.
    
    std::string uri1 = "/one.html";
    std::string uri2 = "/one.html/";
    std::string uri3 = "/one/index.html";
    std::string uri4 = "/one/two/";
    std::string uri5 = "/one/two"; // 이게 바로 location이라고 판단하면 안됨. router table 검색 후 판단!
    std::string uri6 = "/";
    bool ret;

    std::cout<<"==================="<<std::endl;
    std::cout <<"Case: " << uri1 <<std::endl;
    ret = checkAndSetLocation(uri1);
    std::cout<<"--> Result1: " << ret << std::endl;

    std::cout<<"==================="<<std::endl;
    std::cout <<"Case: " << uri2 << std::endl;
    ret = checkAndSetLocation(uri2);
    std::cout<<"--> Result2: " << ret << std::endl;

    std::cout<<"==================="<<std::endl;
    std::cout <<"Case: " << uri3 << std::endl;
    ret = checkAndSetLocation(uri3);
    std::cout <<"--> Result3: " << ret << std::endl;

    std::cout<<"==================="<<std::endl;
    std::cout <<"Case: " << uri4 << std::endl;
    ret = checkAndSetLocation(uri4);
    std::cout <<"--> Result4: " << ret << std::endl;

    std::cout<<"==================="<<std::endl;
    std::cout <<"Case: " << uri5 << std::endl;
    ret = checkAndSetLocation(uri5);
    std::cout <<"--> Result5: " << ret << std::endl;

    std::cout<<"==================="<<std::endl;
    std::cout <<"Case: " << uri6 << std::endl;
    ret = checkAndSetLocation(uri6);
    std::cout <<"--> Result6: " << ret << std::endl;
}