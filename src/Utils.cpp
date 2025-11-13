
#include "../incl/Utils.hpp"
#include "../incl/Libraries.hpp"

std::string trim(const std::string &str)
{
	size_t start = str.find_first_not_of(" \t\r\n");
	size_t end = str.find_last_not_of(" \t\r\n");
	if (start == std::string::npos || end == std::string::npos)
		return "";
	return str.substr(start, end - start + 1);
}

const std::string intToString(int num)
{
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

void replaceAll(std::string &str, const std::string &placeholder, const std::string &goal)
{
	size_t start = 0;
	while((start = str.find(placeholder, start)) != std::string::npos)
	{
		str.replace(start, placeholder.length(), goal);
		start += goal.length();
	}
}

std::string urlDecode(const std::string &str)
{
	std::string result;
	size_t i = 0;
	while (i < str.length())
	{
		if (str[i] == '%')
		{
			if (i + 2 < str.length())
			{
				std::string hex = str.substr(i + 1, 2);
				int val;
				if (isValidHex(hex, val))
				{
					result += static_cast<char>(val);
					i += 3;
				}
			}
			else
			{
				// malformed %
 				result += str[i++];
				return "";
			}
		}
		else if (str[i] == '+') // '+' in URL encoding means space, so convert it
		{
			result += ' ';
			++i;
		}
		else
			result += str[i++]; // Normal character, just append
	}
	return result;
}

bool isValidHex(const std::string& str, int& value)
{
	std::istringstream iss(str);
	iss >> std::hex >> value;

	return !iss.fail() && iss.eof();
}

bool isValidIP(const std::string &host)
{
	std::istringstream iss(host);
	std::string token;
	std::vector<int> parts;

	while (std::getline(iss, token, '.'))
	{
		int num;
		if (!safeAtoi(token, num) || (num < 0 || num > 255))
		{
			
			return false;
		}
		parts.push_back(num);
	}
	if (parts.size() != 4)
		return false;
	//10.0.0.0 /8 and 172.0.0.0 /8 and 192.168.0.0 /16
	if (parts[0] == 10 || parts[0] == 127 || (parts[0] == 192 && parts[1] == 168) || host == "0.0.0.0")
		return true;
	return false;
}

bool safeAtoi(const std::string& str, int& result)
{
	errno = 0;
	char* end;
	long val = std::strtol(str.c_str(), &end, 10);

	if (errno == ERANGE || val > INT_MAX || val < INT_MIN || *end != '\0')
		return false;

	result = static_cast<int>(val);
	return true;
}

std::vector<std::string> parseMultipartBody(std::string& body, const std::string& boundary)
{
	std::vector<std::string> parts;
	std::string delimiter = "--" + boundary;
	size_t start = 0;
	while (true)
	{
		size_t pos = body.find(delimiter, start);
		if (pos == std::string::npos)
			break;
		pos += delimiter.size();
		// Skip trailing CRLF after boundary
		if (body.compare(pos, 2, "\r\n") == 0)
			pos += 2;

		// Find next boundary
		size_t nextPos = body.find(delimiter, pos);
		if (nextPos == std::string::npos)
			break;

		std::string part = body.substr(pos, nextPos - pos);
		parts.push_back(part);
		start = nextPos;
	}
	return parts;
}

// Extract filename from Content-Disposition header of a part
std::string getFilename(std::string& part, std::string& upload_dir)
{
	std::istringstream stream(part);
	std::string line;
	while (std::getline(stream, line) && line != "\r")
	{
		// Normalize line endings
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		std::string headerName = "Content-Disposition:";
		if (line.compare(0, headerName.size(), headerName) == 0)
		{
			// look for filename="..."
			size_t filenamePos = line.find("filename=\"");
			if (filenamePos != std::string::npos)
			{
				filenamePos += 10; // length of 'filename="'
				size_t endPos = line.find("\"", filenamePos);
				if (endPos != std::string::npos)
				{
					std::string filename = line.substr(filenamePos, endPos - filenamePos);
					return getUniqueFilename(upload_dir, filename);
				}
			}
		}
	}
	return "";
}

// Extract file content from part (after headers and blank line)
std::string getFileContent(std::string& part)
{
	size_t headerEnd = part.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return "";
	return part.substr(headerEnd + 4); // +4 to skip \r\n\r\n
}

std::string	checkCwd(std::string &serverRoot, bool serverConf)
{
	std::string cwd;
	std::string path;

	char* rawCwd = getcwd(NULL, 0);
	if (!rawCwd)
		return ("");

	cwd = rawCwd;
	free(rawCwd);

	if (serverConf == false)
	{
		if (cwd.find("/src") != std::string::npos)
			path = "../" + serverRoot;
		else
			path = serverRoot;
	}
	else
	{
		if (cwd.find("/src") != std::string::npos)
			path = "../www/config";
		else
			path = "www/config";
	}

	return (path);
}

bool	isScript(const std::string &path)
{
	std::string ext = findExt(path);
	
	if (ext == ".cgi" || ext == ".pl" || ext == ".py" || ext == ".php" || ext == ".rb" || ext == ".sh" || ext == ".js")
		return true;
	return false;
}

std::string	findExt(const std::string &path)
{
	size_t 		dotPos = path.find_last_of(".");

	if (dotPos != std::string::npos && !path.empty())
		return path.substr(dotPos);
	return "";
}


bool fileExists(const std::string& path)
{
	return (access(path.c_str(), F_OK) == 0);
}

// Split filename into base and extension manually
void splitFilename(const std::string& filename, std::string& base, std::string& ext)
{
	size_t pos = filename.rfind('.');
	if (pos == std::string::npos || pos == 0)
	{
		base = filename;
		ext = "";
	}
	else
	{
		base = filename.substr(0, pos);
		ext = filename.substr(pos);
	}
}

std::string getUniqueFilename(const std::string& directory, const std::string& filename)
{
	std::string base, ext;
	splitFilename(filename, base, ext);
	if (ext.empty())
		return "";

	std::string candidate = filename;
	int counter = 1;

	while (fileExists(directory + "/" + candidate))
	{
		std::ostringstream oss;
		oss << base << "_" << counter << ext;
		candidate = oss.str();
		counter++;
	}
	return candidate;
}
