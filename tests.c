#include "virtual_alloc.h"
#include "virtual_sbrk.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAX_MEMORY_CAPACITY 10000000 // Max block is 2^23

unsigned char global_mem[MAX_MEMORY_CAPACITY];
void *my_heapstart = &global_mem;
void *p_break = (void*)global_mem + (MAX_MEMORY_CAPACITY-1);

void *virtual_sbrk(int32_t increment) {
    void *original = p_break;
    p_break = (void*) ((char*)p_break + increment);
    return original;
}

// Testcases
void init() {
    init_allocator(my_heapstart, 15, 11);

    virtual_info(my_heapstart);
}

void malloc1() {
    init_allocator(my_heapstart, 15, 11);

    virtual_malloc(my_heapstart, 4000);

    virtual_info(my_heapstart);
}

void malloc2() {
    init_allocator(my_heapstart, 15, 11);
    
    virtual_malloc(my_heapstart, 2000);
    virtual_info(my_heapstart);
    
    virtual_malloc(my_heapstart, 2000);
    virtual_malloc(my_heapstart, 2000);

    virtual_info(my_heapstart);
}

void malloc3() {
    init_allocator(my_heapstart, 15, 11);

    virtual_malloc(my_heapstart, 2000);
    virtual_malloc(my_heapstart, 8000);
    virtual_malloc(my_heapstart, 2000);
    virtual_malloc(my_heapstart, 4000);
    
    virtual_info(my_heapstart);
}

void malloc4() {
    init_allocator(my_heapstart, 15, 11);

    virtual_malloc(my_heapstart, 16000);
    virtual_malloc(my_heapstart, 2000);
    virtual_malloc(my_heapstart, 4000);
    virtual_malloc(my_heapstart, 8000);
    
    virtual_info(my_heapstart);
}

void malloc5() {
    init_allocator(my_heapstart, 15, 11);

    for (int i = 0; i < 16; i++) {
        virtual_malloc(my_heapstart, 2000);
            //virtual_info(my_heapstart);
    //
    }
    virtual_info(my_heapstart);
}

void free1() {
    init_allocator(my_heapstart, 15, 11);

    void *ptr1 = virtual_malloc(my_heapstart, 16000);
    virtual_info(my_heapstart);

    virtual_free(my_heapstart, ptr1);
    virtual_info(my_heapstart);
}

void free2() {
    init_allocator(my_heapstart, 15, 11);

    void *ptr1 = virtual_malloc(my_heapstart, 8000);
    void *ptr2 = virtual_malloc(my_heapstart, 2000);
    virtual_info(my_heapstart);

    virtual_free(my_heapstart, ptr1);
    virtual_info(my_heapstart);

    virtual_free(my_heapstart, ptr2);
    virtual_info(my_heapstart);
}

void free3() {
    init_allocator(my_heapstart, 15, 11);

    void *ptr1 = virtual_malloc(my_heapstart, 8000);
    void *ptr2 = virtual_malloc(my_heapstart, 4000);
    virtual_info(my_heapstart);

    virtual_free(my_heapstart, ptr1);
    void *ptr3 = virtual_malloc(my_heapstart, 4000);
    virtual_info(my_heapstart);


    virtual_free(my_heapstart, ptr2);
    virtual_free(my_heapstart, ptr3);
    virtual_info(my_heapstart);
}

void free4() {
    init_allocator(my_heapstart, 15, 11);

    void *ptr1 = virtual_malloc(my_heapstart, 8000);
    void *ptr2 = virtual_malloc(my_heapstart, 4000);
    void *ptr3 = virtual_malloc(my_heapstart, 4000);
    virtual_free(my_heapstart, ptr1);
    virtual_free(my_heapstart, ptr2);
    virtual_info(my_heapstart);

    void *ptr4 = virtual_malloc(my_heapstart, 4000);
    virtual_info(my_heapstart);

    virtual_free(my_heapstart, ptr3);
    virtual_free(my_heapstart, ptr4);
    virtual_info(my_heapstart);
}

void free5() {
    init_allocator(my_heapstart, 15, 11);

    void *ptr1 = virtual_malloc(my_heapstart, 2000);
    for (int i = 0; i < 15; i++)
        virtual_malloc(my_heapstart, 2000);
    
    for (int i = 0; i < 16; i++) {
        virtual_free(my_heapstart, ptr1);
        ptr1 += 2048;
    }

    virtual_info(my_heapstart);
}

void realloc1() {
    init_allocator(my_heapstart, 15, 11);

    void *ptr1 = virtual_malloc(my_heapstart, 8000);
    virtual_info(my_heapstart);

    virtual_realloc(my_heapstart, ptr1, 4000);
    virtual_info(my_heapstart);
}

void realloc2() {
    init_allocator(my_heapstart, 15, 11);

    void *ptr1 = virtual_malloc(my_heapstart, 4000);
    virtual_info(my_heapstart);

    virtual_realloc(my_heapstart, ptr1, 8000);
    virtual_info(my_heapstart);
}

