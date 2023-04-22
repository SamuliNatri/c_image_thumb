
// Minimal web server. No proper error handling.

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>
#define _GNU_SOURCE
#include <string.h>

#define MAX_REQUEST_SIZE 100000
#define MAX_RESPONSE_SIZE 100000

typedef struct {
	char value[MAX_REQUEST_SIZE];
	size_t size;
} Request;

typedef struct {
	int socket;
	struct sockaddr address;
	socklen_t length;
} Host;

Host w_new_host();
void w_socket(Host *host);
void w_accept(Host *host1, Host *host2);
void w_receive(Host *host, Request *request);
void w_handle_request(Request *request);
void w_build_response(char *response);
void w_send(Host *host, char *buffer, size_t len);

int main() {
	char response[MAX_RESPONSE_SIZE];
	Request request = {0};
	Host server = w_new_host();
	w_socket(&server);

	for(;;) {
		Host client = w_new_host();
		w_accept(&server, &client);
		w_receive(&client, &request);
		w_handle_request(&request);
		w_build_response(response);
		w_send(&client, response, strlen(response));
	}

	close(server.socket);
	return 0;
}

Host w_new_host() {
	Host host = {0};
	host.length = sizeof(host.address);
	return host;
}

void w_send(Host *host, char *buffer, size_t len) {
	send(host->socket, buffer, len, 0);
}

void w_receive(Host *host, Request *request) {
	int count = 0;
	int total = 0;

	do {
		count = recv(host->socket, request->value+total, MAX_REQUEST_SIZE - total, 0);
		total += count;
	} while(count > 0);

	request->size = total;
}

void w_accept(Host *host1, Host *host2) {
	host2->socket = accept(host1->socket, &host2->address, &host2->length);
	struct timeval timeout = { .tv_sec = 0, .tv_usec = 500000};  setsockopt(host2->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

void w_socket(Host *host) {
	struct addrinfo hints = {0};
	struct addrinfo *bind_address;

	// Specify what kind of socket address we want to get.

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// Get the address.
	
	getaddrinfo(0, "8080", &hints, &bind_address);

	// Create a socket.
	
	host->socket = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

	// Set socket options.
	
	setsockopt(host->socket, SOL_SOCKET, SO_REUSEADDR, &(char){1}, sizeof(int));

	// Bind the socket.
	
	bind(host->socket, bind_address->ai_addr, bind_address->ai_addrlen);

	// Start listening.
	
	listen(host->socket, 10);
}
