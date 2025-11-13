#include "../incl/Response.hpp"
#include "../incl/Server.hpp"
#include <sys/wait.h>

void Response::cgiExecuter(std::string path, const std::string &query)
{
	int inPipe[2];
	int outPipe[2];

	std::string method = this->_request->getMethod();

	std::ifstream file(path.c_str());
	if (!file)
	{
		std::cerr << "Requested file open error!\n";
		handleERROR(404);
		return;
	}

	if (pipe(inPipe) == -1 || pipe(outPipe) == -1)
	{
		std::cerr << "ERROR: Pipe creation failed\n";
		handleERROR(500);
		return;
	}

	pid_t pid = fork();
	if (pid < 0)
	{
		std::cerr << "ERROR: Fork failed\n";
		handleERROR(500);
		return;
	}

	if (pid == 0)	//child
	{

		std::string upl = _server->getUploadDir()["root"] + _server->getUploadDir()["location"];
		char absolute_upl[PATH_MAX];
		if (realpath(upl.c_str(), absolute_upl) == NULL)
		{
			std::cerr << "ERROR: upload realpath failed!\n";
			exit(EXIT_FAILURE);
		}
		std::string root = this->_server->getRoot();
		char absolute_root[PATH_MAX];
		if (realpath(root.c_str(), absolute_root) == NULL)
		{
			std::cerr << "ERROR: root realpath failed!\n";
			exit(EXIT_FAILURE);
		}

		if (chdir((_server->getCGIDir()["root"] + _server->getCGIDir()["location"]).c_str()) == -1)
		{
    		std::cerr << "ERROR: chdir failed: " << strerror(errno) << std::endl;
    		exit(EXIT_FAILURE);
		}

		size_t pos = path.find_last_of('/');
		std::string tmp = path.substr(pos + 1);

		std::string methodSTR		= "REQUEST_METHOD=" + method;
		std::string querySTR		= "QUERY_STRING=" + query;
		std::string contentTypeSTR	= "CONTENT_TYPE=" + _request->getHeader("Content-Type");
		std::string contentLenSTR	= "CONTENT_LENGTH=" + _request->getHeader("Content-Length");
		std::string absoluteUploadSTR	= std::string("ABSOLUTE_UPLOAD=") + absolute_upl;
		std::string redirectStatus	= "REDIRECT_STATUS=200";
		std::string gatewayInterface	= "GATEWAY_INTERFACE=CGI/1.1";
		std::string serverProtocol	= "SERVER_PROTOCOL=HTTP/1.1";
		std::string scriptFilename	= "SCRIPT_FILENAME=" + tmp;
		std::string bodySTR			= "BODY_STRING=" + this->_request->getBody();
		std::string rootPath		= std::string("ROOT_PATH=") + absolute_root;

		char *env[] = {
			const_cast<char *>(methodSTR.c_str()),
			const_cast<char *>(querySTR.c_str()),
			const_cast<char *>(contentTypeSTR.c_str()),
			const_cast<char *>(contentLenSTR.c_str()),
			const_cast<char *>(absoluteUploadSTR.c_str()),
			const_cast<char *>(redirectStatus.c_str()),
			const_cast<char *>(gatewayInterface.c_str()),
			const_cast<char *>(serverProtocol.c_str()),
			const_cast<char *>(scriptFilename.c_str()),
			const_cast<char *>(bodySTR.c_str()),
			const_cast<char *>(rootPath.c_str()),
			NULL
		};

		dup2(inPipe[0], STDIN_FILENO);
		dup2(outPipe[1], STDOUT_FILENO);

		close(inPipe[0]);
		close(inPipe[1]);
		close(outPipe[0]);
		close(outPipe[1]);

		char resolved_path[PATH_MAX];
		if (realpath(tmp.c_str(), resolved_path) == NULL)
		{
			std::cerr << "ERROR: realpath failed!\n";
			exit(EXIT_FAILURE);
		}


		char *argv[] = {resolved_path, NULL};

		execve(resolved_path, argv, env);

		// execve failed
		std::cerr << "ERROR: execve failed!\n";
		//std::cerr << "Errno: " << errno << std::endl;
		exit(EXIT_FAILURE);
	}

	// parent
	close(inPipe[0]);   // We only write to inPipe
	close(outPipe[1]);  // We only read from outPipe

	// Send POST data to CGI script (if needed)
	if (method == "POST")
	{
		std::string body = this->_request->getBody();
		if (!body.empty())
		{
			ssize_t written = write(inPipe[1], body.c_str(), body.size());
			std::cerr << "DEBUG: Written to CGI stdin: " << written << " bytes\n";
		}
	}
	close(inPipe[1]); // EOF to child stdin

	// wait for child with timeout
	time_t start = time(NULL);
	int status = 0;

	while (true)
	{
		int ret = waitpid(pid, &status, WNOHANG);

		if (ret == 0)
		{
			if (time(NULL) - start > TIMEOUT_SEC)	// defined in Server.hpp
			{
				std::cerr << "CGI Timeout. Killing child...\n";
				kill(pid, SIGKILL);
				waitpid(pid, &status, 0); // reap
				handleERROR(504);
				close(outPipe[0]);
				return;
			}
			// time_t currentTime = time(NULL);
			// std::cerr << "Time: " << currentTime << '\n';
			usleep(1000); // 1ms
			continue;
		}
		else if (ret == -1)
		{
			std::cerr << "waitpid() failed\n";
			handleERROR(500);
			close(outPipe[0]);
			return;
		}
		else	// Child exited
			break;
	}

	ssize_t n;
	char buffer[1024];
	std::string output;

	while ((n = read(outPipe[0], buffer, sizeof(buffer))) > 0)
		output.append(buffer, n);
	close(outPipe[0]);

	// Handle child exit status
	if (WIFEXITED(status))
	{
		int exitStatus = WEXITSTATUS(status);
		if (exitStatus == 0)
		{
			parseCGIOutput(output);
			setCode(200);
		}
		else
		{
			std::cerr << "CGI script exited with status: " << exitStatus << "\n";
			if (exitStatus == 2)
				handleERROR(400);
			else if (exitStatus == 5)
				redirect(this->_server->getIndex());
			else if (exitStatus == 7)
			{
				std::istringstream lstream(output);
				std::string username;
				std::string status;
				lstream >> username >> status;
				std::string sess_id = _request->getSessionID();
				//std::cout << "CGI body: " << output << std::endl;
				if (status == "login=true")
					_request->setCookie(sess_id, true, username);
				else
					_request->setCookie(sess_id, false, username);
				redirect(this->_server->getIndex());
			}
			else
				handleERROR(500);
		}
	}
	else
	{
		std::cerr << "CGI script did not exit cleanly\n";
		handleERROR(500);
	}

}

void Response::parseCGIOutput(const std::string &output)
{
	std::istringstream stream(output);
	std::string line;

	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r') 		// CGI often uses \r\n so remove \r if present
			line.erase(line.size() - 1);
		if (line.empty())
			break;

		size_t sep = line.find(": ");
		if (sep != std::string::npos)
		{
			std::string key = line.substr(0, sep);
			std::string value = line.substr(sep + 2);
			_headers[key] = value;
		}
	}
	std::string body((std::istreambuf_iterator<char>(stream)),
						std::istreambuf_iterator<char>());  //using fancy iterator to read the rest of the body
	//std::cout << "CGI body: " << body << std::endl;
	_body = body;
	int bodyLen = _body.size();
	this->_headers["Content-Length"] = intToString(bodyLen);
}
