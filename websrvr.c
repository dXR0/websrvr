#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

// based on: https://bruinsslot.jp/post/simple-http-webserver-in-c/

// socket -> bind -> listen -> accept -> read/write

#define PORT 0x1f1f // 7967
#define BUF_SIZE 2048

int main(int argc, char **argv) {
	char buf[BUF_SIZE];
	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket == -1) {
		perror("webserver (socket)");
		return 1;
	}
	struct sockaddr_in host_addr;
	host_addr.sin_family = AF_INET;
	host_addr.sin_port = PORT;	// htons(PORT) if PORT is defined in decimal
							// 0x1f1f is the same in big and little endian - bit of a hack (from tsoding)
	host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int host_addrlen = sizeof(host_addr);

	struct sockaddr_in client_addr;
	int client_addrlen = sizeof(client_addr);

	int bound = bind(tcp_socket, (struct sockaddr *)&host_addr, host_addrlen);
	if (bound != 0) {
		perror("webserber (bind)");
		return 1;
	}
	int listened = listen(tcp_socket, SOMAXCONN);
	if (listened != 0) {
		perror("webserver (listen)");
		return 1;
	}
	while (1){
		int newsockfd = accept(tcp_socket, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
		if (newsockfd < 0) {
			perror("webserver (accept)");
			continue;
		}

		int sockn = getsockname(newsockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);
		if (sockn < 0) {
			perror("webserver (getsockname)");
			continue;
		}
		
		int valread = read(newsockfd, buf, BUF_SIZE);
		if (valread < 0) {
			perror("webserver (read)");
			continue;
		}

		char method[BUF_SIZE], uri[BUF_SIZE], version[BUF_SIZE];
		sscanf(buf, "%s %s %s", method, uri, version);
		printf("[%s:%u] %s %s %s\n", 
			inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
			method, uri, version
		);

		char msg[] = "HTTP/1.0 200 OK\r\nServer: ws-c\r\nContent-Type: application/json\r\n\r\n{\"hello\": \"sailor\"}";
		size_t msg_len = strlen(msg);
		int valwrite = write(newsockfd, msg, msg_len);
		if (valwrite < 0) {
			perror("webserver (write)");
			continue;
		}
		close(newsockfd);
	}
	return 0;
}