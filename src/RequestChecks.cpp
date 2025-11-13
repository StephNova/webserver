#include "../incl/Request.hpp"

int Request::checkURILength()
{
	size_t maxLen = 0;

	//get server config map
	std::map<std::string, std::string> *config = this->_server->getConfigMap("serverConfig");
	if (!config)
	{
		std::cerr << "Config map 'serverConfig' not found!\n";
		this->_code = 500;
		return (1);
	}

	// without line in config --> Default value
	std::map<std::string, std::string>::iterator it = config->find("limitRequestLine");
	if (it == config->end())
	{
		std::cerr << "LimitRequestLine not found in config map!\n";
		std::cerr << "Default value: '4096' used for check!\n";
		maxLen = 4096;
	}
	else	// with line in config
	{
		std::stringstream ss(it->second);
		ss >> maxLen;

		if (ss.fail())
		{
			std::cerr << "Invalid Number in limitRequestLine!\n";
			this->_code = 500;
			return (1);
		}
	}

	if (this->_path.size() > maxLen)
	{
		std::cerr << "Request length is bigger then 'limitRequestLine' in server config!\n";
		this->_code = 414;	// 431? checks all headers??
		return (1);
	}

	return (0);
}

int	Request::checkPathChars()
{
	const std::string allowedChars = "-_./~ ";	//inclued A-Z a-Z 0-9 (checked with isalnum())
	const std::string reservedChars = "!*'();:@&=+$,/?#%";
	const std::string unsafeChars = "`<>\"{}";

	for (int i = 0; this->_path[i]; ++i)
	{
		char	c = this->_path[i];

		//alphanumeric and allowed chars
		if (std::isalnum(c) || allowedChars.find(c) != std::string::npos)
			continue;

		//Reserved raw chars - not encoded
		if (reservedChars.find(c) != std::string::npos)
			continue;

		if (unsafeChars.find(c))
			this->_code = 400;

		this->_code = 400;
		return (1);
	}
	return (0);
}
