# Web server from scratch, a project at 42 school
Contributors: Stephanie (@StephNova), Chris (@Spanxx) and Henriette (@Henrizz)
<br>
<br>

## Context
It's true that nowdays it's quite easy and fast to set up a web server using existing tools. However, the goal of this project was to build a fully functional HTTP/1.1 web server from scratch! no external frameworks, no Boost, just the standard C++98 library and UNIX system calls.

It was a great way to understand how the web actually works at a low level, from handling sockets and parsing requests to generating responses. It was also my first object-oriented programming project in C++, where I learned to design modular, maintainable code using classes, exceptions, and encapsulation.

Additionally, this was a team project with three members working mostly remotely. We collaborated through Git branches and pull requests, learning to manage version control effectively, divide responsibilities by functionality, and handle errors gracefully across the system.

Although we didn’t implement explicit design patterns in this project, it helped us understand the situations where they could be valuable. For instance, a Builder or Factory pattern would have improved the structure of our server and response creation logic. We also realized that our event-driven loop could be described using the Reactor pattern.

## Project description
The objective of this project is to learn the rules and basics of HTTP and client-server communication. We are asked to build a server that can host various websites with individual databases and specifications determined in a configuration file (similar to NGINX). A special focus is on keeping the server non-blocking at all times, which we tested with siege, to ensure 100% availability even with many simultaneous requests.
<br>
We handle the following aspects:
- regular and chunked requests
- GET, POST and DELETE methods
- login feature with user and cookie management
- CGI scripts (using python and php)
- upload and deletion of files
- html and css
- automatically updated image gallery with javascript
- custom error pages with the adequate error codes
- routing per specificied location blocks
- directory listing for directory listing (when no index page is specified in config file and autoindex is on)

## Prerequisites
- Docker (for the Docker instructions)
- For native builds: a C++ toolchain that supports C++98, GNU make (on Linux/macOS: e.g. build-essential on Ubuntu, Xcode command-line tools on macOS)
- If you want to run the PHP CGI examples on the host: php-cgi (location differs per OS)

---

## Quick Start — Docker (recommended)
These steps work on macOS, Linux, and Windows (with Docker Desktop).

1) Build and run with docker-compose:

```bash
docker compose up --build
```

2) Open the websites in your browser:
- http://localhost:3050 — webserv42
- http://localhost:9090 — second server

3) Stop the servers:

```bash
docker compose down
```

Notes:
- Docker gives a reproducible Linux environment; it is the simplest way to run the project on Windows without porting code.

---

## Native build & run (macOS / Linux / WSL)
Use this if you want to compile and run the server directly on a Unix-like environment.

1) From the project root, build with make and run :

```bash
make && ./webserv server.conf
```

Notes:
- By default the example config uses ports like 3050/9090. If you bind to ports <1024 you will need elevated privileges (sudo).
- If you get bind errors, ensure the port is free and that no firewall is blocking access.

---

## Windows users
This project is implemented with POSIX APIs and is not natively compatible with MSVC/WinSock without porting.

Recommended run with Docker Desktop (see Docker section) — easiest and fully supported.

## PHP CGI script note ("SAY HI" button)
The small PHP CGI used in `www/.../cgi-bin/hello.php` is configured for a Linux php-cgi shebang. On macOS you may need to update the shebang line to the location of your `php-cgi` binary.

To find php-cgi on your machine:

```bash
which php-cgi
```

Then open the `hello.php` script and update the first line (shebang) to the returned path, or use a portable shebang like:

```text
#!/usr/bin/env php-cgi
```

---

## Usage summary
- Docker: `docker compose up --build` and visit localhost:3050 / 9090
- Native: `make` then `./webserv config/server.conf` (or pass another config file)

## Troubleshooting
- "Bind failed" or "Address already in use": another process is using the port. Use `lsof -i :PORT` (macOS/Linux) to find and stop it.
- Missing compiler or headers: ensure your toolchain is installed (build-essential / Xcode CLI).
- Windows users: prefer Docker/WSL — native Windows builds are not supported without a port.

---




