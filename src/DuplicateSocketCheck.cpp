#include "../incl/Server.hpp"
#include "../incl/Libraries.hpp"

int	checkForPortDuplicates(std::vector<std::string> *ports, std::string &port)
{
	for (size_t i = 0; i < ports->size(); ++i)
	{
		if (ports->at(i) == port)
			return (1);
	}
	return (0);
}

int	extractPorts(std::vector<std::string> *ports, std::string &line)
{
	size_t						equalPos = line.find_first_of("=");
	size_t						commaPos;
	std::string					tmpLine;
	std::string					port;

	if (equalPos == std::string::npos)
		return (1);

	tmpLine = line.substr(equalPos + 1);
	tmpLine = trim(tmpLine);

	while (1)
	{
		commaPos = tmpLine.find_first_of(",");
		if (commaPos != std::string::npos)
		{
			port = tmpLine.substr(0, commaPos);
			port = trim(port);
			tmpLine = tmpLine.substr(commaPos + 1);			
		}
		else
			port = trim(tmpLine);
		
		if (checkForPortDuplicates(ports, port) == 1)
			return 1;
		
		ports->push_back(port);
		
		if (commaPos == std::string::npos)
			break;
	}

	return (0);
}

int	checkforSocketDuplicates(std::vector<std::string> &configList)
{
	std::vector<std::string>	ports;
	std::string					serverConfigs;
	std::string 				line;
	
	std::vector<std::string>::iterator it = configList.begin();
	
	while (it != configList.end())
	{
		serverConfigs = *it;
		std::istringstream			iss(serverConfigs);

		while (getline(iss, line))
		{
			line = trim(line);
			if (line.find("listen") != std::string::npos)
			{
				if (extractPorts(&ports, line) == 1)
					return (1);
			}
		}
		++it;
	}
	return (0);
}
