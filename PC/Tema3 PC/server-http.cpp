#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <queue>
#include <time.h>

using namespace std;

#define HTTP_PORT 80
#define BUFLEN 1024
#define LEN 100
#define MAX_CLIENTS 10
#define MIN_CLIENTS 5
#define MAX_DEPTH 5

struct client {
	int socket;
	int port;
	string ip_address;
	string path_file;
	bool _ready;
};

bool _r_flag = false;
bool _e_flag = false;
bool _o_flag = false;
bool _p_flag = false;
bool _print_enable = false;
bool _download_nivel = false;
bool _finish_download = true;

/* fisierele de stdout si stderr */
string log_stdout, log_error;
fstream fout, ferr;

/* multimea de filedescriptori pentru clientii care se vor conecta */
fd_set read_fds;	//multimea de citire folosita in select()
fd_set tmp_fds;	//multime folosita temporar
int fdmax;	//valoare maxima file descriptor din multimea read_fds
struct sockaddr_in serv_addr, cli_addr;
unsigned int sockfd, newsockfd, portno, clilen;
char buffer[BUFLEN], sendbuf[BUFLEN], recvbuf[BUFLEN];
char err[BUFLEN];
int n;
char msg[BUFLEN], cmd[BUFLEN];
int DEPTH = -1;

/* adresa introdusa de la tastatura */
string url;
string command;
/* vectorul de clienti conectati */
vector<client> clients;
/* coada de linkuri */
queue<string> Qsend, Qrecv;

void error(const char *e)
{
	if (_print_enable == true) {
		ferr << e;
	} else {
		cerr << e;
	}

	ferr.flush();
	cerr.flush();
}

void status_messges(const char *e)
{
	if (_print_enable == true) {
		fout << e;
	} else {
		cout << e;
	}

	fout.flush();
	cout.flush();
}

void print_clients()
{
	if (_print_enable == true) {
		fout << "Clientii conectati" << "\n";
		for (int i = 0; i < clients.size(); i++) {
			fout <<clients[i].port << " "
					<< clients[i].ip_address << "\n";
		}
	} else {
		cout << "Clientii conectati" << "\n";
		for (int i = 0; i < clients.size(); i++) {
			cout << clients[i].port << " "
					<< clients[i].ip_address << "\n";
		}
	}

	fout.flush();
	cout.flush();
}

void server_listening()
{
	/* golim multimea de filedes */
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	try {
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
			throw("Error opening socket");
	} catch (const char* e) {
		error(e);
	}

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
	serv_addr.sin_port = htons(portno);

	try {
		if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(struct sockaddr)) < 0)
			throw("ERROR on binding");
	} catch (const char* e) {
		error(e);
	}

	listen(sockfd, MAX_CLIENTS);

	FD_SET(sockfd, &read_fds);
	FD_SET(0, &read_fds);
	fdmax = sockfd;
}

void erase_client(int i) {
	FD_CLR(clients[i].socket, &read_fds);
	close(clients[i].socket);
	clients.erase(clients.begin() + i);

}

void close_connection() {
	for(int i = 0; i < clients.size(); i++)
		erase_client (i);
	close (sockfd);

	status_messges("Close connection\n");
	exit(EXIT_SUCCESS);
}

void add_link_to_queue(string url) {
	string link;
	size_t found = url.find("//");
	link.assign(url.substr(found + 1));
	Qsend.push(link);
}

void read_from_input() {
	fgets(buffer, BUFLEN - 1, stdin);
	buffer[strlen(buffer) - 1] = '\0';
	if (strncmp(buffer, "exit", 4) == 0) {
		close_connection();
		exit(EXIT_SUCCESS);
	}
	/* comanda de status */
	if (strncmp(buffer, "status", 6) == 0) {
		print_clients();
	}
	/* comanda de download */
	if (strncmp(buffer, "download", 8) == 0) {
		command.assign(buffer);
		size_t last = command.find_last_of(" ");
		/*extragem url introdus*/;
		url.assign(command.substr(last + 1));
		add_link_to_queue(url);

	}
}

