#include "virtual_alloc.h"
#include "virtual_sbrk.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size) {
    // Bad arguments
    if (heapstart == NULL)
        return;
    if (initial_size < min_size)
        return;

    // Program break should equal heapstart before initialisation
    int start_diff = heapstart - (void*)virtual_sbrk(0); 
    virtual_sbrk(start_diff);  

    // Store constant variables containing heap information
    int *min_size_ptr = virtual_sbrk(sizeof(int));
    *min_size_ptr = pow(2, min_size);
    
    int *num_of_b_ptr = virtual_sbrk(sizeof(int));
    *num_of_b_ptr = pow(2, (initial_size - min_size));

    // Store buddy allocator data structures
    for (int i = 0; i < *num_of_b_ptr; i++) {
        MEM_BLOCK *block = (MEM_BLOCK*)virtual_sbrk(sizeof(MEM_BLOCK));
        
        block->isLeft = true;
        block->isBuddy = false;
        block->isFree = true;  
        block->isEnd = false;
        if (i == *num_of_b_ptr - 1)
            block->isEnd = true;
    }

    // Make space for memory of size 2^initial_size
    for (int i = 0; i < *num_of_b_ptr; i++) {
        virtual_sbrk(*min_size_ptr);
    }

}

void * virtual_malloc(void * heapstart, uint32_t size) {
    //  Constant variables
    int min_size = *((int*)heapstart); // Minimum size of a block
    int num_of_b = *((int*)(heapstart + sizeof(int))); // Number of minimum size blocks required
    MEM_BLOCK *first_block = (MEM_BLOCK*)(heapstart + 2*sizeof(int)); // Pointer to first data structure 
    void *memory_start = (void*)(heapstart + 2 * sizeof(int) 
    + (num_of_b * sizeof(MEM_BLOCK))); // Pointer to start of memory and end of data structures

    // Bad arguments
    if (heapstart == NULL || size == 0)
        return NULL;
    if (size < 0 || size > (num_of_b * min_size))
        return NULL;

    // Variables for allocation
    MEM_BLOCK *ret_ptr = NULL; MEM_BLOCK *b_allocated = NULL;
    MEM_BLOCK *b_iterator = first_block;

    int b_to_alloc = 1;
    while (b_to_alloc * min_size < size)
        b_to_alloc *= 2;

    int b_free = 0;

    // Look for available buddies first
    int b_count = 0;
    while ((void*)b_iterator < memory_start) {
        if (b_iterator->isBuddy == false) {
            b_free = 0;
            b_count++; 
            b_iterator++;
            continue;
        }

        if ((b_iterator - 1)->isEnd)
            b_free = 0;
        if (b_iterator->isBuddy != (b_iterator - 1)->isBuddy)
            b_free = 0;

        if (b_iterator->isFree) 
            b_free++;
        else 
            b_free = 0;

        // Check if available space has been found
        b_count++; 
        if (b_free == b_to_alloc) { 
            int b_num = b_count - b_to_alloc;

            b_allocated = first_block + b_num;
            ret_ptr = memory_start + (b_num * min_size);
            break;
        }
        b_iterator++;
    }

    // If there are no available buddies, look for the left most available space
    if (b_allocated == NULL || ret_ptr == NULL) {
        b_count = 0;
        b_free = 0;

        b_iterator = first_block;
        while ((void*)b_iterator < memory_start) {
            if ((b_iterator - 1)->isEnd)
                b_free = 0;
            if (b_iterator->isBuddy != (b_iterator - 1)->isBuddy)
                b_free = 0;

            if (b_iterator->isFree) 
                b_free++;
            else 
                b_free = 0;

            // Check if available space has been found
            b_count++; 
            if (b_free == b_to_alloc) { 
                int b_num = b_count - b_to_alloc;

                b_allocated = first_block + b_num;
                ret_ptr = memory_start + (b_num * min_size);
                break;
            }
            b_iterator++;
        }
    }
    // Available space could not be found
    if (b_allocated == NULL || ret_ptr == NULL)
        return NULL;
    
    // Allocate memory
    b_iterator = b_allocated;
    for (int i = 0; i < b_to_alloc; i++) {
        b_iterator->isFree = false;
        b_iterator++;
    }
    (b_iterator - 1)->isEnd = true;

    // Determine if allocated block is left or right buddy
    bool b_is_left = true;
    b_iterator = b_allocated;
    for (int i = 0; i < b_to_alloc; i++) {
        if (b_iterator->isLeft == false)
            b_is_left = false;
        b_iterator++;
    }
    
    /* 
        Split blocks affected by allocation 
        Note that my implementation splits from smallest to largest
    */
    if (b_is_left) { 
        bool was_all_left = true;

        // Special case where size of allocated block equals min_size
        if (b_to_alloc == 1) {
            if (b_iterator->isLeft == false)
                was_all_left = false;

            b_iterator->isLeft = false;
            b_iterator->isBuddy = true;
            b_iterator++;
        }

        // Split direct block of allocated
        for (int i = 0; i < b_to_alloc / 2; i++) {
            b_iterator->isLeft = true;
            b_iterator->isBuddy = true;
            b_iterator++;
        }      
        for (int i = 0; i < b_to_alloc / 2; i++) {
            if (b_iterator->isLeft == false)
                was_all_left = false;

            b_iterator->isLeft = false;
            b_iterator->isBuddy = true;
            b_iterator++;
        }
        (b_iterator - 1)->isEnd = true;

        // Split all blocks affected by allocation
        while ((void*)b_iterator < memory_start) {
            if (b_allocated->isBuddy) {
                if (b_iterator->isBuddy == false)
                    break;
            } else {
                if (!(was_all_left))
                    break;
            }

            b_to_alloc *= 2;
            for (int i = 0; i < b_to_alloc/2; i++) {
                b_iterator->isLeft = true;
                b_iterator++;
            }
            for (int i = 0; i < b_to_alloc/2; i++) {
                if (b_iterator->isLeft == false)
                    was_all_left = false;
                b_iterator->isLeft = false;
                b_iterator++;
            }
            (b_iterator - 1)->isEnd = true;
        }
    }
    return ret_ptr;
}

