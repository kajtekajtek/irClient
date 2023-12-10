irClient : main.cpp client.h connection.h client.cpp connection.cpp
	g++ -o irClient main.cpp client.cpp connection.cpp

clean : 
	rm irClient
