CXX=g++
EXTRA=-lnsl

build: server client

server: server-http.cpp
	$(CXX) server-http.cpp -o server

client: client-http.cpp
	$(CXX) client-http.cpp -o client

clean:
	rm client
	rm server
	rm -rf www.*
	rm *.stdout
	rm *.stderr
