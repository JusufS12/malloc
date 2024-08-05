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
		return (void*) header++; 
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

	// ofset the pointer for 1 byte to get the block (by incrementing it),
	// and cast it as a void pointer so we can return it
	return (void*) (header++);
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


// function for freeing memory blocks
void free(void* block) {

	header_t *header, *iterator;
	void* program_break;

	// check if the block exists
	if (!block)
		return;
	
	// aquiaring the global lock (or something)
	pthread_mutex_lock(&global_malloc_lock);
	// getting the header of the block by offsetting the block pointer
	// (this time by decrementing it)
	header = (header_t*) block--;


	// check if the block is the last block on the heap,
	// by offsetting the block pointer by the size of the block
	// (the char* cast is used because the size variable of the header is in bytes)
	// and comparing it to the current address of the program_break
	if ((char*) block + header->s.size == program_break) {

		// if there is no other blocks, unset both the head and tail
		if (head == tail)
			head = tail = NULL;
		
		// if there are still blocks before this one,
		// search the linked list from the begining
		// and find the block before the tail (this block that is being freed)
		// and remove the link to the block being freed and set it as the new tail
		else {

			iterator = head;

			// iterate trough the list until the block before the tail is found
			while(iterator) {

				// when found remove the link and set it as the new tail
				if (iterator->s.next == tail) {
					iterator->s.next == NULL;
					tail == iterator;
				}

				tail = tail->s.next;
			}

		}

		// give the memory back to the OS by decrementing the program break

		// decrement the program break by the combined size of both header and the block
		// by subtracting it from 0 (because it decrements by passing in a negative number)
		sbrk(0 - sizeof(header) - header->s.size);
		pthread_mutex_unlock(&global_malloc_lock);
	}

	// marking the block as free
	header->s.isFree = true;
	pthread_mutex_unlock(&global_malloc_lock);
}


int main(void) {
	return 0;
}
