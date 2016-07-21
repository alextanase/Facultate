#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <queue>
#include <fstream>

using namespace std;

#define HTTP_PORT 80
#define BUFLEN 1024
#define HTTP_OK 200

string log_stdout, log_error;
fstream fout, ferr;

char msg[BUFLEN], err[BUFLEN];
char buffer[BUFLEN], sendbuf[BUFLEN], recvbuf[BUFLEN];

/* pentru conectarea catre serverul catre care vom trimite fisierele */
unsigned int sockfd, n, portno;
struct sockaddr_in serv_addr;
struct hostent *server;
string ip_server;

/* conectarea la serverul de http */
unsigned int sockfd_http;
struct sockaddr_in serv_addr_http;
string ip_server_http;

/* numele serverului */
string server_name;
string link_url;

bool _o_flag = false;
bool _a_flag = false;
bool _p_flag = false;
bool _print_enable = false;
bool _e_flag = false;
bool _r_flag = false;
bool _html_file = false;

/* coada de mesaje */
queue<string> Q;

void error(const char *e) {
	if (_print_enable == true) {
		ferr << e;
	} else {
		cerr << e;
	}
	ferr.flush();
	cerr.flush();
}

void status_messges(const char *e) {
	if (_print_enable == true) {
		fout << e;
	} else {
		cout << e;
	}
	fout.flush();
	cout.flush();
}

int ReadLine(int sockfd, char* buff) {
	/* Citeste o linie de pe socket si intoarce numarul de caractere citite */

	int n;
	int count = 0;
	char c;

	memset(buff, 0, BUFLEN);

	while ((n = read(sockfd_http, &c, 1)) == 1) {
		*buff++ = c;
		count++;

		if (c == '\n')
			break;
	}

	return count;
}

void connect_to_server() {
	try {
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
			throw("Error opening socket");
	} catch (const char* e) {
		error(e);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	inet_aton(ip_server.c_str(), &serv_addr.sin_addr);

	try {
		if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))< 0)
			throw(" Eroare la conectare");
	} catch (const char* e) {
		error(e);
	}
}