void realloc3() {
    init_allocator(my_heapstart, 15, 11);

    virtual_malloc(my_heapstart, 8000);
    void *ptr1 = virtual_malloc(my_heapstart, 4000);
    virtual_info(my_heapstart);

    virtual_realloc(my_heapstart, ptr1, 2000);
    virtual_info(my_heapstart);
}

void realloc4() {
    init_allocator(my_heapstart, 15, 11);

    virtual_malloc(my_heapstart, 8000);
    void *ptr1 = virtual_malloc(my_heapstart, 4000);
    virtual_info(my_heapstart);
    
    virtual_realloc(my_heapstart, ptr1, 8000);
    virtual_info(my_heapstart);
}

void realloc5() {
    void *ptr1 = virtual_malloc(my_heapstart, 4000);
    for (int i = 0; i < 15; i++)
        virtual_malloc(my_heapstart, 4000);
    virtual_info(my_heapstart);

    for (int i = 0; i < 16; i++) {
        virtual_realloc(my_heapstart, ptr1, 2000);
        ptr1 += 4096;
    }
    virtual_info(my_heapstart);
}

void realloc6() {
    init_allocator(my_heapstart, 15, 11);

    void *ptr1 = virtual_malloc(my_heapstart, 4000);
    virtual_info(my_heapstart);

    virtual_realloc(my_heapstart, ptr1, 32000);
    virtual_info(my_heapstart);
}

void malloc_size0() {
    init_allocator(my_heapstart, 15, 11);

    if (virtual_malloc(my_heapstart, 0) == NULL)
        printf("success\n");
    else
        printf("fail\n");
}

void malloc_fail() {
    init_allocator(my_heapstart, 15, 11);

    if (virtual_malloc(my_heapstart, 32769) == NULL)
        printf("success\n");
    else
        printf("fail\n");
}

void free_ptr_null() {
    init_allocator(my_heapstart, 15, 11);

    if (virtual_free(my_heapstart, NULL) == 1)
        printf("success\n");
    else
        printf("fail\n");
}

void free_fail() {
    init_allocator(my_heapstart, 15, 11);
    void *ptr1 = virtual_malloc(my_heapstart, 4000);

    if (virtual_free(my_heapstart, ptr1 + 2048) == 1)
        printf("success\n");
    else
        printf("fail\n");
}

void realloc_size0() {
    init_allocator(my_heapstart, 15, 11);

    void *ptr1 = virtual_malloc(my_heapstart, 4096);
    virtual_info(my_heapstart);
    virtual_realloc(my_heapstart, ptr1, 0);

    virtual_info(my_heapstart);
}

void realloc_ptr_null() {
    init_allocator(my_heapstart, 15, 11);

    virtual_realloc(my_heapstart, NULL, 4000);

    virtual_info(my_heapstart);
}

void realloc_fail() {
    init_allocator(my_heapstart, 15, 11);

    void *ptr1 = virtual_malloc(my_heapstart, 4096);
    if (virtual_realloc(my_heapstart, ptr1, 32769) == NULL)
        printf("success\n");
    else
        printf("fail\n");
}

// Function that pipes test output to file
void pipe_to_file(void func(), char* fname) {
    char out_file[100] = "tests_actual/";
    strcat(out_file, fname);
    FILE *fp = freopen(out_file, "w", stdout);
    func();
    fclose(fp);
}

int main(int argc, char** argv) {
    // init
    pipe_to_file(init, "init.act");

    // malloc
    pipe_to_file(malloc1, "malloc1.act");
    pipe_to_file(malloc2, "malloc2.act");
    pipe_to_file(malloc3, "malloc3.act");
    pipe_to_file(malloc4, "malloc4.act");
    pipe_to_file(malloc5, "malloc5.act");

    // free
    pipe_to_file(free1, "free1.act");
    pipe_to_file(free2, "free2.act");
    pipe_to_file(free3, "free3.act");
    pipe_to_file(free4, "free4.act");
    pipe_to_file(free5, "free5.act");

    // realloc
    pipe_to_file(realloc1, "realloc1.act");
    pipe_to_file(realloc2, "realloc2.act");
    pipe_to_file(realloc3, "realloc3.act");
    pipe_to_file(realloc4, "realloc4.act");
    pipe_to_file(realloc5, "realloc5.act");
    pipe_to_file(realloc6, "realloc6.act");
    pipe_to_file(realloc_size0, "realloc_size0.act");
    pipe_to_file(realloc_ptr_null, "realloc_ptr_null.act");

    // Negative test cases
    pipe_to_file(malloc_size0, "malloc_size0.act");
    pipe_to_file(malloc_fail, "malloc_fail.act");

    pipe_to_file(free_ptr_null, "free_ptr_null.act");
    pipe_to_file(free_fail, "free_fail.act");

    pipe_to_file(realloc_fail, "realloc_fail.act");

    return 0;
}



