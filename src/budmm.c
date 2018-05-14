/*
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "budmm.h"

#define DEBUG 1

int isBuddy(bud_free_block*, bud_free_block*, unsigned long);
void debugFree();
void debugMalloc();
int isValidPtr(void*);

/*
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in budmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
extern bud_free_block free_list_heads[NUM_FREE_LIST];

/*
 * This is your implementation of bud_malloc. It acquires uninitialized memory that
 * is aligned and padded properly for the underlying system.
 *
 * @param size The number of bytes requested to be allocated.
 *
 * @return If successful, a pointer to a valid region of memory of the
 * requested size is returned, 
 *			else NULL is returned and errno as follows:
 *
 * If the size passed is invalid, errno is set to EINVAL.
 * If the request cannot be satisfied, errno is set to ENOMEM.
 */
void *bud_malloc(uint32_t rsize) {
	if(rsize <= 0 || rsize > (MAX_BLOCK_SIZE - sizeof(bud_header))){
		errno = EINVAL;
		return NULL;
	}
	unsigned long actualSize = rsize + sizeof(bud_header);
	int order = 0;
	unsigned long offset;
	unsigned long temp = 1UL;
	while(temp < actualSize){
		order++;
		temp*=2;
	}
	if(order < ORDER_MIN)
		order = ORDER_MIN;
	int continueLoop = 1;
	struct bud_free_block* ptr;
	int i = (order - ORDER_MIN);
	for(; i < NUM_FREE_LIST && continueLoop; i++){
		ptr = &free_list_heads[i];
		ptr = (ptr->next);
		while((ptr != &free_list_heads[i]) && (ptr->header.allocated))	ptr = (ptr -> next);
		if(!ptr->header.allocated && ptr != &free_list_heads[i])	continueLoop = 0;
	}
	struct bud_free_block* fSptr = ptr;
	if(continueLoop){
		fSptr = ((bud_free_block*)(bud_sbrk()));
		if(errno == ENOMEM) return NULL;
		offset = MAX_BLOCK_SIZE;
		bud_free_block* fSStart = fSptr;
		bud_free_block* fSEnd =  bud_heap_end();
	}
	else{
		offset = 1 << (fSptr->header.order);
	}
	fSptr = (bud_free_block*)(((void*)fSptr) + offset);
	int j = i-2;
	for(offset /= 2 ; offset >= (rsize + sizeof(bud_header)) && j >= 0; offset /= 2, j--){
		fSptr = (bud_free_block*)(((void*)fSptr) - offset);
		bud_header newSpace = {0x0, (j + ORDER_MIN) , 0x0, 0x0, 0x0, 0x0};
		bud_free_block newNode = {newSpace, (free_list_heads[j].next), &free_list_heads[j]};
		memcpy(fSptr , &newNode, sizeof(bud_free_block));
		((free_list_heads[j].next)->prev) = fSptr;
		(free_list_heads[j].next) = fSptr;
	}
	if(j == -1)	j = 0;
	fSptr = (bud_free_block*)(((void*)fSptr) - 2*offset);
	if(!continueLoop){
		(fSptr->next->prev) = fSptr->prev;
		(fSptr->prev->next) = fSptr->next;
	}
	bud_header allocatedSpace = {0x1, order, (rsize + sizeof(bud_header) != (1 << order)), 0x0, rsize, 0x0};
	memcpy(fSptr, &allocatedSpace, sizeof(bud_header));
	return (((void*)fSptr) + sizeof(bud_header));
}


/*
 * Resizes the memory pointed to by ptr to size bytes.
 *
 * @param ptr Address of the memory region to resize.
 * @param size The minimum size to resize the memory to.
 *
 * @return If successful, a pointer to a valid region of memory is
 * returned, else NULL is returned and errno is set appropriately.
 *
 * A call to bud_realloc with a ptr == NULL should be equivalent to a
 * call to bud_malloc(size).
 * A call to bud_realloc with size == 0 should be equivalent to a
 * call to bud_free(ptr).
 * If bud_realloc is called with an invalid ptr it should call abort()
 * to exit the program.
 * If there is no memory available bud_realloc should set errno to ENOMEM.
 */