void connect_to_HTTP() {
	try {
		sockfd_http = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd_http <= 0) {
			throw("Eroare la  creare socket");
		}
	} catch (const char* e) {
		error(e);
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	memset(&serv_addr_http, 0, sizeof(serv_addr_http));
	serv_addr_http.sin_family = AF_INET;
	serv_addr_http.sin_port = htons(HTTP_PORT);

	struct hostent *host = gethostbyname(server_name.c_str());
	ip_server_http.assign(inet_ntoa((struct in_addr &) *host->h_addr_list[0]));
	try {
		n = inet_aton(ip_server_http.c_str(), &serv_addr_http.sin_addr);
		if (n <= 0) {
			throw("Adresa IP invalida");
		}
	} catch (const char *e) {
		error(e);
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	try {
		if (connect(sockfd_http, (struct sockaddr *) &serv_addr_http,
				sizeof(serv_addr_http)) < 0) {
			throw("Eroare la conectare");
		}
	} catch (const char* e) {
		error(e);
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	memset(msg, '\0', BUFLEN);
	sprintf(msg, "Conectare cu succes la %s\n", server_name.c_str());
	status_messges(msg);
}

int get_status(char* buffer) {
	char tmp[20];
	int status;
	sscanf(buffer, "%s %d", tmp, &status);
	return status;
}

void close_connection() {
	close(sockfd);
	close(sockfd_http);
	exit(EXIT_FAILURE);
}

/* parsarea modului de download si a link-ului */
char *split(char *buff) {
	string c(buff);
	string mode;
	char *page;

	page = (char*) malloc(BUFLEN * sizeof(char));
	size_t find1 = c.find(" ");

	mode.assign(c.substr(0, find1));
	/* activam modurile de descarcare */
	if (mode.find("-r") != string::npos)
		_r_flag = true;
	if (mode.find("-e") != string::npos)
		_e_flag = true;

	link_url.assign(c.substr(find1 + 1));
	/*find server name*/
	size_t find2 = link_url.find("/");
	size_t find3 = link_url.find("/", find2 + 1);
	server_name.assign(link_url.substr(find2 + 1, find3 - find2 - 1));

	sprintf(page, "%s", link_url.substr(find3).c_str());
	/* returnam pagina care trebuie descarcata */
	return page;
}

bool is_HTML_file(char *link)
{
	if (strstr(link, ".html") || strstr(link, ".htm"))
		return true;
	return false;
}

bool valid_HTML_link(char* line)
{
	if (strstr(line, "href=") && strstr(line, ".htm") && !strstr(line, "//")
			&& !strstr(line, "#"))
		if (strstr(line, "<a") || strstr(line, "</a>"))
			return true;

	return false;
}

bool valid_attach_link(char* line)
{
	if (strstr(line, "href=") && !strstr(line, ".htm") && !strstr(line, "//")
			&& !strstr(line, "#"))
		if (strstr(line, "<a") || strstr(line, "</a>"))
			if (!strstr(line, "/\"") && !strstr(line, "/'")
					&& !strstr(line, "mailto:"))
				return true;

	return false;
}

/*descarcarea paginii si trimiterea catre server */
void download_file(char *link_to_send)
{
	if (is_HTML_file(link_to_send) == true)
		_html_file = true;
	else
		_html_file = false;

	memset(sendbuf, 0, BUFLEN);
	sprintf(sendbuf, "GET %s HTTP/1.0\n\n", link_to_send);

	sprintf(msg, "Trimit cererea %s\n", link_to_send);
	status_messges(msg);

	/* trimit comanda GET pentru a cere fiserul */
	write(sockfd_http, sendbuf, strlen(sendbuf));

	n = ReadLine(sockfd_http, recvbuf);
	recvbuf[strlen(recvbuf) - 2] = '\0';

	int sc = get_status(recvbuf);

	/* verificam codul intors de pagina */
	if (sc != HTTP_OK) {
		sprintf(msg, "Eroare. Status code %d (%s) intors la cererea %s\n", sc,
				recvbuf + 13, sendbuf);
		error(msg);
		sprintf(sendbuf, "ERROR");
		sprintf(msg, "ERROR\n");
		status_messges(msg);

		send(sockfd, sendbuf, BUFLEN, 0);
		recv(sockfd, buffer, BUFLEN, 0);
		close(sockfd_http);
		return;
	}

	/* Sar peste restul headerului HTTP din raspunsul serverului */
	do {
		n = ReadLine(sockfd, recvbuf);
	} while (n != 2);

	sprintf(msg, "Downloading... %s\n", link_to_send);
	status_messges(msg);

	if (_html_file == true) {
		/* citire si parsare linkuri fisier html */
		while ((n = ReadLine(sockfd, recvbuf)) != 0) {
			/* trimitem catre server */
			send(sockfd, recvbuf, n, 0);
			/* primim ACK */
			recv(sockfd, buffer, BUFLEN, 0);
			/* extragem linkurile relative */
			if (_html_file == true) {
				if (valid_HTML_link(recvbuf)) {
					/* Extrag pozitia 'href' */
					char* s = strstr(recvbuf, "href=");

					char* path = (char*) calloc(BUFLEN, sizeof(char));

					for (int i = 6; s[i] != '\"' && s[i] != '\''; i++)
						path[i - 6] = s[i];

					Q.push(string(path));
				}
				if (_e_flag == true) {
					if (valid_attach_link(recvbuf)) {
						/* Extrag pozitia 'href' */
						char* s = strstr(recvbuf, "href=");

						char* path = (char*) calloc(BUFLEN, sizeof(char));

						for (int i = 6; s[i] != '\"' && s[i] != '\''; i++)
							path[i - 6] = s[i];

						Q.push(string(path));
					}
				}
			}
		}

		sprintf(sendbuf, "DONE");
		sprintf(msg, "DONE\n\n");
		status_messges(msg);

		/* Trimitem confirmare ca am terminat de trimis fisierul */
		send(sockfd, sendbuf, BUFLEN, 0);
		recv(sockfd, buffer, BUFLEN, 0);
		/*inchidem socketul HTTP*/
		close(sockfd_http);

		/* daca e o pagina HTML trimitem linkurile catre server */
		if (_html_file == true) {

			sprintf(msg, "Trimit link-urile de pe pagina catre server\n");
			status_messges(msg);

			size_t pos = link_url.find_last_of("/");
			string dir(link_url.substr(0, pos + 1));

			while (Q.empty() == false) {
				string rel_path = Q.front();
				Q.pop();
				memset(sendbuf, 0, BUFLEN);
				sprintf(sendbuf, "%s%s", dir.c_str(), rel_path.c_str());

				n = send(sockfd, sendbuf, strlen(sendbuf), 0);
				/* primim confirmarea ACK */
				recv(sockfd, buffer, BUFLEN, 0);

				if (n <= 0)
					error("Connection close");
			}

			/* trimitem confirmare ca am terminat de trimis link-urile */
			memset(buffer, 0, BUFLEN);
			sprintf(buffer, "DONE");
			sprintf(msg, "DONE\n\n");
			status_messges(msg);

			n = send(sockfd, buffer, BUFLEN, 0);
			recv(sockfd, buffer, BUFLEN, 0);

			if (n <= 0)
				error("Connection close");
		}
	} else {
		/*download attachements */
		do {
			memset(recvbuf, 0, BUFLEN);
			n = read(sockfd_http, recvbuf, BUFLEN);
			if (n == 0)
				break;
			send(sockfd, recvbuf, n, 0);
			/* primim ACK */
			recv(sockfd, buffer, BUFLEN, 0);
		} while (n != 0);

		sprintf(sendbuf, "DONE");
		sprintf(msg, "DONE\n");
		status_messges(msg);

		send(sockfd, sendbuf, BUFLEN, 0);
		recv(sockfd, buffer, BUFLEN, 0);

		close(sockfd_http);
	}
}

int main(int argc, char *argv[]) {
	/* verificam argumentele date serverului din cmdl */
	pid_t pid;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-a") == 0) {
			_a_flag = true;
			ip_server.assign(argv[i + 1]);
		}
		if (strcmp(argv[i], "-p") == 0) {
			_p_flag = true;
			portno = atoi(argv[i + 1]);
		}
		if (strcmp(argv[i], "-o") == 0) {
			pid = getpid();
			memset(buffer, 0, BUFLEN);
			sprintf(buffer,"%d",pid);
			_o_flag = true;

			log_stdout.append(argv[i + 1]);
			log_stdout.append("-");
			log_stdout.append(buffer);
			log_stdout.append(".stdout");
			fout.open(log_stdout.c_str(), ios::out);

			log_error.append(argv[i + 1]);
			log_error.append("-");
			log_error.append(buffer);
			log_error.append(".stderr");
			ferr.open(log_error.c_str(), ios::out);
			_print_enable = true;
		}
	}

	try {
		if (_p_flag == false || _a_flag == false)
			throw("Usage: ./client [-o logfile] -a <adresa ip server> -p <port>\n");
	} catch (const char* e) {
		error(e);
		exit(EXIT_FAILURE);
	}
	connect_to_server();
	/* asteptam adresa de la server */
	while (1) {
		/* primim calea care trebuie descarcata */
		try {
			memset(recvbuf, 0, BUFLEN);
			n = recv(sockfd, recvbuf, BUFLEN, 0);
			if (n <= 0)
				throw("Close connection");

			char *link = split(recvbuf);
			connect_to_HTTP();
			download_file(link);

		} catch (const char* e) {
			error(e);
			close(sockfd);
			exit(EXIT_SUCCESS);
		}
	}

	return 0;
}
