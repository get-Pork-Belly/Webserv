#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <errno.h>
#include <cstdio>
#include "utils.hpp"
#include "Base64.hpp"
#include <termios.h>
#include <filesystem>

std::string error;

enum class Options
{
    START_SERVERS,
    SET_IDPASSWORD,
    QUIT,
    ERROR,
    APPEND_NEW_IDPASSWORD,
    RESET_ALL_IDPASSWORD
};

int
exitWithError(const std::string& message, int exit_number)
{
    std::cerr << "\033[1;31m" << message << "\033[0m" << std::endl; 
    exit(exit_number);
}

int
ft_putstr(std::string msg)
{
    int bytes = write(1, msg.c_str(), msg.length());
    return (bytes);
}

Options
static selectStartingOption()
{
    std::cout<<"\033[104;1;97m"<<"Please select option!"<<"\033[0m"<<std::endl;\
    std::cout<<"1. Start Servers"<<std::endl;
    std::cout<<"2. Set id/password"<<std::endl;
    std::cout<<"3. Quit"<<std::endl;

    while (1)
    {
        std::cout<<"\n\033[1;35m"<<"Put option: "<<"\033[0m"<<std::endl;

        std::string option;
        std::getline(std::cin, option);
        if (option == "1")
            return (Options::START_SERVERS);
        else if (option == "2")
            return (Options::SET_IDPASSWORD);
        else if (option == "3")
            return (Options::QUIT);
        else
            std::cerr<<"Please put valid option! ex) 1"<<std::endl;
    }
    return (Options::QUIT);
}

Options
static selectSettingOption()
{
    std::cout<<"\033[104;1;97m"<<"Please select option!"<<"\033[0m"<<std::endl;\
    std::cout<<"1. Append new id/password"<<std::endl;
    std::cout<<"2. Reset all id/password"<<std::endl;
    std::cout<<"3. Just start servers"<<std::endl;
    std::cout<<"4. Just quit"<<std::endl;

    while (1)
    {
        std::cout<<"\n\033[1;35m"<<"Put option: "<<"\033[0m"<<std::endl;

        std::string option;
        std::getline(std::cin, option);
        if (option == "1")
            return (Options::APPEND_NEW_IDPASSWORD);
        else if (option == "2")
            return (Options::RESET_ALL_IDPASSWORD);
        else if (option == "3")
            return (Options::START_SERVERS);
        else if (option == "4")
            return (Options::QUIT);
        else
            std::cerr<<"Please put valid option! ex) 1"<<std::endl;
    }
    return (Options::QUIT);
}

static int
openWithAppendOption()
{
    std::string pwd = std::__fs::filesystem::current_path();
    std::string password_dir = "/.htpassword/";
    std::cout<<"\n\033[1;35m"<<"Please put file name to append your id/password!"<<"\033[0m"<<std::endl;
    std::string file_name;
    std::getline(std::cin, file_name);

    std::string path = pwd + password_dir + file_name;

    int fd = open(path.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0644);
    if (fd < 0)
    {
        error += "Open Error: ";
        error += strerror(errno);
        throw (error.c_str());
    }
    return (fd);
}

static int
openWithTruncOption()
{
    std::string pwd = std::__fs::filesystem::current_path();
    std::string password_dir = "/.htpassword/";
    std::cout<<"\n\033[1;35m"<<"Please put file name to reset and save your id/password!"<<"\033[0m"<<std::endl;
    std::string file_name;
    std::getline(std::cin, file_name);

    std::string path = pwd + password_dir + file_name;

    int fd = open(path.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd < 0)
    {
        error += "Open Error: ";
        error += strerror(errno);
        throw (error.c_str());
    }
    return (fd);
}

static void
addNewIdPassword(int fd)
{
    std::cout<<"\033[1;35m"<<"Put your ID: "<<"\033[0m";
    std::string id;
    std::getline(std::cin, id);

    //NOTE set terminal mode to hide inputs 
    struct termios t;
    tcgetattr(0, &t);
    t.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &t);
    std::cout<<"\033[1;35m"<<"Put your password: "<<"\033[0m";
    std::string password;
    std::getline(std::cin, password);
    std::cout<<std::endl;

    std::cout<<"\033[1;35m"<<"Re-type your password: "<<"\033[0m";
    for (size_t i = 0; i < 5; i++)
    {
        std::string re_type_password;
        std::getline(std::cin, re_type_password);
        if (re_type_password == password)
            break ;
        std::cout<<"\033[41;1;97m"<<"Password incorect! Put again! (";
        std::cout<<std::to_string(i + 1);
        std::cout<<"/5): "<<"\033[0m"<<std::endl;
        if (i == 5)
            std::cout<<"\033[1;35m"<<"Failed to generate htpasswd file."<<"\033[0m"<<std::endl;
    }
    std::cout<<std::endl;

    std::string id_password = id + ":" + password;
    std::string encoded;
    Base64::encode(id_password, encoded);
    encoded += "\n";
    write(fd, encoded.c_str(), encoded.length());
}

static Options
setIdPassword()
{
    try
    {
        int fd;
        switch (selectSettingOption())
        {
        case Options::APPEND_NEW_IDPASSWORD:
            fd = openWithAppendOption();
            break;
        
        case Options::RESET_ALL_IDPASSWORD:
            fd = openWithTruncOption();
            break;

        case Options::START_SERVERS:
            return (Options::START_SERVERS);
            break;

        case Options::QUIT:
            return (Options::QUIT);
            break;

        default:
            break;
        }
    
        while (true)
        {
            std::cout<<"\033[1;35m"<<"Add new id/password? (y/n)"<<"\033[0m"<<std::endl;

            //NOTE set terminal mode to echo inputs 
            struct termios t;
            tcgetattr(0, &t);
            t.c_lflag ^= ~ECHO;
            t.c_lflag |= ECHO;
            tcsetattr(0, TCSANOW, &t);

            std::string option;
            std::getline(std::cin, option);
            if (option == "y")
                addNewIdPassword(fd);
            else if (option == "n")
                return (Options::START_SERVERS);
            else
                std::cout<<"Invalid input! please put y or n"<<std::endl;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(const char* e)
    {
        std::cerr << e << '\n';
    }
    return (Options::QUIT);
    
}


int main()
{
    Options res = selectStartingOption();

    switch (res)
    {
    case Options::START_SERVERS:
        std::cout<<"\033[104;1;97m"<<"Start Servers! please wait :)"<<"\033[0m"<<std::endl;
        exit(static_cast<int>(Options::START_SERVERS));
        break;
    
    case Options::SET_IDPASSWORD:
        if (setIdPassword() == Options::QUIT)
            exitWithError("Failed to set id/password", 3);
        else
            std::cout<<"\033[104;1;97m"<<"Start Servers! please wait :)"<<"\033[0m"<<std::endl;
        exit(static_cast<int>(Options::START_SERVERS));
        break;
    
    case Options::QUIT:
        exit(static_cast<int>(Options::QUIT));
        break;

    default:
        break;
    }
    return 0;
}
