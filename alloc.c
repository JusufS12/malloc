#include <unistd.h>
#include <pthread.h>

// implementing bool type with enum
typedef enum {false, true} bool;

// defining ALIGN type for aligning header to 16 bytes
typedef char ALIGN[16];

// header for allocated memory so it can be parsed and freed
union header {
	struct
	{
		size_t size;
		bool isFree;
		union header* next;
	} s;
	ALIGN stub;
};
typedef union header header_t;

// keeping track of head and tail of the heap
header_t *head, *tail;

// no idea what this is
pthread_mutex_t global_malloc_lock;

// declare so it can be called in malloc
header_t* get_free_block(size_t);


// function for allocating dynamic memory to the heap
void* malloc(size_t size) {

	void* block;
	header_t* header;

	if (!size)
		return NULL;

	// aquire lock (whatever that means)
	pthread_mutex_lock(&global_malloc_lock);

  // get a suitable block if it exists
	header = get_free_block(size);

	// if it found a block, unlock the thread and return the address
	if (header) {
		// also set it as occupied so id doesn't get parsed
		header->s.isFree = false;
		pthread_mutex_unlock(&global_malloc_lock);
		return (void*) header + 1; 
	}

  // if not then allocate the space on top of the heap,
	// and create a new block
	block = sbrk(sizeof(header_t) + size);

	//if it fails then return NULL
	if (block == (void*) -1) {
		pthread_mutex_unlock(&global_malloc_lock);
		return NULL;
	}


	// now handle the linked list

	header = block;
	header->s.isFree = false;
	header->s.next = NULL;

	// if there is no linked list set this as the first item
	if (!head)
		head = header;

	// if it has a tail then link the tail to the header
	if (tail)
		tail->s.next = header;

	// set the new item as the tail
	tail = header;

	pthread_mutex_unlock(&global_malloc_lock);

	// ofset the pointer for 1 byte to get the block,
	// and cast it as a void pointer so we can return it
	return (void*) (header + 1);
}

// gets sutable block if it already exists
header_t* get_free_block(size_t size) {

	header_t* curr = head;

	while(curr) {
		if (curr->s.isFree && curr->s.size >= size)
			return curr;
		curr = curr->s.next;
	}

	return NULL;
}

int main(void) {
	return 0;
}
