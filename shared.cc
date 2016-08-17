
#define assert(exp) if (!exp) { *((int*)0) = 0; }
#define kilobytes(num) (num*1024)
#define megabytes(num) (kilobytes(num)*1024)
#define gigabytes(num) (megabytes(num)*1024)

struct StackAllocator {
	char *mem;
	size_t size;
	size_t used;
};

StackAllocator createStackAllocator (size_t size) {
	StackAllocator memStack = {};

	memStack.mem = (char*)VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
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