void *bud_realloc(void *ptr, uint32_t rsize) {
	if(ptr == NULL){
		return bud_malloc(rsize);
	}
	if(rsize == 0){
		bud_free(ptr);
		return NULL;
	}
	bud_header* headerPtr = (bud_header*) (ptr-sizeof(bud_header));

	ptr = (((void*)ptr) - sizeof(bud_header));
	int order = isValidPtr(ptr);
	if(!order && order != headerPtr->order)
		abort();
	ptr = (((void*)ptr) + sizeof(bud_header));

	if((1 << (order-1)) <= rsize+sizeof(bud_header) && rsize+sizeof(bud_header) < (1 << (order)))
		return ptr;
	if((rsize+sizeof(bud_header)) > (1 << (order))){
		unsigned long sizeOfPayload = headerPtr->rsize;
		void* newPtr = bud_malloc(rsize);
		memcpy(newPtr, ptr, sizeOfPayload);
		bud_free(ptr);
		return newPtr;
	}

	struct bud_free_block* fSptr = ((void*)headerPtr) + (1<<(order));
	struct bud_free_block* rootPtr = (bud_free_block*) headerPtr;
	unsigned long offset =   (1<<(order-1));
	int j = order - ORDER_MIN;
	unsigned long smallestPossibleSize = rsize + sizeof(bud_header);
	if(smallestPossibleSize < (1 << ORDER_MIN)){
		smallestPossibleSize = (1 << ORDER_MIN);
	}
	
	order--;
	j--;
	for( ;smallestPossibleSize <= offset && j >= 0;  j--, order--, offset = (1<<(order))){
		fSptr = (bud_free_block*)(((void*)fSptr) - offset);
		bud_header newSpace = {0x0, order , 0x0, 0x0, 0x0, 0x0};
		bud_free_block newNode = {newSpace, (free_list_heads[j].next), &free_list_heads[j]};
		memcpy(fSptr , &newNode, sizeof(bud_free_block));
		((free_list_heads[j].next)->prev) = fSptr;
		(free_list_heads[j].next) = fSptr;
		
	}
	headerPtr->order = order+1;
	headerPtr->allocated = 1;
	headerPtr->padded = (rsize + sizeof(bud_header) != (1 << (order+1)));
	headerPtr->rsize = rsize;
    return ptr;
}

/*
 * Marks a dynamically allocated region as no longer in use.
 * Adds the newly freed block to the free list.
 *
 * @param ptr Address of memory returned by the function bud_malloc.
 *
 * If bud_free is called with an invalid ptr, it should call abort()
 * to exit the program.
 */
