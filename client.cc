
#include <winSock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <dsound.h>
#include <stdio.h>
#include <stdint.h>

// #include <gj_lib.h>
#include "shared.cc"

#pragma pack(push, 1)
struct WavFormatChunk {
	char id[4];
	uint32_t size;
	uint16_t formatTag;
	uint16_t channels;
	uint32_t samplesPerSec;
	uint32_t bytesPerSec;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	/*uint16_t cbSize;
	int16_t validBitsPerSample;
	int32_t channelMask;
	char subFormat[16];*/
};

struct WavHeader {
	// header
	char chunkId[4];
	uint32_t chunkSize;
	char waveId[4];

	WavFormatChunk format;
	// format
	/*char formatId[4];
	uint32_t formatSize;
	uint16_t formatTag;
	uint16_t channels;
	uint32_t samplesPerSec;
	uint32_t bytesPerSec;
	uint16_t blockAlign;
	uint16_t bitsPerSample;*/
	/*uint16_t cbSize;
	int16_t validBitsPerSample;
	int32_t channelMask;
	char subFormat[16];*/

	// data
	char dataId[4];
	uint32_t dataSize;
	
	// actual data
};
#pragma pack(pop)

#if 0
short swapEnd16 (short num) {
	short result;
	char *num8 = (char*)&num;
	char *result8 = (char*)&result;
	result8[0] = num8[1];
	result8[1] = num8[0];
	return result;
}

int swapEnd32(int num) {
	int result;
	char *num8 = (char*)&num;
	char *result8 = (char*)&result;
	result8[0] = num8[3];
	result8[1] = num8[2];
	result8[2] = num8[1];
	result8[3] = num8[0];
	return result;
}
#endif

void writeAudioToFile (char *filename, void *data, size_t size) {
	WavHeader header = {};
	header.chunkId[0] = 'R';
	header.chunkId[1] = 'I';
	header.chunkId[2] = 'F';
	header.chunkId[3] = 'F';
	header.chunkSize = (sizeof(WavHeader) - 8) + size;
	header.waveId[0] = 'W';
	header.waveId[1] = 'A';
	header.waveId[2] = 'V';
	header.waveId[3] = 'E';

	header.format.id[0] = 'f';
	header.format.id[1] = 'm';
	header.format.id[2] = 't';
	header.format.id[3] = ' ';
	header.format.size = sizeof(WavFormatChunk) - 8;
	header.format.formatTag = 1;
	header.format.channels = SOUND_CHANNELS;
	header.format.samplesPerSec = SOUND_HTZ;
	header.format.bytesPerSec = SOUND_BYTES_PER_SEC;
	header.format.blockAlign = SOUND_CHANNELS * SOUND_SAMPLE_BYTES;
	header.format.bitsPerSample = SOUND_SAMPLE_BYTES * 8;

	header.dataId[0] = 'd';
	header.dataId[1] = 'a';
	header.dataId[2] = 't';
	header.dataId[3] = 'a';
	header.dataSize = size;

	FILE *file;
	fopen_s(&file, filename, "wb");
	fwrite(&header, sizeof(WavHeader), 1, file);
	fwrite(data, size, 1, file);
	fclose(file);
}

