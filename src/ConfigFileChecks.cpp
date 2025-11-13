#include	"../incl/Server.hpp"
#include <dirent.h>

void Server::allowedMethods(std::string &trimmed)
{
	size_t pos = trimmed.find_last_of(" ");
	if (pos != std::string::npos)
	{
		std::stringstream ss(trimmed.substr(pos + 1));
		std::string item;
		while (std::getline(ss, item, ','))
		{
			if (item != "POST" && item != "GET" && item != "DELETE")
				throw ConfigException("Only allowed methods are GET, POST, DELETE");
		}
	}
}

void Server::assignUploadDir()
{
	for (std::map<std::string, std::map<std::string, std::string> >::iterator it = _locationBlocks.begin(); it != _locationBlocks.end(); ++it)
	{

		std::map<std::string, std::string>::iterator it_dir = it->second.find("upload_dir");
		if (it_dir != it->second.end() && it_dir->second == "yes")
		{
			if (_uploadDir.empty() )
			{
				if (!checkPOST(it->second))
					throw ConfigException("Upload directory must allow POST method");
				_uploadDir ["root"] = findRoot(it->second);
				_uploadDir["location"] = it->first;
				std::cout << "Assigned upload dir location: " << _uploadDir["location"] << std::endl;
			}
			else
				throw ConfigException("Can only assign one upload dir");

		}
	}
	if (_uploadDir.empty())
		throw ConfigException("Must assign one upload directory. Set in location block 'upload_dir = yes'");
}

void Server::assignCGIDir()
{
	for (std::map<std::string, std::map<std::string, std::string> >::iterator it = _locationBlocks.begin(); it != _locationBlocks.end(); ++it)
	{

		std::map<std::string, std::string>::iterator it_dir = it->second.find("allowed_scripts");
		if (it_dir != it->second.end())
		{
			if (_cgiDir.empty() )
			{
				std::stringstream ss(it_dir->second);
				std::string buffer;
				while (std::getline(ss, buffer, ','))
				{
					_allowedScripts.push_back(buffer);
					//std::cout << "BUFFER: " << buffer << std::endl;
				}
				_cgiDir ["root"] = findRoot(it->second);
				_cgiDir["location"] = it->first;
				std::cout << "Assigned cgi dir location: " << _cgiDir["location"] << std::endl;
			}
			else
				throw ConfigException("Can only assign one cgi dir");

		}
	}
	if (_cgiDir.empty())
		throw ConfigException("Must assign one cgi directory. Set in location block 'allowed_scripts = [allowed script types]'");
}

bool Server::checkPOST(std::map<std::string, std::string> configblock)
{
	std::map<std::string, std::string>::iterator it = configblock.find("methods");
	if (it != configblock.end() && it->second.find("POST") != std::string::npos)
		return true;
	return false;
}

std::string Server::findRoot(std::map<std::string, std::string> configblock)
{
	std::map<std::string, std::string>::iterator it = configblock.find("root");
	if (it != configblock.end())
		return it->second;
	return "";
}

void	Server::checkScriptsExecutable()
{
	std::string dir = _cgiDir["root"] + _cgiDir["location"] ;

	DIR* directory = opendir(dir.c_str());
	if (!directory)
		throw ServerException("Can not open directory " + dir);
	struct dirent* entry;
	/*
			struct dirent {
		ino_t          d_ino;       // inode number (file ID)
		off_t          d_off;       // offset to next dirent
		unsigned short d_reclen;    // length of this record
		unsigned char  d_type;      // type of file (not always available)
		char           d_name[256]; // filename (null-terminated string)
		};
	*/
	while ((entry = readdir(directory)) != NULL)
	{
		std::string filename = entry->d_name;
		if (filename == "." || filename == "..") //skip current and parent directory
			continue;
		std::string ext = findExt(filename);
		if (ext == ".py" || ext == ".js" || ext == ".php" || ext == ".cgi")
		{
			std::string fullPath = dir + "/" + filename;
			if (access(fullPath.c_str(), X_OK) != 0)
			{
				closedir(directory);
				throw ServerException("Script " + filename + " inside of " + dir + " has to be executable");
			}
		}
	}
	closedir(directory);
}
