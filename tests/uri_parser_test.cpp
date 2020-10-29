#include <iostream>
#include <string>
#include "UriParser.hpp"

int main()
{
    std::string Uri = "http://www.naver.com:8080/path1/path2/file";
    std::string noHost= "/path1/path2/file";
    std::string onlyhost= "www.naver.com";
    std::string noPaths= "/";
    std::string folder = "/path1/path2/folder/";
    std::string file = "/path1/path2/file";

    UriParser parser;
    std::cout << "----------------- Full Uri -------------" << std::endl;
    std::cout << "Uri: " << Uri << std::endl;
    parser.parseUri(Uri);
    parser.clear();
    std::cout << "----------------- Only host -------------" << std::endl;
    std::cout << "Uri: " << onlyhost << std::endl;
    parser.parseUri(onlyhost);
    parser.clear();
    std::cout << "----------------- noHosts -------------" << std::endl;
    std::cout << "Uri: " << noHost << std::endl;
    parser.parseUri(noHost);
    parser.clear();
    std::cout << "----------------- noPahts -------------" << std::endl;
    std::cout << "Uri: " << noPaths<< std::endl;
    parser.parseUri(noPaths);
    parser.clear();
    std::cout << "----------------- folder -------------" << std::endl;
    std::cout << "Uri: " << folder << std::endl;
    parser.parseUri(folder);
    parser.clear();
    std::cout << "----------------- file -------------" << std::endl;
    std::cout << "Uri: " << file << std::endl;
    parser.parseUri(file);
    parser.clear();

    std::cout << "----------------- exmpty -------------" << std::endl;
    std::cout << "Uri: " << "" << std::endl;
    parser.parseUri("");
    parser.clear();
}