int main () {
	printf("Voice recording test \n");
	
	StackAllocator transient = createStackAllocator(megabytes(1));

	// Make sure these are the sizes I expect on whatever compiler I'm using
	assert(sizeof(short) == 2);
	assert(sizeof(int) == 4);
	assert(sizeof(long) == 8);

	/*WavHeader testHeader = {};
	FILE *testFile;
	fopen_s(&testFile, "output.wav", "r");
	fread(&testHeader, sizeof(WavHeader), 1, testFile);
	fclose(testFile);

	WavHeader shootHeader = {};
	FILE *shootFile;
	fopen_s(&shootFile, "shoot.wav", "r");
	fread(&shootHeader, sizeof(WavHeader), 1, shootFile);
	fclose(shootFile);

	exit(0);*/

	/*PlaySound("output.wav", NULL, SND_FILENAME);*/

	bool dsCaptureInitialised = false;
	bool dsOutputInitialised = false;

	char *recording = (char*)malloc(SOUND_BUFFER_SIZE);

	WAVEFORMATEX waveFormat = {};
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = SOUND_CHANNELS;
	waveFormat.nSamplesPerSec = SOUND_HTZ;
	waveFormat.nAvgBytesPerSec = SOUND_BYTES_PER_SEC;
	waveFormat.nBlockAlign = SOUND_CHANNELS * SOUND_SAMPLE_BYTES;
	waveFormat.wBitsPerSample = SOUND_SAMPLE_BYTES * 8;

	IDirectSoundCapture8 *dsCapture;
	IDirectSoundCaptureBuffer *dsCaptureBuffer = NULL;
	HRESULT dsResult = DirectSoundCaptureCreate8(&DSDEVID_DefaultVoiceCapture,
												 &dsCapture, NULL);
	if (dsResult == DS_OK) {
		DSCCAPS dsCaps = {};
		dsCaps.dwSize = sizeof(DSCCAPS);
		dsCapture->GetCaps(&dsCaps);

		DSCBUFFERDESC dsBufferDesc = {};
		dsBufferDesc.dwSize = sizeof(DSCBUFFERDESC);
		dsBufferDesc.dwFlags = 0;
		dsBufferDesc.dwBufferBytes = SOUND_BUFFER_SIZE;
		dsBufferDesc.lpwfxFormat = &waveFormat;

		dsResult = dsCapture->CreateCaptureBuffer(&dsBufferDesc, &dsCaptureBuffer, NULL);
		if (dsResult == DS_OK) {
			dsCaptureInitialised = true;
			dsCaptureBuffer->Start(DSCBSTART_LOOPING);

			/*while (true) {
				DWORD status;
				dsCaptureBuffer->GetStatus(&status);
				if (!(status & DSCBSTATUS_CAPTURING)) {
					break;
				}
				if (status & DSCBSTATUS_LOOPING) {
				}
			}

			void *region1;
			DWORD region1Size;
			void *region2;
			DWORD region2Size;
			dsResult = dsCaptureBuffer->Lock(0, SOUND_BUFFER_SIZE,
				&region1, &region1Size, &region2, &region2Size,
				DSCBLOCK_ENTIREBUFFER);
			if (dsResult == DS_OK) {
				writeAudioToFile("output.wav", region1, region1Size);
				memcpy(recording, region1, SOUND_BUFFER_SIZE);

				dsCaptureBuffer->Unlock(&region1, region1Size, &region2, region2Size);
			}

			dsCaptureBuffer->Stop();*/
		}
	}

	HWND desktopWindow = GetDesktopWindow();

	IDirectSound8 *dsOutput;
	IDirectSoundBuffer *secondaryBuffer = NULL;
	if (DirectSoundCreate8(0, &dsOutput, 0) == DS_OK) {
		if (dsOutput->SetCooperativeLevel(desktopWindow, DSSCL_PRIORITY) == DS_OK) {
			DSBUFFERDESC bufferDesc = {};
			bufferDesc.dwSize = sizeof(DSBUFFERDESC);
			bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
			IDirectSoundBuffer *primaryBuffer;
			if (dsOutput->CreateSoundBuffer(&bufferDesc, &primaryBuffer, 0) == DS_OK) {
				HRESULT dsResult;
				if (SUCCEEDED(dsResult = primaryBuffer->SetFormat(&waveFormat))/* == DS_OK*/) {
					DSBUFFERDESC secondBufferDesc = {};
					secondBufferDesc.dwSize = sizeof(DSBUFFERDESC);
					secondBufferDesc.dwFlags = DSBCAPS_GLOBALFOCUS; /*| DSBCAPS_GETCURRENTPOSITIOH*/
					secondBufferDesc.dwBufferBytes = SOUND_BUFFER_SIZE;
					secondBufferDesc.lpwfxFormat = &waveFormat;
					if (dsOutput->CreateSoundBuffer(&secondBufferDesc, &secondaryBuffer, 0) == DS_OK) {
						dsOutputInitialised = true;

						/*void *region1;
						DWORD region1Size;
						void *region2;
						DWORD region2Size;
						secondaryBuffer->Lock(0, SOUND_BUFFER_SIZE, &region1, &region1Size, &region2, &region2Size, DSCBLOCK_ENTIREBUFFER);
						memcpy(region1, recording, SOUND_BUFFER_SIZE);
						secondaryBuffer->Unlock(&region1, region1Size, &region2, region2Size);*/

						HRESULT dsResult;
						if ((dsResult = secondaryBuffer->SetCurrentPosition(0)) != DS_OK) {
							assert(false);
						}
						if ((dsResult = secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING)) != DS_OK) {
							assert(false);
						}

						/*while (true) {
							DWORD status;
							secondaryBuffer->GetStatus(&status);
							if (status != DSBSTATUS_PLAYING) {
								break;
							}
						}*/
					}
				}
				else {
					int x = 0;
					switch (dsResult) {
					case DSERR_BADFORMAT: {
						int x = 0;
					} break;
					case DSERR_INVALIDCALL: {
						int x = 0;
					} break;
					case DSERR_INVALIDPARAM: {
						int x = 0;
					} break;
					case DSERR_OUTOFMEMORY: {
						int x = 0;
					} break;
					case DSERR_PRIOLEVELNEEDED: {
						int x = 0;
					} break;
					case DSERR_UNSUPPORTED: {
						int x = 0;
					} break;
					default: {
						int x = 0;
					}
					}
				}
			}
		}
	}

	struct LockedBuffer {
		void *ptr1;
		DWORD size1;
		void *ptr2;
		DWORD size2;
	};

	char recvBuffer[SOUND_BUFFER_SIZE];
	int outputCursor = 0;

	if (dsCaptureInitialised && dsOutputInitialised) {
		WSADATA winsockData;
		WSAStartup(MAKEWORD(1, 1), &winsockData);

		addrinfo hints = {};
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		addrinfo *serverInfo;
		int socketHandle;
		if (getaddrinfo("localhost", PORT, &hints, &serverInfo) == 0) {
			socketHandle = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
			if (socketHandle != INVALID_SOCKET) {
				if (connect(socketHandle, serverInfo->ai_addr, serverInfo->ai_addrlen) != SOCKET_ERROR) {
					freeaddrinfo(serverInfo);

					while (true) {
#if 1
						static DWORD captureLastReadPosition = 0;
						DWORD captureCursor;
						DWORD readCursor;
						HRESULT captureGetCursorResult = dsCaptureBuffer->GetCurrentPosition(&captureCursor, &readCursor);
						if (captureGetCursorResult == DS_OK) {
							LockedBuffer lock;
							if (readCursor < captureLastReadPosition) {
								readCursor += SOUND_BUFFER_SIZE;
							}
							int amount = readCursor - captureLastReadPosition;
							// printf("amount %i \n", amount);
							if (amount > 0) {
								HRESULT captureLockResult = dsCaptureBuffer->Lock(captureLastReadPosition, amount,
														  						  &lock.ptr1, &lock.size1, &lock.ptr2, &lock.size2, 0);
								if (captureLockResult == DS_OK) {
									AudioPacketHeader *packet = (AudioPacketHeader*)pushStack(&transient, sizeof(AudioPacketHeader));
									memcpy(packet->id, "CASY", 4);
									packet->size = amount;
									char *packetData = pushStack(&transient, amount);
									memcpy(packetData, lock.ptr1, lock.size1);
									if (lock.ptr2) {
										memcpy(packetData + lock.size1, lock.ptr2, lock.size2);
									}
									int packetSize = sizeof(AudioPacketHeader) + amount;

									send(socketHandle, (char*)packet, packetSize, 0);
									popStack(&transient, packetSize);
									dsCaptureBuffer->Unlock(lock.ptr1, lock.size1, lock.ptr2, lock.size2);
									captureLastReadPosition += amount;
									if (captureLastReadPosition >= SOUND_BUFFER_SIZE) {
										captureLastReadPosition -= SOUND_BUFFER_SIZE;
									}

									int bytesRead = recv(socketHandle, recvBuffer, SOUND_BUFFER_SIZE, 0);
									AudioPacketHeader *serverPacket = (AudioPacketHeader*)&recvBuffer[0];
									void *serverPacketData = serverPacket + 1;
									if (bytesRead != sizeof(AudioPacketHeader) + serverPacket->size) {
										printf("partial packet %i/%i \n", bytesRead, sizeof(AudioPacketHeader) + serverPacket->size);
									} else {
										// printf("bytes received %i \n", bytesRead);

										DWORD playCursor;
										DWORD writeCursor;
										if (secondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor) == DS_OK) {
											// printf("write cursor %i \n", writeCursor);
											LockedBuffer writeLock = {};
											HRESULT result = secondaryBuffer->Lock(writeCursor, serverPacket->size, &writeLock.ptr1, &writeLock.size1, &writeLock.ptr2, &writeLock.size2, 0);
											if (result == DS_OK) {
												printf("write %i outputLast %i \n", writeCursor, outputCursor);
												memcpy(writeLock.ptr1, serverPacketData, writeLock.size1/*bytesRead*/);
												// printf("written to output %i \n", writeLock.size1);
												secondaryBuffer->Unlock(writeLock.ptr1, writeLock.size1, writeLock.ptr2, writeLock.size2);
												outputCursor = writeCursor + bytesRead;
											} else {
												assert(false);
											}
										} else {
											assert(false);
										}
									}
								} else {
									assert(false);
								}
							}
						} else {
							assert(false);
						}
#else
						send(socketHandle, "hey", 3, 0);
						char buffer[10] = {};
						int bytesRead = recv(socketHandle, buffer, 10, 0);
						printf("Recv %s \n", buffer);
#endif

						Sleep(50);
					}
				} else {
					int error = WSAGetLastError();
					int x = 0;
				}
			}
		}
	}

	exit(0);
}
