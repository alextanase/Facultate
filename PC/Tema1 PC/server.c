#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

#define simplu 0
#define stop_wait 1
//int guess_smaller(int low, int up){
//	int new_number;
//	new_number = up + low / 2;
//	return new_number;
//	
//}
void send_number(int x){
	msg number;
	sprintf(number.payload, "%d", x);
	number.len = strlen(number.payload) + 1;
	(void) send_message(&number);	
	printf("[Server]%s\n", number.payload);
}
void send_exit (){	
	msg e;
	sprintf(e.payload, "%s", "exit");
	e.len = sizeof("exit");
	//Send Hello message
	(void) send_message (&e);
	printf("[Server]%s\n", e.payload);
}
void send_hello (){
	//Build Hello message
	msg h;
	sprintf(h.payload, "%s", "Hello");
	h.len = sizeof("Hello");
	//Send Hello message
	(void) send_message (&h);
	printf("[Server]%s\n", h.payload);
	}
void send_yey (){
	//Build Yey message
	msg y;
	sprintf(y.payload, "%s", "YEY");
	y.len = sizeof("YEY");
	//Send Yey message
	(void) send_message (&y);
	printf("[Server]%s\n", y.payload);
	}
void send_ok (){
	//Build Hello message
	msg o;
	sprintf(o.payload, "%s", "OK");
	o.len = sizeof("OK");
	//Send Hello message
	(void) send_message (&o);
	printf("[Server]%s\n", o.payload);
	}
void send_ack () {
	/* Build ACK message */
	msg a;
	sprintf (a.payload, "%s", "ACK");
	a.len = sizeof("ACK");

	/* Send message */
	(void) send_message (&a);
	printf("[Server]Trimit %s\n", a.payload);
	}
void send_nack () {
	/* Build NACK message */
	msg n;
	sprintf (n.payload, "%s", "NACK");
	n.len = sizeof("NACK");

	/* Send message */
	(void) send_message (&n);
	printf("[Server] Trimit %s\n", n.payload);
}
void mod_simplu (){
	msg r, number;
	int x = 500, low = 0, up = 1000;
	recv_message(&r);
	printf("[Client] %s\n", r.payload);
	send_hello();
	recv_message(&r);
	printf("[Client] %s\n", r.payload);
	recv_message(&r);
	printf("[Client] %s\n", r.payload);
	recv_message(&r);
	printf("[Client] %s\n", r.payload);
	send_yey();
	send_ok();
	recv_message(&r);
	printf("[Client] %s\n", r.payload);
	sprintf(number.payload, "%d", x);
	number.len = strlen(number.payload)+1;
	//Send number message
	(void) send_message (&number);
	printf("[Server]%s\n", number.payload);
	recv_message(&r);
	printf("[Client] %s\n", r.payload);
	while(strcmp(r.payload,"success\n") != 0){
		if(strcmp(r.payload,"smaller\n") == 0){
			up = x ;
			x = (low + up) / 2;
			send_number(x);
		}
		if(strcmp(r.payload,"bigger\n") == 0){
			low = x;
			x = (low + up) / 2;
			send_number(x);
		}
		recv_message(&r);
		printf("[Client] %s\n", r.payload);
	}	
	recv_message(&r);
	printf("[Client] %s\n", r.payload);
	send_exit();//Trimit exit
}
void mod_ack(){
	msg r, number;
	int x = 500, low = 0, up = 1000;
	recv_message(&r);
	printf("[Client]%s\n", r.payload);
	send_ack();
	send_hello();
	recv_message(&r);
	printf("[Client]%s\n", r.payload);
	recv_message(&r);
	printf("[Client]%s\n", r.payload);
	send_ack();
	recv_message(&r);
	printf("[Client]%s\n", r.payload);
	send_ack();
	recv_message(&r);
	printf("[Client]%s\n", r.payload);
	send_ack();
	send_yey();
	recv_message(&r);
	printf("[Client]%s\n", r.payload);
	send_ok();
	recv_message(&r);
	printf("[Client]%s\n", r.payload);
	recv_message(&r);
	printf("[Client]%s\n", r.payload);
	send_ack();
	sprintf(number.payload, "%d", x);
	number.len = strlen(number.payload)+1;
	//Send number message
	(void) send_message (&number);
	printf("[Server]%s\n", number.payload);//trimit 500
	recv_message(&r); //primesc ack
	printf("[Client]%s\n", r.payload);
	recv_message(&r);
	printf("[Client] %s\n", r.payload);
	send_ack();
	while(strcmp(r.payload,"success\n") != 0){
		if(strcmp(r.payload,"smaller\n") == 0){
			up = x ;
			x = (low + up) / 2;
			send_number(x);
			recv_message(&r);//primesc ack
			printf("[Client] %s\n", r.payload);
			send_ack();
					
	}
		if(strcmp(r.payload,"bigger\n") == 0){
			low = x;
			x = (low + up) / 2;
			send_number(x);
			recv_message(&r);//primesc ack
			printf("[Client] %s\n", r.payload);
			send_ack();
		}
		recv_message(&r);
		printf("[Client] %s\n", r.payload);
	}
	send_ack();	
	recv_message(&r);
	printf("[Client] %s\n", r.payload);
	//send_ack();
	send_exit();
	recv_message(&r);
	printf("[Client] %s\n", r.payload);
}
int main(int argc,char** argv)
{
	int res;
	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);
		// MODUL SIMPLU
		if(argc == 1){
			mod_simplu();
		}
		// MODUL ACK
		if(strcmp(argv[1],"ack") == 0){
			mod_ack();
		}
		// MODUL PARITY
		//if(strcmp(argv[1],"parity") == 0)
		exit(0);
		/* send dummy ACK */
		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return -1;
		}
		printf("[RECEIVER] Finished receiving..\n");
		return 0;
		
}
