compile: server.out client.out

server.out: server.cpp
		g++ -o server server.cpp -lpthread
client.out: client.cpp
		g++ -o client client.cpp -lpthread
run_server: server.out
		./server 9011
run_client: client.out
		./client localhost 9011
clean:
		rm -rf server client