/* clientul initiaza o noua conexiune */
void new_connection() {
	clilen = sizeof(cli_addr);
	try {
		if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen))
				== -1)
			throw("Error in accept");
		else {
			//adaug noul socket intors de accept() la multimea descriptorilor de citire
			FD_SET(newsockfd, &read_fds);
			if (newsockfd > fdmax)
				fdmax = newsockfd;
		}
	} catch (const char* e) {
		error(e);
	}
	/* adaugam clientul la lista de clienti */
	client c;
	c.socket = newsockfd;
	c.port = ntohs(cli_addr.sin_port);
	c.ip_address.assign(inet_ntoa(cli_addr.sin_addr));
	c._ready = true;

	clients.push_back(c);

	memset(msg, 0, BUFLEN);
	sprintf(msg, "Noua conexiune de la %s, port %d, socket_client %d\n ",
			inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
	status_messges(msg);
}

void send_mode_and_link_to_client(int i) {
	/* trimitem modul -e -r or nothing */
	string mod;
	string link = Qsend.front();
	Qsend.pop();
	clients[i].path_file.assign(link);

	if (_e_flag == true)
		mod.assign("-e");
	if (_r_flag == true)
		mod.append("-r");
	if (_e_flag == false && _r_flag == false)
		mod.assign("no_op");
	mod.append(" ");
	mod.append(link);

	try {
		memset(sendbuf, 0, BUFLEN);
		sprintf(sendbuf, "%s", mod.c_str());
		n = send(clients[i].socket, sendbuf, strlen(sendbuf), 0);
		clients[i].path_file.assign(link);

		memset(msg, 0, BUFLEN);
		sprintf(msg, "Send to downloading to %d:%s\n", clients[i].socket, sendbuf);
		status_messges(msg);

		if (n <= 0) {
			sprintf(err, "Connection close from client %d\n", clients[i].socket);
			throw(err);
		}

	} catch (const char* e) {
		error(e);
		erase_client(i);
	}
}

int  find_client(int fd) {
	for(int i = 0; i < clients.size(); i++) {
		if(clients[i].socket == fd)
			return i;
	}
	return -1;
}

string path_to_dir(string file){
	size_t find = file.find_last_of("/");
	string dir(".");
	dir.append(file.substr(0, find));
	return dir;
}

bool is_HTML_file(char *link) {
	if(strstr(link, ".html") || strstr(link, ".htm"))
		return true;
	return false;
}

