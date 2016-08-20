
#include <stdlib.h>

#define assert(exp) if (!exp) { *((int*)0) = 0; }
#define kilobytes(num) (num*1024)
#define megabytes(num) (kilobytes(num)*1024)
#define gigabytes(num) (megabytes(num)*1024)

#define PORT "3430"

#define SOUND_HTZ (44100 / 2)
#define SOUND_CHANNELS 2
#define SOUND_SAMPLE_BYTES 2
#define SOUND_BYTES_PER_SEC (SOUND_HTZ * SOUND_CHANNELS * SOUND_SAMPLE_BYTES)
#define SOUND_BUFFER_SIZE (SOUND_BYTES_PER_SEC * 4)

#ifdef _WIN32
#	define SOCKERR_INVALID INVALID_SOCKET
#	define SOCKERR_ERROR SOCKET_ERROR
#endif
#ifdef __linux__
#	define SOCKERR_INVALID -1
#	define SOCKERR_ERROR -1
#endif

struct AudioPacketHeader {
	char id[4];
	int size;
};

struct StackAllocator {
	char *mem;
	size_t size;
	size_t used;
};

StackAllocator createStackAllocator (size_t size) {
	StackAllocator memStack = {};

#ifdef _WIN32
	memStack.mem = (char*)VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
#endif
#ifdef __linux__
	memStack.mem = (char*)malloc(size);
#endif
	if (memStack.mem) {
		memStack.size = size;
	} else {
		assert(false);
	}

	return memStack;
}

void clearStack (StackAllocator *memStack) {
	memStack->used = 0;
}

char *pushStack (StackAllocator *memStack, size_t size, bool clear = false) {
	if (memStack->used + size <= memStack->size) {
		char *result = memStack->mem + memStack->used;
		memStack->used += size;
		if (clear) {
			memset(result, 0, size);
		}
		return result;
	} else {
		printf("Ran out of memory %lu/%lu \n", memStack->used + size, memStack->size);
		assert(false);
	}

	return NULL;
}

void popStack (StackAllocator *memStack, size_t size) {
	if (size <= memStack->used) {
		memStack->used -= size;
	} else {
		assert(false);
	}
}