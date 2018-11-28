#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "hacking.h" // for displaying packet data (memory dump function)

#define PORT 7890 // the port users will be connecting to

int main(void)
{
	int sockfd, new_sockfd; // declare the socket file descriptors
	struct sockaddr_in host_addr, client_addr; // internet address information in the sockaddr_in struct
	socklen_t sin_size; // size of client socket
	int recv_length = 1, yes = 1;
	char buffer[1024];

	// setting up a socket with socket
	// PF_INET for ipv4 protocol
	// SOCK_STREAM for TCP stream socket
	// 0 because there is only one protocol in the PF_INET family
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		fatal("in socket");

	// setting the sockfd (file descriptor) socket options with setsockopt
	// SO_REUSEADDR = true, allows reuse of given address for binding (without this, we can't use a port that's already in use)
	// socket level set to SOL_SOCKET
	// &yes is a pointer to the data that the option should be set to
	// sizeof(int) is the length of that data
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		fatal("setting socket option SO_REUSEADDR");

	host_addr.sin_family = AF_INET; // host byte order, address family = AF_INET
	host_addr.sin_port = htons(PORT); // short, network byte order
	host_addr.sin_addr.s_addr = 0; // automatically fill with my IP
	memset(&(host_addr.sin_zero), '\0', 8); // zero the rest of the struct
	
	// bind with the file descriptor, the address structure, and the size of the address structure
	// This binds the socket to the current IP address on port PORT
	if (bind(sockfd, (struct sockaddr *)&host_addr, sizeof(struct sockaddr)) == -1)
		fatal("binding to socket");
	
	// listen on socket with max size backlog
	if (listen(sockfd, 5) == -1)
		fatal("listening on socket");
	
	// loop that accepts incoming connections
	while (1) { 
		sin_size = sizeof(struct sockaddr_in);
		// accept new connection on socket, with client information and pointer to sockaddr_in size
		new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);

		if (new_sockfd == -1)
			fatal("accepting connection");

		printf("Server: got connection from %s on port %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		// send the 21 byte string to the new socket that describes the new connection (final argument is a flag)
		send(new_sockfd, "\n> Hello, connection\n", 21, 0);
		recv_length = recv(new_sockfd, &buffer, 1024, 0);
		
		// receive loop
		while (recv_length > 0) {
			printf("RECV: %d bytes", recv_length);
			dump(buffer, recv_length);
			// pointer to buffer and maximum length to write to that buffer,
			// returns the amount of bytes it actually had to write
			recv_length = recv(new_sockfd, &buffer, 1024, 0);
		}
		close(new_sockfd);
	}
	return 0;
}