void accept_connections()
{
	int fd_client;
	while (1) {
		tmp_fds = read_fds;
		/* trimitem linkuri pentru descarcare */
		if (clients.size() >= MIN_CLIENTS && Qsend.empty() == false) {
			for (int i = 0; i < clients.size(); i++) {
				if(Qsend.empty() == false && _finish_download == true) {
					/* alegem un client random catre care trimitem task-ul */
					srand (time(NULL));
					int x = rand()%clients.size();
					send_mode_and_link_to_client(x);
					_finish_download = false;
				}
			}
		} else  {
			if (clients.size() >= MIN_CLIENTS && Qsend.empty() == true && _download_nivel == true) {
				_download_nivel = false;
				/*crestem nivelul de recursivitate */
				DEPTH++;
				if(DEPTH == 1 && _r_flag == false) {
					close_connection();
					exit(EXIT_SUCCESS);
				}

				if(DEPTH == MAX_DEPTH && _r_flag == true) {
					close_connection();
					exit(EXIT_SUCCESS);
				}
				/* adugam  din Qsend in Qrecv */
				char path_to_file[BUFLEN];
				while(Qrecv.empty() == false) {
					string s = Qrecv.front();
					Qrecv.pop();
					memset(path_to_file, 0, BUFLEN);
					sprintf(path_to_file, ".%s",s.c_str());
					/* verificam daca fierul exista */
					if (access(path_to_file, F_OK) == -1)
							Qsend.push(s);
				}
				continue;
			}
		}

		try {
			if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1)
				throw("Error in select");
		} catch (const char* e) {
			error(e);
		}

		for (fd_client = 0; fd_client <= fdmax; fd_client++) {
			if (FD_ISSET(fd_client, &tmp_fds)) {
				/* input de la tastatura */
				if (fd_client == 0) {
					read_from_input();
				}

				if (fd_client == sockfd) {
					new_connection();
				}

				if (fd_client != 0 && fd_client != sockfd) {
					/*primim pagina descarcata*/
						memset(buffer, 0, BUFLEN);
						sprintf(buffer, "ACK");
						/* gasim clientul */
						int x = find_client(fd_client);
						/*creare director */
						char path_to_file[BUFLEN];
						sprintf(path_to_file, ".%s",clients[x].path_file.c_str());

						sprintf(msg, "Primim fisierul %s\n", path_to_file);
						status_messges(msg);

						string dir = path_to_dir(clients[x].path_file);

						sprintf(msg, "Cream directorul %s\n", dir.c_str());
						status_messges(msg);

						sprintf(cmd, "mkdir -p %s",dir.c_str());
						system(cmd);


						int f = open(path_to_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
						/* primim fisierul */
						while(1) {
							memset(recvbuf, 0, BUFLEN);

							n = recv(fd_client, recvbuf, BUFLEN, 0);
							send(fd_client, buffer, BUFLEN, 0);

							if(strncmp(recvbuf,"DONE",4) == 0)
								break;

							if(strncmp(recvbuf, "ERROR", 5) == 0)
								break;

							write(f, recvbuf, n);
						}
						close(f);
						sprintf(msg, "File Done\n");
						status_messges(msg);

						if(strncmp(recvbuf, "ERROR", 5) == 0) {
							memset(msg, 0, BUFLEN);
							sprintf(msg, "Eroare descarcare pagina\n");
							status_messges(msg);

							if(Qsend.empty() == true)
								_download_nivel = true;
							_finish_download = true;
							break;
						}
						/* primim link-urile catre pagini */
						if(is_HTML_file(path_to_file) == true) {

							memset(msg, 0, BUFLEN);
							sprintf(msg, "Primim linkurile de descarcat\n");
							status_messges(msg);

							while(1) {
								memset(recvbuf, 0, BUFLEN);
								n = recv(fd_client, recvbuf, BUFLEN, 0);
								send(fd_client, buffer, BUFLEN, 0);
								if(strncmp(recvbuf,"DONE",4) == 0) {
																break;
															}
								Qrecv.push(string(recvbuf));
							}

							memset(msg, 0, BUFLEN);
							sprintf(msg, "DONE\n\n");
							status_messges(msg);

						}
						if(Qsend.empty() == true)
							_download_nivel = true;

						_finish_download = true;
					}
			}
		} /*end for*/

	}/*end while*/
} /* end accept_connections*/

int main(int argc, char *argv[]) {
	/* verificam argumentele date serverului din cmdl */
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-r") == 0)
			_r_flag = true;
		if (strcmp(argv[i], "-e") == 0)
			_e_flag = true;
		if (strcmp(argv[i], "-p") == 0) {
			_p_flag = true;
			portno = atoi(argv[i + 1]);
		}
		if (strcmp(argv[i], "-o") == 0) {
			_o_flag = true;
			log_stdout.append(argv[i + 1]);
			log_stdout.append(".stdout");
			fout.open(log_stdout.c_str(), ios::out);

			log_error.append(argv[i + 1]);
			log_error.append(".stderr");
			ferr.open(log_error.c_str(), ios::out);
			_print_enable = true;
		}
	}
	/* argumente invalide */
	try {
		if (_p_flag == false)
			throw "Usage: ./server [-r] [-e] [-o logfile] -p <port>\n";
	} catch (const char* e) {
		error(e);
		exit(EXIT_FAILURE);
	}
	/* serverul asculta conexiuni pe portul deschis */
	server_listening();
	accept_connections();

	return 0;
}
