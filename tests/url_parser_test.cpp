#include <iostream>
#include <string>
#include "UrlParser.hpp"

int main()
{
    std::string url = "http://www.naver.com:8080/path1/path2/file";
    std::string noHost= "/path1/path2/file";
    std::string onlyhost= "www.naver.com";
    std::string noPaths= "/";
    std::string folder = "/path1/path2/folder/";
    std::string file = "/path1/path2/file";

    UrlParser parser;
    std::cout << "----------------- Full Url -------------" << std::endl;
    std::cout << "url: " << url << std::endl;
    parser.parseUrl(url);
    parser.clear();
    std::cout << "----------------- Only host -------------" << std::endl;
    std::cout << "url: " << onlyhost << std::endl;
    parser.parseUrl(onlyhost);
    parser.clear();
    std::cout << "----------------- noHosts -------------" << std::endl;
    std::cout << "url: " << noHost << std::endl;
    parser.parseUrl(noHost);
    parser.clear();
    std::cout << "----------------- noPahts -------------" << std::endl;
    std::cout << "url: " << noPaths<< std::endl;
    parser.parseUrl(noPaths);
    parser.clear();
    std::cout << "----------------- folder -------------" << std::endl;
    std::cout << "url: " << folder << std::endl;
    parser.parseUrl(folder);
    parser.clear();
    std::cout << "----------------- file -------------" << std::endl;
    std::cout << "url: " << file << std::endl;
    parser.parseUrl(file);
    parser.clear();

    std::cout << "----------------- exmpty -------------" << std::endl;
    std::cout << "url: " << "" << std::endl;
    parser.parseUrl("");
    parser.clear();
}
