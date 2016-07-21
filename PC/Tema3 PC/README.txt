
Arhiva include urmatoarele fisiere:
	-> client-http.cpp
	-> server-http.cpp
	-> README.txt
	->Makefile
	Pentru rularea temei, e nevoie ca sa NU EXISTE directorul descarcat,
deoarece, inainte de a trimite un link spre descarcare, verific existenta 
fiserului.
	Pentru o noua rulare a temei, e nevoie de schimbat portul(vechiul port e
	pe bind);
	Pentru parametrii -r -e, tema ruleaza minute bune(aproximativ 8-10). 


	--server-http.cpp--
	Pentru multiplexarea comunicatiei, am folosit apelul select;
	Am utilizat laboratoarele de multiplexare a comunicatiei si de HTTP.

	Pentru a simula nivelurile de recursivitate, am folosit 2 cozi, Qsend si 
Qrecv. Folosind Qsend, trimit catre clienti link-urile de descarcat, pentru
nivelul meu. In Qrecv, primesc toate link-urile de pe nivelul urmator. Cind 
Qsend devine vida, Transfer link-urile care inca nu au fost descarcate din Qrecv
in Qrecv.
	
	Am dat nume intuitive functiilor ajutatoare, de verificare si parsare, asa ca 
nu cred ca e relevanta explicarea lor.

--accept_connections--
	Functia care practic realizeaza descarcarea. 
	->Descarcarea nu porneste atata timp cat numarul de clienti e mai mic decit 5;
	->Daca am primit comanda de download, trimitem modul de descarcare(-e -r) si
		de descarcare catre un client random, in functia 
		send_mode_and_link_to_client(x);

	->Dupa apelul select, acolo unde asteptam conexiunile: gasim clientul care
		ne trimite pagina ceruta:
		-cream directorul cu apelul system;
		-scriem in fisier datele de pe socket, pina cind intilnim flagul DONE.
		-daca am cerut o pagina HTML, asteptam si link-urile de pe pagina ceruta
		-daca am descarcat toate link-urile de pe nivelul nostru trecem pe urmatorul
			nivel.
 
	->Daca nu avem setat flagul -r, atunci descarcam pina la nivelul 1 de recursivitate
	(pagina si link-urile din pagina), altfel pina la nivelul 5;


	--client-http--
	Clientul se va conecta la 2 socketi:unul care comunica cu serverul HTTP, al doilea
 	care comunica cu serverul master, catre care va trimite paginile descarcate.
	--connect_to_server--
	ne conectam la server pe portul si adresa ip date ca parametru din linia de comanda
	In main(), intr-o bucla while(1), primim pagina care trebuie descarcata, impreuna cu
parametrii de descarcare.
	--split--
	parsam si extragem modul de descarcare, extragem adresa serverului si cererea 
	care trebuie sa o trimitem catre serverul de HTTP.
	--connect_to_HTTP()--
	ne conectam la serverul de HTTP, pe socketul sockfd_http;

	--download_file(char* )--
	functia care descarca linkul primit ca parametru;
	Folosim protocolul HTTP/1.0
	-> trimitem cererea pentru pagina primita ca parametru.
	-> daca primim un cod de eroare, inchidem conexiunea catre serverul HTTP si
	 trimitem mesajul de eroare catre serverul-master
	->daca codul de eroare e HTTP_OK(200), sarim peste header-ul HTTP si citim cite o linie de
	de pe socket;
	->pentru o pagina .html, extragem linkurile de pe pagina, iar daca avem flagul -e activat,
	extragem si linkurile atasementelor;
	->daca pagina descarcata e .html, trimitem link-urile gasite pe pagina catre serverul-master.
	

	


	

	
