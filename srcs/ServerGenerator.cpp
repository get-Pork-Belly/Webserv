#include "ServerGenerator.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

ServerGenerator::ServerGenerator(const ServerManager& server_manager)
: _server_manager(server_manager) {}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

ServerGenerator::~ServerGenerator()
{
}

/*============================================================================*/
/*******************************  Overload  ***********************************/
/*============================================================================*/

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

void
ServerGenerator::convertFileToStringVector(const char *config_file_path)
{
	int							fd;
	int							readed;
	char						buf[BUF_SIZE];
	std::string 				readed_string;
	std::vector<std::string>	lines;

	fd = open(config_file_path, O_RDONLY, 0644);
	if (fd < 0)
		throw (strerror(errno));
	memset(reinterpret_cast<void *>(buf), 0, BUF_SIZE);

	while ((readed = read(fd, reinterpret_cast<void *>(buf), BUF_SIZE)))
	{
		if (readed < 0)
			throw(strerror(errno));
		readed_string += std::string(buf);
	}
	lines = ft::split(readed_string, "\n");

	for (std::string line : lines)
	{
		//TODO trimmed 데이터가 유지될까? 포인터로 만들어야하지 않을까?
		//TODO push_back의 인자로 std::string(trimmed) 처럼 새롭게 만들어서 넣을까?
		//TODO: test
		std::string trimmed = ft::ltrim(ft::rtrim(line));
		if (trimmed.size() > 0)
			_str_vector_configfile.push_back(std::string(trimmed));
	}
}

// TODO setGlobalConfig가 진짜로 필요한가? 만약 불필요하다면 이 함수를 parseHttpBlock로 바꿔보자
void
ServerGenerator::setGlobalConfig()
{
    _global_config._http_version = std::string("HTTP/1.1");
    _global_config._os = std::string("MAC OS");
}

// NOTE 함수 내부에서 parse 함수들을 호출한다.
void
ServerGenerator::generateServers(std::vector<Server *>& servers)
{
	std::vector<std::string>	directives;
	// TODO server_config를 map으로 할지 struct로 할지 확실히 정하기.
	// struct ServerConfig			_server_config;
	
	// TODO
	// std::map<std::string, std::string> _http_config;

	std::vector<std::string>::iterator it = this->_str_vector_configfile.begin();
	std::vector<std::string>::iterator ite = this->_str_vector_configfile.end();
	//NOTE 범위 기반 반복문  C++11문법
	//NOTE map 하면 생기는 이점. _server_config 먼저 server블록 밖의 정보들을 세팅 한 다음에
	// 그 다음에 server 블록을 돌면 자연스럽게 구체화가 된다.
	/*
	 * http{
	 *   root = /user/;
	 *
	 *   server {
	 *		root = /user/sanam/;
	 *   } -> server_confing -> servers.push_back(new Server(server_config))
	 *
	 *   server {
	 *   } -> server_confing -> servers.push_back(new Server(server_config))
	 * }
	 *
	 * _server_config[root] = /user;
	 * _server_config[root] = /user/sanam/으로 갱신 - 구체화된다.
	 *
	 */
	//TODO http_config 를 먼저 작성하자.
	// _http_config = parseHttpBlock();
	// {
	//    http version, os
	// }

	while (it++ != ite)
	{
		if ( *it == "server {")
		{
			// NOTE _server_config 는 반복이 될 때 마다 갱신된다.
			// NOTE http 블록 설정 
			std::map<std::string, std::string> _server_config(_http_config);
			//NOTE Server블록에 대한 설정을 읽고 _server_config 맵 자료구조에 키 밸류 값으로 넣어준다.
			//NOTE ++it를 넣어준 이유는 "server {" 이 녀석 자체는 필요 없기 때문이다.
			parseServerBlock(++it, _server_config);
			//NOTE Server를 생성한 다음에 _servers에 push_back 한다.
			//NOTE 여기서 _server_config은 구조체가 아니라 map 자료구조임을 명심하자.
			servers.push_back(new Server(_server_config));
			_server_config.clear();
		}
	}
}


//NOTE it를 넣어준 이유는 parseServerBlock이 끝낸 후에 it가 serverBlock 이후를 볼 수 있게 만들기 위함.
void
ServerGenerator::parseServerBlock(std::vector<std::string>::iterator& it, std::map<std::string, std::string>& server_config)
{
	std::vector<std::string>	directives;

	while (true)
	{
		// directives[0] = root, directives[1] = /user
		directives = ft::split(*it, " "); //NOTE directives[0]: key, [1]: value
		if (directives[0] == "location")
			parseLocationBlock(it, server_config); //NOTE server_config에서 location 설정 처리도 같이하는건 어떨까?
		if (directives[0] == "}")
			return ;

		//NOTE ServerConfig를 map으로 하면 편한(?) 이유.
		// 한줄 씩 파싱하면서 나온 값을 키 값을 기준으로 넣어주면 편한데
		// 구조체를 사용해서 값을 할당하면 찾을 방법이 없다.
		// NOTE simple지시어는 자신의 끝을 알려주기 위해 ';'를 사용한다. 쓸모 없으므로 지워주자.
		std::string temp = ft::rtrim(directives[directives의 마지막 인덱스], ";");
		directives[1] = temp;
		server_config[directives[0]] = directives[1]; //NOTE key = value
		// NOTE 지시어를 초기화 해준다.
		directives.clear();
		it++;
	}
	
}

//NOTE it를 넣어준 이유는 parseServerBlock이 끝낸 후에 it가 serverBlock 이후를 볼 수 있게 만들기 위함.
void
ServerGenerator::parseLocationBlock(std::vector<std::string>::iterator& it, std::map<std::string, std::string>& server_config)
{
	std::vector<std::string> directives;

	while(true)
	{
		directives = ft::split(it, " "); //NOTE directives[0]: key, [1]: value
		if (directives[0] == "}")
			return ;
		std::string temp = ft::rtrim(directives[1], ";");
		directives[1] = temp;
		server_config[directives[0]] = directives[1]; //NOTE key = value
		// NOTE 지시어를 초기화 해준다.
		directives.clear();
		it++;
	}
}
