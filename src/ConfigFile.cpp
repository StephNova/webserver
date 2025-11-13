#include	"../incl/Server.hpp"
#include <sys/stat.h>
#include <sys/types.h>

void	createConfigList(std::string configPath, std::vector<std::string> &configList)
{
	bool			inServerBlock = false;
	bool			inLocationBlock = false;

	std::string		line;
	std::string		serverConfig;
	std::ifstream	iss(configPath.c_str());
	if (!iss)
	{
		throw std::runtime_error("Creating coinfg list failed!");
		// std::cerr << "Creating config list failed!\n";
		// return;
	}

	while (getline(iss, line))
	{
		if (line.find("#") != std::string::npos || line.empty())
			continue;
		if (!inServerBlock && line.find("server") != std::string::npos)
			inServerBlock = true;
		else if (inServerBlock && line.find("{") != std::string::npos)
			inLocationBlock = true;
		else if (inServerBlock && inLocationBlock && line.find("}") != std::string::npos)
			inLocationBlock = false;
		else if (inServerBlock && !inLocationBlock && line.find("}") != std::string::npos)
		{
			serverConfig.append(line);
			inServerBlock = false;
			configList.push_back(serverConfig);
			serverConfig.clear();
			continue;
		}
		line += '\n';
		serverConfig.append(line);
	}
}

std::map<std::string, std::string>* Server::getConfigMap(const std::string &configName)
{
	if (configName == "serverConfig")
		return(&this->_serverConfig);
	if (configName == "dirConfig")
		return(&this->_dirConfig);
	if (configName == "mimeConfig")
		return(&this->_mimetypeConfig);
	if (configName == "typeDirConfig")
		return(&this->_typeDirConfig);

	return (NULL);
}

void	saveKeyValuePair(std::string &trimmed, std::map<std::string, std::string> &targetMap, std::string *host, std::string *locationPath)
{
	size_t		equalPos = trimmed.find("=");

	if (equalPos != std::string::npos)
	{
		std::string	key = trimmed.substr(0, equalPos - 1);
		std::string value = trimmed.substr(equalPos + 1);
		key = trim(key);
		value = trim(value);

		if (key == "host" && value != "")
			*host = value;
		if (key == "location" && value != "")
			*locationPath = value;

		//check for key duplicates
		if (targetMap.find(key) != targetMap.end())
		{
			// std::cout << "Warning: Duplicate key found " << key << '\n'	// check how nginx handles it
			// 		<< "new Value overwrites existing Value!\n";
			targetMap[key] = value;	//new values overrides existing one (nginx is handling it like this)
		}
		else
			targetMap[key] = value;
	}
}

int	handleLocationBlocks(bool *inBlock, std::string &trimmed)
{
	if (!(*inBlock) && trimmed.find("location") == 0 && trimmed.find("{") != std::string::npos)
	{
		*inBlock = true;
		return (1);
	}
	// Check for end of block
	if (*inBlock && trimmed.find("}") == std::string::npos)
		return (1);

	if (*inBlock && trimmed.find("}") != std::string::npos)
	{
		*inBlock = false;
		return (1);
	}

	return (0);
}

void	Server::extractConfigMap(std::string &configFile, std::map<std::string, std::string> &targetMap, std::string target)
{
	std::string	line;
	std::string trimmed;
	std::string locationPath;
	bool		inBlock = false;

	std::istringstream iss(configFile);
	if (!iss)
		throw ConfigException("Extracting config file failed!\n");

	while (getline(iss, line))
	{
		trimmed = trim(line);
		if (trimmed.find(target) != std::string::npos)
		{
			//save dir before continue in dirblock
			if (target == "location")
			{
				if (trimmed.find_first_of('{') != std::string::npos)
					trimmed.erase(trimmed.find_first_of('{'));	//remove curly bracket at end of location
				saveKeyValuePair(trimmed, targetMap, &this->_IPHost, &locationPath);		//save location
			}

			while (getline(iss, line))
			{
				trimmed = trim(line);

				if (trimmed.empty() || trimmed[0] == '#')
					continue;

				if (trimmed == "}" && !inBlock)
				{
					if (target == "location" && !targetMap.empty())
					{
						this->_locationBlocks[locationPath] = targetMap;
						doesRootExist(targetMap);
						doesMethodsExist(targetMap);
						targetMap.clear();
						break;
					}
				}
				if (trimmed.find("methods") != std::string::npos)
					allowedMethods(trimmed);
				if (handleLocationBlocks(&inBlock, trimmed) == 1)
					continue;

				saveKeyValuePair(trimmed, targetMap, &this->_IPHost, &locationPath);
			}

		}
	}
}

void	Server::doesRootExist(std::map<std::string, std::string> &targetMap)
{
	struct stat st;
	std::string path = findRoot(targetMap);
	//std::cout << "ROOT: " << path << std::endl;
	if (stat(path.c_str(), &st) == 0) // returns 0 if path exists
	{
		if (S_ISDIR(st.st_mode) == 0)
			throw ServerException("Path " + path + " is not a directory");
	}
	else
		throw ServerException("Path " + path + " does not exist");
}


void	Server::doesMethodsExist(std::map<std::string, std::string> &targetMap)
{
	std::map<std::string, std::string>::iterator it = targetMap.find("methods");
	if (it != targetMap.end())
		return;
	throw ConfigException("Methods need to be specified in every location block");
	
}

void	Server::loadTypeFiles(std::string fileName, std::string keyword)
{
	std::string line;
	std::string config;
	std::string fullPath;

	fullPath = checkCwd(this->_serverRoot, true);

	fullPath += "/" + fileName;
	std::ifstream file(fullPath.c_str());
	if (!file)
		throw ServerException("Loading Typefile failed!");

	while(getline(file, line))
	{
		if (line.empty() || line.find('#') != std::string::npos)
			continue;
		line += '\n';
		config.append(line);
	}

	if (fileName == "mime.types")
		this->extractConfigMap(config, _mimetypeConfig, keyword);
	else if (fileName == "typeDir.conf")
		this->extractConfigMap(config, _typeDirConfig, keyword);
}

// int	Server::createConfig(char *av, std::string &serverConfig)
void	Server::createConfig(std::string &serverConfig)
{
	// std::string filePath = checkFilePath(av);

	std::map<std::string, std::string>	_dirConfig;

	this->extractConfigMap(serverConfig, _serverConfig, "server");
	this->extractConfigMap(serverConfig, _dirConfig, "location");
	this->loadTypeFiles("mime.types", "types");
	this->loadTypeFiles("typeDir.conf", "typeDir");
}
