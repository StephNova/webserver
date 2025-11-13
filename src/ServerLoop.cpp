#include	"../incl/Server.hpp"
#include	"../incl/Request.hpp"
#include	"../incl/Response.hpp"
#include	"../incl/Libraries.hpp"

bool Server::isServerSocket(int fd)
{
	for (size_t i = 0; i < this->_serverSockets.size(); ++i)
	{
		if (this->_serverSockets[i] == fd)
			return true;
	}
	return false;
}

std::vector<int>	Server::makeNewConnections(int server_fd)
{
	std::vector<int> newClients;

	while (true)
	{
		struct sockaddr_in clientAddr;
		socklen_t clientLen = sizeof(clientAddr);
		int clientSocket = accept(server_fd, (struct sockaddr *)&clientAddr, &clientLen);

		if (clientSocket < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			else
			{
				std::cerr << "Error accepting client connection\n";
				break;
			}
		}
		fcntl(clientSocket, F_SETFL, O_NONBLOCK);
		newClients.push_back(clientSocket);
		std::cout << "New connection accepted: fd = " << clientSocket << std::endl;
	}
	return newClients;
}

bool Server::readFromConnection(std::map<int, std::string> &response_collector, int fd, std::map<int, bool> &keepAlive, std::vector<struct pollfd> &globalPollFds)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read = recv(fd, buffer, sizeof(buffer), 0); // Check why 0

	// --- Connection closed or error ---
	if (bytes_read <= 0) {
		if (bytes_read < 0)
			std::cerr << "[recv error] fd " << fd << "\n";
		else
			std::cout << "[client closed] fd " << fd << "\n";
		return false;
	}

	//std::cout << "Read " << bytes_read << " bytes from fd " << fd << "\n";
	_socketBuffers[fd].append(buffer, bytes_read);

	std::string &data = _socketBuffers[fd];

	size_t header_end = data.find("\r\n\r\n");

	if (header_end == std::string::npos)
	{
		//std::cout << "[Incomplete headers] Still waiting for \\r\\n\\r\\n\n";
		return true; // Wait for more header data
	}

	// --- Initialize Request object if it's the first time ---
	if (_requestCollector.find(fd) == _requestCollector.end())
		initialize_request(fd, data, header_end);

	Request *request = _requestCollector[fd];
	keepAlive[fd] = request->getConnection();

	if (request->getCode() == 200)
	{
		RequestState state = handleRequest(data, header_end, keepAlive, fd);

		if (state == REQUEST_INCOMPLETE)
			return true; // Still waiting for body/chunks

		// --- Clean body from socket buffer ---
		size_t consumed = header_end + 4; // header + \r\n\r\n
		if (request->isChunked()) {
			consumed = request->getParsePos(); // parse_chunks sets it
		} else {
			consumed += request->getContentLength();
		}
		_socketBuffers[fd] = data.substr(consumed); // remove consumed part
	}
	// --- Prepare response ---
	prepare_response(fd, response_collector);

	// --- Switch to POLLOUT mode ---
	for (size_t i = 0; i < globalPollFds.size(); ++i) {
		if (globalPollFds[i].fd == fd)
		{
			globalPollFds[i].events = POLLOUT;
			//std::cout << "Switched fd " << fd << " to POLLOUT\n";
			break;
		}
	}
	return true;
}

int	Server::write_to_connection(std::map<int, std::string> &response_collector, int fd, std::vector<struct pollfd> &globalPollFds)
{
	std::string &resp = response_collector[fd];

	//std::cout << "Writing response to fd " << fd << ": " << resp.size() << " bytes\n";
	ssize_t sent = send(fd, resp.c_str(), resp.size(), 0);
	if (sent <= 0)
	{
		if (sent < 0)
			std::cerr << "Send error on fd " << fd << std::endl;
		if (sent == 0)
			std::cerr << "Nothing was sent on fd " << fd << " - connection might be closed\n";
		return (SEND_ERROR);
	}

	if ((size_t)sent < resp.size())
		resp = resp.substr(sent); // not all sent; store remainder
	else
	{
		response_collector.erase(fd);
		_socketBuffers[fd].clear(); // Optional: clear to prepare for pipelined request
		_requestCollector.erase(fd);
		// Switch to POLLIN

		// Switch fd back to POLLIN to receive more requests
		for (size_t i = 0; i < globalPollFds.size(); ++i)
		{
			if (globalPollFds[i].fd == fd)
			{
				globalPollFds[i].events = POLLIN;
				//std::cout << "Switched fd " << fd << " to POLLIN\n";
				return (SEND_COMPLETE);
			}
		}
	}
	return (SEND_CONTINUE);
}