MEM_BLOCK * merge_blocks(MEM_BLOCK * b_curr, void * first_block, void * memory_start);
int virtual_free(void * heapstart, void * ptr) {
    // Constant variables
    int min_size = *((int*)heapstart); // Minimum size of a block
    int num_of_b = *((int*)(heapstart + sizeof(int))); // Number of minimum size blocks required
    MEM_BLOCK *first_block = (MEM_BLOCK*)(heapstart + 2*sizeof(int)); // Pointer to first data structure 
    void *memory_start = (void*)(heapstart + 2 * sizeof(int) 
    + (num_of_b * sizeof(MEM_BLOCK))); // Pointer to start of memory and end of data structures

    // Bad arguments
    if (heapstart == NULL || ptr == NULL) 
        return 1;
    if (ptr < memory_start || ptr > virtual_sbrk(0))
        return 1;

    // Locate ptr
    MEM_BLOCK *b_iterator = first_block; MEM_BLOCK *b_freed = NULL;
    int b_count = 0;
    while ((void*)b_iterator < memory_start) {
        void* b_memoryAddress = memory_start + (b_count * min_size);
        if (b_memoryAddress == ptr && b_iterator->isFree == false) {

            if (b_iterator != first_block && (b_iterator - 1)->isEnd == false)
                return 1;

            b_freed = b_iterator;
            while ((void*)b_iterator < memory_start) {
                b_iterator->isFree = true;

                if (b_iterator->isEnd)
                    break;
                b_iterator++;
            }

        }
        b_count++;
        b_iterator++;
    }
    // ptr is unallocated memory
    if (b_freed == NULL) 
        return 1;

    // Merge freed block with direct buddy
    b_iterator = b_freed;
    while ((void*)b_iterator < memory_start)
        b_iterator = merge_blocks(b_iterator, first_block, memory_start);

    // Merge all blocks affected by free
    b_iterator = first_block;
    while ((void*)b_iterator < memory_start)
        b_iterator = merge_blocks(b_iterator, first_block, memory_start);

    return 0;
}

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size) {
    // Constant variables
    int min_size = *((int*)heapstart); // Minimum size of a block
    int num_of_b = *((int*)(heapstart + sizeof(int))); // Number of minimum size blocks required
    MEM_BLOCK *first_block = (MEM_BLOCK*)(heapstart + 2*sizeof(int)); // Pointer to first data structure 
    void *memory_start = (void*)(heapstart + 2 * sizeof(int) 
    + (num_of_b * sizeof(MEM_BLOCK))); // Pointer to start of memory and end of data structures

    // Special cases
    if (ptr == NULL) {
        return virtual_malloc(heapstart, size);
    }
    if (size == 0) {
        virtual_free(heapstart, ptr);
        return NULL;
    }
    
    // Bad arguments
    if (heapstart == NULL)
        return NULL;
    if (ptr < memory_start || ptr > virtual_sbrk(0))
        return NULL;
    if (size < 0 || size > (num_of_b * min_size))
        return NULL;

    // Locate ptr
    MEM_BLOCK *b_iterator = first_block; MEM_BLOCK *b_freed;
    int b_count = 0;
    while ((void*)b_iterator < memory_start) {
        void* b_memoryAddress = memory_start + (b_count * min_size);
        if (b_memoryAddress == ptr && b_iterator->isFree == false) {
            b_freed = b_iterator;
            break;
        }
        b_count++;
        b_iterator++;
    }
    // ptr is unallocated memory
    if (b_freed == NULL) 
        return NULL;

    // Variables used for reallocation
    int realloc_s = 1; 
    while ((realloc_s * min_size) < size)
        realloc_s *= 2;

    int current_s = 1;
    b_iterator = b_freed;
    while (b_iterator->isEnd == false) {
        current_s++;
        b_iterator++;
    }

    // Decrease size
    if (realloc_s <= current_s) {
        int buddy_end_count = current_s / 2;
        for (int i = 0; i < current_s - realloc_s; i++) {
            b_iterator->isFree = true;

            if (i == buddy_end_count) {
                b_iterator->isEnd = true;
                buddy_end_count += buddy_end_count / 2;
            }
            b_iterator--;
        }
        b_iterator->isEnd = true;
        return ptr;
    }

    // Increase size or reallocate
    if (realloc_s > current_s) {
            virtual_free(heapstart, ptr);
            void* new_ptr = virtual_malloc(heapstart, size);
            if (new_ptr != NULL)
                memmove(new_ptr, ptr, size);

            return new_ptr;
    }
    return NULL;
}

