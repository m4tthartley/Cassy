
#include <winSock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include "shared.cc"

#define PORT "3430"

#define SOUND_HTZ 44100
#define SOUND_CHANNELS 2
#define SOUND_SAMPLE_BYTES 2
#define SOUND_BYTES_PER_SEC (SOUND_HTZ * SOUND_CHANNELS * SOUND_SAMPLE_BYTES)
#define SOUND_BUFFER_SIZE (SOUND_BYTES_PER_SEC * 4)

#define MAX_CLIENTS 16

struct Client {
	int socket = -1;
};

Client clients[MAX_CLIENTS] = {};

int main () {
	int socketHandle;

	WSADATA winsockData;
	WSAStartup(MAKEWORD(1, 1), &winsockData);

	fd_set socketSet;
	fd_set tempSocketSet;
	FD_ZERO(&socketSet);
	FD_ZERO(&tempSocketSet);

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
			// fcntl(socketHandle, F_SETFL, O_NONBLOCK);
			unsigned long nonBlock = 1;
			int nonBlockResult = ioctlsocket(socketHandle, FIONBIO, &nonBlock);
			if (nonBlockResult != 0) {
				assert(false);
			}

			int value = 1;
			if (setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, (char*)&value, sizeof(int)) != SOCKET_ERROR) {
				if (bind(socketHandle, serverInfo->ai_addr, serverInfo->ai_addrlen) != SOCKET_ERROR) {
					freeaddrinfo(serverInfo);

					if (listen(socketHandle, 10) != SOCKET_ERROR) {
						FD_SET(socketHandle, &socketSet);

						while (true) {
							tempSocketSet = socketSet;
							// todo: Might need to set the first parameter for linux
							int selectResult = select(0, &tempSocketSet, NULL, NULL, NULL);
							if (selectResult == SOCKET_ERROR) {
								int error = WSAGetLastError();
								assert(false);
							}

							if (FD_ISSET(socketHandle, &tempSocketSet)) {
								sockaddr clientAddr;
								int clientAddrSize;
								int acceptHandle = accept(socketHandle, &clientAddr, NULL/*&clientAddrSize*/);
								if (acceptHandle != INVALID_SOCKET) {
									char ipBuffer[16];
									sockaddr_in *inAddr = (sockaddr_in*)&clientAddr;
									char *remoteIp = (char*)InetNtop(/*clientAddr.sa_family*/AF_INET, &inAddr->sin_addr, ipBuffer, 16);
									printf("Connection from %s \n", ipBuffer);
									int error = WSAGetLastError();
									int x = 0;

									for (int i = 0; i < MAX_CLIENTS; ++i) {
										if (clients[i].socket == -1) {
											clients[i].socket = acceptHandle;
											FD_SET(acceptHandle, &socketSet);
											break;
										}
									}
								}
							}

							for (int i = 0; i < MAX_CLIENTS; ++i) {
								if (clients[i].socket != -1 && FD_ISSET(clients[i].socket, &tempSocketSet)) {
									int bytesRead = recv(clients[i].socket, recvBuffer, SOUND_BUFFER_SIZE, 0);
									printf("bytes received %i \n", bytesRead);
									if (bytesRead == SOCKET_ERROR) {
										// int x = 0;
										int error = WSAGetLastError();
										int x = 0;
										printf("Client disconnected with error %i \n", error);

										closesocket(clients[i].socket);
										FD_CLR(clients[i].socket, &socketSet);
										clients[i].socket = -1;
										continue;
									}
									if (bytesRead == 0) {
										printf("Client disconnected \n");

										closesocket(clients[i].socket);
										FD_CLR(clients[i].socket, &socketSet);
										clients[i].socket = -1;
										continue;
									}

									send(clients[i].socket, recvBuffer, bytesRead, 0);
									printf("Recv %s \n", recvBuffer);

									memset(recvBuffer, 0, SOUND_BUFFER_SIZE);
								}
							}

							Sleep(100);
						}

#if 0
						while (true) {
							printf("Waiting for connection... \n");

							int acceptHandle = accept(socketHandle, &clientAddr, NULL/*&clientAddrSize*/);
							if (acceptHandle != INVALID_SOCKET) {
								while (true) {
									
									int bytesRead = recv(acceptHandle, recvBuffer, SOUND_BUFFER_SIZE, 0);
									printf("bytes received %i \n", bytesRead);
									if (bytesRead == SOCKET_ERROR) {
										// int x = 0;
										int error = WSAGetLastError();
										int x = 0;
										printf("Error %i \n", error);
									}
									send(acceptHandle, recvBuffer, bytesRead, 0);
									printf("Recv %s \n", recvBuffer);

									memset(recvBuffer, 0, SOUND_BUFFER_SIZE);
								}
							} else {
								int error = WSAGetLastError();
								int x = 0;
							}

							Sleep(100);
						}
#endif
					}
				}
			}
		}
	}
}