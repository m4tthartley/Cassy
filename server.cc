
#include <winSock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#include <stdio.h>

#define PORT "3430"

#define SOUND_HTZ 44100
#define SOUND_CHANNELS 2
#define SOUND_SAMPLE_BYTES 2
#define SOUND_BYTES_PER_SEC (SOUND_HTZ * SOUND_CHANNELS * SOUND_SAMPLE_BYTES)
#define SOUND_BUFFER_SIZE (SOUND_BYTES_PER_SEC * 4)

int main () {
	int socketHandle;

	WSADATA winsockData;
	WSAStartup(MAKEWORD(1, 1), &winsockData);

	addrinfo hints = {};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	char recvBuffer[SOUND_BUFFER_SIZE];

	addrinfo *serverInfo;
	int result;
	if (result = getaddrinfo(NULL, PORT, &hints, &serverInfo) == 0) {
		socketHandle = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
		if (socketHandle != INVALID_SOCKET) {
			int value = 1;
			if (setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, (char*)&value, sizeof(int)) != SOCKET_ERROR) {
				if (bind(socketHandle, serverInfo->ai_addr, serverInfo->ai_addrlen) != SOCKET_ERROR) {
					freeaddrinfo(serverInfo);

					if (listen(socketHandle, 10) != SOCKET_ERROR) {
						sockaddr clientAddr;
						int clientAddrSize;
						while (true) {
							printf("Waiting for connection... \n");

							int acceptHandle = accept(socketHandle, &clientAddr, NULL/*&clientAddrSize*/);
							if (acceptHandle != INVALID_SOCKET) {
								while (true) {
									memset(recvBuffer, 0, SOUND_BUFFER_SIZE);
									int bytesRead = recv(acceptHandle, recvBuffer, SOUND_BUFFER_SIZE, 0);
									if (bytesRead == SOCKET_ERROR) {
										// int x = 0;
										int error = WSAGetLastError();
										int x = 0;
									}
									send(acceptHandle, "ho", 2, 0);
									printf("Recv %s \n", recvBuffer);
								}
							} else {
								int error = WSAGetLastError();
								int x = 0;
							}

							Sleep(1000);
						}
					}
				}
			}
		}
	}
}