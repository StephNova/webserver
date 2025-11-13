#include "../incl/Server.hpp"

std::string checkFilePath(char *av)
{
	std::string filePath;
	std::string tryPath;
	std::string workDir;

	if (av != NULL)
		filePath = av;

	std::ifstream f1(filePath.c_str());
	if (f1.good())
		return (filePath);

	char *cwd =	getcwd(NULL, 0);
	if (cwd)
	{
		workDir = cwd;
		free (cwd);
	}

	if (workDir.find("src/") != std::string::npos)
		tryPath = "../";
	
	tryPath += "www/config/" + filePath;
	std::ifstream f2(tryPath.c_str());
	if (f2.good())
		return (tryPath);

	tryPath += "www/" + filePath;
	std::ifstream f3(tryPath.c_str());
	if (f3.good())
		return (tryPath);
	
	return ("");
}