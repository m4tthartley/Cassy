
#ifdef _WIN32
#	include <winSock2.h>
#	include <windows.h>
#	include <ws2tcpip.h>
#endif
#ifdef __linux__
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netdb.h>
#	include <sys/ioctl.h>
#	include <arpa/inet.h>
#	include <errno.h>
#	include <string.h>

// #	include <fctl.h>
#	define closesocket close
#	define Sleep sleep
#	define WSAGetLastError() errno
#	define ioctlsocket ioctl
#endif
#include <stdio.h>

#include "shared.cc"

#define MAX_CLIENTS 16

struct Client {
	int socket;
};

Client clients[MAX_CLIENTS] = {};

int main () {
	int test1 = SOCKERR_INVALID;
	int test2 = SOCKERR_ERROR;

	for (int i = 0; i < MAX_CLIENTS; ++i) {
		clients[i].socket = -1;
	}

	int socketHandle;

#ifdef _WIN32
	WSADATA winsockData;
	WSAStartup(MAKEWORD(1, 1), &winsockData);
#endif

	fd_set socketSet;
	fd_set tempSocketSet;
	FD_ZERO(&socketSet);
	FD_ZERO(&tempSocketSet);

	addrinfo hints = {};
	hints.ai_family = AF_INET /*AF_UNSPEC*/;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	char recvBuffer[SOUND_BUFFER_SIZE];
	int maxSocket;

	addrinfo *serverInfo;
	int result;
	if ((result = getaddrinfo(NULL, PORT, &hints, &serverInfo)) == 0) {
		socketHandle = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
		if (socketHandle != SOCKERR_INVALID) {
			maxSocket = socketHandle;
			// fcntl(socketHandle, F_SETFL, O_NONBLOCK);
			unsigned long nonBlock = 1;
			int nonBlockResult = ioctlsocket(socketHandle, FIONBIO, &nonBlock);
			if (nonBlockResult != 0) {
				assert(false);
			}

			int value = 1;
			if (setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, (char*)&value, sizeof(int)) != SOCKERR_ERROR) {
				if (bind(socketHandle, serverInfo->ai_addr, serverInfo->ai_addrlen) != SOCKERR_ERROR) {
					freeaddrinfo(serverInfo);

					if (listen(socketHandle, 10) != SOCKERR_ERROR) {
						FD_SET(socketHandle, &socketSet);

						printf("Listening for connections... \n");

						while (true) {
							tempSocketSet = socketSet;
							// todo: Might need to set the first parameter for linux
							int selectResult = select(maxSocket+1, &tempSocketSet, NULL, NULL, NULL);
							if (selectResult == SOCKERR_ERROR) {
								int error = WSAGetLastError();
								assert(false);
							}

							if (FD_ISSET(socketHandle, &tempSocketSet)) {
								sockaddr clientAddr;
								socklen_t clientAddrSize = sizeof(sockaddr);
								int acceptHandle = accept(socketHandle, &clientAddr, &clientAddrSize);
								if (acceptHandle != SOCKERR_INVALID) {
									if (acceptHandle > maxSocket) {
										maxSocket = acceptHandle;
									}

									char ipBuffer[16];
									sockaddr_in *inAddr = (sockaddr_in*)&clientAddr;
									char *remoteIp = (char*)inet_ntop(/*clientAddr.sa_family*/AF_INET, &inAddr->sin_addr, ipBuffer, 16);
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
								} else {
									int error = WSAGetLastError();
									printf("accept error %i \n", error);
									assert(false);
								}
							}

							for (int i = 0; i < MAX_CLIENTS; ++i) {
								if (clients[i].socket != -1 && FD_ISSET(clients[i].socket, &tempSocketSet)) {
									int bytesRead = recv(clients[i].socket, recvBuffer, SOUND_BUFFER_SIZE, 0);
									AudioPacketHeader *packet = (AudioPacketHeader*)&recvBuffer[0];
									void *packetData = packet + 1;

									if (bytesRead == SOCKERR_ERROR) {
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

									if (strncmp(packet->id, "CASY", 4) != 0) {
										printf("Packet id is incorrect! \n");
									}
									if (bytesRead != sizeof(AudioPacketHeader) + packet->size) {
										printf("partial packet %i / %lu \n", bytesRead, sizeof(AudioPacketHeader) + packet->size);
										send(clients[i].socket, (char*)packetData, 4, 0);
									} else {
										send(clients[i].socket, (char*)packet, sizeof(AudioPacketHeader) + packet->size, 0);
										// printf("Recv %s \n", recvBuffer);

										memset(recvBuffer, 0, SOUND_BUFFER_SIZE);
									}
								}
							}

							sleepMsecs(50);
						}

#if 0
						while (true) {
							printf("Waiting for connection... \n");

							int acceptHandle = accept(socketHandle, &clientAddr, NULL/*&clientAddrSize*/);
							if (acceptHandle != SOCKERR_INVALID) {
								while (true) {
									
									int bytesRead = recv(acceptHandle, recvBuffer, SOUND_BUFFER_SIZE, 0);
									printf("bytes received %i \n", bytesRead);
									if (bytesRead == SOCKERR_ERROR) {
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