void bud_free(void *ptr) {
	//Remove the offset
	ptr = (((void*)ptr) - sizeof(bud_header));

	int order = isValidPtr(ptr);
	if(!order && order != ((bud_header*)ptr)->order){
		abort();
	}

	struct bud_free_block* tempPtr;
	struct bud_free_block* ptr1;
	struct bud_free_block* ptr2;
	struct bud_free_block* insertPtr = (bud_free_block*)ptr;
	int firstTime = 1;
	int coalesce = 1;
	for(int i = 0; i < NUM_FREE_LIST; i++){
		//insert
		if(coalesce){
			if(firstTime){
				insertPtr->header.order = order;
				firstTime = 0;
			}
			else{
				order = i+ORDER_MIN;
				insertPtr->header.order = order;
			}
			insertPtr->header.allocated = 0;
			int index = order - ORDER_MIN;
			insertPtr->next = free_list_heads[index].next;
			insertPtr->prev = &free_list_heads[index];
			((free_list_heads[index].next)->prev) = insertPtr;
			(free_list_heads[index].next) = insertPtr;
			coalesce = 0;
		}
		int len = 0;
		tempPtr = (&free_list_heads[i])->next;
		while(tempPtr != (&free_list_heads[i]) && ++len) tempPtr = tempPtr -> next;
		if(len >= 2 && i != NUM_FREE_LIST-1){
			ptr1 = (&free_list_heads[i]);
			ptr2 = (&free_list_heads[i]);
			int tempOrder = i + ORDER_MIN;
			unsigned long tempSize = 1 << tempOrder;
			coalesce = isBuddy(ptr1, ptr2, tempSize);
			for(int k = 0; !coalesce && k < len ; k++){
				ptr1 = ptr1->next;
				ptr2 = (&free_list_heads[i]); //reset ptr2
				for(int j = 0; !coalesce && j < len; j++){
					ptr2 = ptr2->next;
					if(ptr1!=ptr2){
						coalesce = isBuddy(ptr1, ptr2, tempSize);
					}
				}
			}//Removing both blocks if needed to coalesce
			if(coalesce){
				((bud_free_block*)ptr1)->next->prev = ((bud_free_block*)ptr1)->prev;
				((bud_free_block*)ptr1)->prev->next = ((bud_free_block*)ptr1)->next;
				((bud_free_block*)ptr2)->next->prev = ((bud_free_block*)ptr2)->prev;
				((bud_free_block*)ptr2)->prev->next = ((bud_free_block*)ptr2)->next;
				insertPtr = (bud_free_block*) ((ptr1 < ptr2) ? (ptr1) : (ptr2));
				coalesce = 1;
			}
		}
	}
    return;
}


int isBuddy(bud_free_block* ptr1, bud_free_block* ptr2, unsigned long size){
	return ((((unsigned long)ptr1)^size) == ((unsigned long)ptr2)) && ((((unsigned long)ptr2)^size) == ((unsigned long)ptr1));
}

int isValidPtr(void* ptr){
	void* heapStart = bud_heap_start();
	void* heapEnd = bud_heap_end();
	bud_header* header = ((bud_header*)ptr);
	int order = header->order;
	int allocated = header->allocated;
	int padded = header->padded;
	unsigned long rsize = header->rsize;
	unsigned long maxSize = 1 << order;
	unsigned long minSize = 1 << (order-1);
	if(!(heapStart <= ptr && ptr <= heapEnd))
		return 0;
	if((unsigned long)ptr % 0x8 != 0)
		return 0; 
	if(!(ORDER_MIN <= order && order <= ORDER_MAX))
		return 0;
	if(order != ORDER_MIN && rsize+sizeof(bud_header) <=  1 << ORDER_MIN)
		return 0;
	if(padded != (rsize + sizeof(bud_header) < (maxSize)))
		return 0;
	if(!(rsize + sizeof(bud_header) <= (maxSize)))
		return 0;
	if(!allocated)
		return 0;
	return order;
}

void debugFree(){
	if(DEBUG){
		printf("%s: ", "Free");
		bud_free_block* ptr3;
		for(int i = 0; i < NUM_FREE_LIST; i++){
			ptr3 = (&free_list_heads[i])->next;
			int num = 0;
			if((ptr3 != &free_list_heads[i])){
				//printf("Size: %d\n", 1 << ptr3->header.order);
			}
			
			while((ptr3 != &free_list_heads[i])){
				//printf("bud_free_block address at %d: %p\n", i, ptr3);
				num++;
				ptr3 = (ptr3 -> next);
			}
			printf("%d ", num);

		}
		printf("\n");
	}
}

void debugMalloc(){
	if(DEBUG){
		bud_free_block* ptr;	
		printf("%s: ", "Malloc");
		for(int k = 0; k < NUM_FREE_LIST; k++){
			ptr = &free_list_heads[k];
			ptr = (ptr->next);
			int num = 0;
			while((ptr != &free_list_heads[k]))	{
				num++;
				ptr = ptr -> next;
			}
			printf("%d ", num);
		}
		printf("%s\n", "");
	}
}