void virtual_info(void * heapstart) {
    if (heapstart == NULL)
        return;

    // Constant variables
    int min_size = *((int*)heapstart); // Minimum size of a block
    int num_of_b = *((int*)(heapstart + sizeof(int))); // Number of minimum size blocks required
    MEM_BLOCK *first_block = (MEM_BLOCK*)(heapstart + 2*sizeof(int)); // Pointer to first data structure 
    void *memory_start = (void*)(heapstart + 2 * sizeof(int) 
    + (num_of_b * sizeof(MEM_BLOCK))); // Pointer to start of memory and end of data structures

    // Print information
    int b_size = 0;
    MEM_BLOCK *b_iterator = first_block;
    while ((void*)b_iterator < memory_start)  {
        b_size += min_size;
        
        if (b_iterator->isEnd) {
            if (b_iterator->isFree)
                printf("free %d\n", b_size);
            if (b_iterator->isFree == false)
                printf("allocated %d\n", b_size);
            b_size = 0;
        }
        b_iterator++;
    }  
}

// My function - Merges two blocks if they are buddies and both unallocated
MEM_BLOCK * merge_blocks(MEM_BLOCK * b_curr, void * first_block, void * memory_start) {
    MEM_BLOCK* b_iterator = b_curr;

    // Get block size
    bool can_skip = false;
    int b_curr_num = 1;
    while ((void*)b_iterator < memory_start) {
        if (b_iterator->isFree == false  && b_iterator->isLeft)
            can_skip = true;
        if (b_iterator->isEnd)
            break;

        b_curr_num++;
        b_iterator++;
    }
    // Special cases
    if (can_skip)
        return (b_iterator + b_curr_num + 1);
    if ((b_curr_num & (b_curr_num - 1)) != 0)
        return (MEM_BLOCK*)memory_start;

    // Check if block is left or right buddy
    b_iterator = b_curr;
    bool b_is_left = true;
    if (b_curr_num == 1) {
        if (b_iterator->isLeft == false)
            b_is_left = false;
        b_iterator++;
    }
    for (int i = 0; i < b_curr_num / 2; i++) {
        if (b_iterator->isLeft == false)
            b_is_left = false;
        b_iterator++;
    }
    for (int i = 0; i < b_curr_num / 2; i++)
        b_iterator++;

    if (b_is_left && b_curr->isBuddy) {
        if (b_iterator->isBuddy == false) 
            b_is_left = false;
    } 

    // Determine if blocks are mergeable
    MEM_BLOCK *b_to_merge;
    if (b_is_left) {
        if ((void*)b_iterator == memory_start)
            return (MEM_BLOCK*)memory_start;

        b_to_merge = b_curr; 
        for (int i = 0; i < b_curr_num; i++) {
            if (b_iterator->isFree == false) 
                return (MEM_BLOCK*)memory_start;

            b_iterator++;
        }
    }
    if (b_is_left == false) {
        b_iterator -= (b_curr_num + 1);
        for (int i = 0; i < b_curr_num; i++) {
            if (b_iterator->isFree == false) {
                return (MEM_BLOCK*)memory_start;
            }

            b_iterator--;
        }
        b_to_merge = b_iterator + 1;
    }

    if (b_is_left && b_curr->isBuddy == false) {
        if (b_curr != first_block)
            return (MEM_BLOCK*)memory_start;
    }

    // Merge blocks
    bool not_buddy_b = false;
    b_iterator = b_to_merge;

    for (int i = 0; i < b_curr_num; i++) {
        if (b_iterator->isBuddy == false)
            not_buddy_b = true;

        b_iterator->isLeft = true;
        b_iterator->isEnd = false;
        b_iterator++;
    }
    for (int i = 0; i < b_curr_num; i++) {
        if (not_buddy_b) 
            b_iterator->isBuddy = false;

        b_iterator->isLeft = false;
        b_iterator->isEnd = false;
        b_iterator++;
    }
    (b_iterator - 1)->isEnd = true;
    
    return b_to_merge;
}
