#include "ServerGenerator.hpp"
#include "ServerManager.hpp"

int main()
{
    try
    {
        ServerManager sm("./tests/config_testfile");
        // ServerGenerator sg(&sm);
    }
    catch(const char* e)
    {
        std::cout<<e<<std::endl;
    }
    
    return 0;
}
