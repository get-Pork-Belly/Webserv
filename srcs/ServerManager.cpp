#include "ServerManager.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

ServerManager::ServerManager(char *config_path="default config_path")
{
    if (!(this->setConfigs(config_path)))
        throw exception;
}

ServerManager::ServerManager(const ServerManager& object)
{
}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

ServerManager::~ServerManager()
{
}

/*============================================================================*/
/*******************************  Overload  ***********************************/
/*============================================================================*/

ServerManager& ServerManager::operator=(const ServerManager& object)
{
    return (*this);
}
/*============================================================================*/
/********************************  Getter  ************************************/
/*============================================================================*/

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/