void	Server::close_erase(int fd)
{
	if (isServerSocket(fd))
		return;

	_socketBuffers.erase(fd);

	std::map<int, Request*>::iterator it = _requestCollector.begin();
	while(it != _requestCollector.end())
	{
		if (it->first == fd)
		{
			delete it->second; // Free Request object
			_requestCollector.erase(it->first); // Remove from collector
			break;
		}
		else
			++it;
	}
}

void Server::initialize_request(int fd, const std::string &data, size_t header_end)
{
	std::string header_part = data.substr(0, header_end + 4);
	//std::cout << "Request from client fd " << fd << std::endl;
	Request *request = new Request(this);
	request->check_headers(header_part); // add check for header size
	_requestCollector[fd] = request;
}

RequestState Server::handleRequest(std::string &data, size_t header_end, std::map<int, bool> &keepAlive, int fd)
{
if (_requestCollector.find(fd) == _requestCollector.end())
{
    std::cerr << "Missing request for fd " << fd << "!\n";
}

	Request *request = _requestCollector[fd];
	keepAlive[fd] = request->getConnection();

	if (!request->isChunked())
	{
		int content_length = request->getContentLength();
		if (content_length < 0)
		{
			//std::cerr << "Missing or invalid Content-Length for fd " << fd << "\n";
			request->setCode(400); // Bad Request
			// close connection!?
			return REQUEST_ERROR_RESPOND; // Return true so prepare_response can set 400 response
		}

		size_t total_required = header_end + 4 + content_length; // +4 for "\r\n\r\n"
		if (data.size() < total_required)
		{
			//std::cout << "Not enough data yet for fd " << fd << ", waiting for more...\n";
			return REQUEST_INCOMPLETE; // Keep connection open, still waiting for data
		}
		std::string body_part;
		if (content_length > 0)
			body_part = data.substr(header_end + 4, content_length);

		request->append_body(body_part);

		if (request->getBodySize() > _maxBodySize)
			request->setCode(413);
		return REQUEST_COMPLETE; // Request complete, response can be prepared (even if it's an error response)
	}
	else // It's a chunked request
	{
		size_t chunk_start = header_end + 4; // Start of chunked body after header
		if (request->parse_chunks(data, chunk_start))
		{
			if (request->getBodySize() > _maxBodySize)
				request->setCode(413); // Payload Too Large, even if chunk parsing was successful
			return REQUEST_COMPLETE; // Request complete, response can be prepared
		}
		else if (request->getCode() != 200) // Error during chunk parsing
			return REQUEST_ERROR_RESPOND; // e.g., 400 Bad Request due to chunking error
		return REQUEST_INCOMPLETE; // Still waiting for more chunks
	}
}

void Server::prepare_response(int fd, std::map<int, std::string> &response_collector)
{
	Request *request = _requestCollector[fd];
	Response *response = new Response(request, this, this->_name);

	response_collector[fd] = response->process_request();

	delete request;
	delete response;

	_requestCollector.erase(fd);
}

bool	Request::parse_chunks(std::string &data, size_t start)
{
	size_t pos;
	if (_parse_pos == 0)
		pos = start;
	else
		pos = _parse_pos;
	while (true)
	{
		size_t end = data.find("\r\n", pos);
		if (end == std::string::npos)
			break;
		int chunk_size = 0;
		if (!isValidHex(data.substr(pos, end - pos), chunk_size))
		{
			std::cerr << "Invalid chunk size\n";
			_code = 400;
			return false;
		}
		size_t chunk_start = end + 2;
		size_t chunk_end = chunk_start + chunk_size;
		if (chunk_size == 0)
		{
			if (data.size() < end + 4 || data.substr(end + 2, 2) != "\r\n")
			{
				std::cerr << "Missing CRLF after final 0 chunk\n";
				_code = 400;
				return false;
			}
			data.erase(0, end + 4);
			_parse_pos = 0;
			return true;

		}
		if (data.find("\r\n", chunk_start) != chunk_end)
		{
			std::cerr << "Chunk data smaller than indicated\n";
			_code = 400;
			return false;
		}
		if (data.size() < chunk_end + 2) // not enough data yet
			break;
		if (data.substr(chunk_end, 2) != "\r\n")
		{
			std::cerr << "Missing CRLF after chunk data\n";
			_code = 400;
			return false;
		}
		_body.append(data.substr(chunk_start, chunk_size));
		if (_body.size() > _server->getMaxBodySize())
		{
			_code = 413;
			return false;
		}

		pos = chunk_end + 2;
	}
	_parse_pos = pos;
	return false;
}
