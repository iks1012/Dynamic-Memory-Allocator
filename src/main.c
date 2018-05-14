#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>


#include "budmm.h"

//sizeof(int) = 4 <------- For future reference




int main(int argc, char const *argv[]) {
    bud_mem_init();

    void *x = bud_malloc(sizeof(double) * 4); // 32 -> 64
    void *y = bud_realloc(x, sizeof(int));

/*    										 //  5   6   7   8   9  10  11  12  13  14
    void *a = bud_malloc(sizeof(long));      //  1   1   1   1   1   1   1   1   1   0
    void *w = bud_malloc(sizeof(int) * 100); //  1   1   1   1   0   1   1   1   1   0
    void *x = bud_malloc(sizeof(char));      //  0   1   1   1   0   1   1   1   1   0
    void *b = bud_malloc(sizeof(int));       //  1   0   1   1   0   1   1   1   1   0
    void *y = bud_malloc(sizeof(int) * 100); //  1   0   1   1   1   0   1   1   1   0
    void *z = bud_malloc(sizeof(char));      //  0   0   1   1   1   0   1   1   1   0
    void *c = bud_malloc(sizeof(int));       //  1   1   0   1   1   0   1   1   1   0 
    void *d = bud_malloc(sizeof(int));       //  0   1   0   1   1   0   1   1   1   0
    //14 9 5 6 10 5 7 5

    printf("\n");

	printf("\n");

    bud_free(c);                             //  1   1   0   1   1   0   1   1   1   0
    bud_free(z);                             //  2   1   0   1   1   0   1   1   1   0
    bud_free(y);                             //  2   1   0   1   0   1   1   1   1   0 
    bud_free(a);                             //  3   1   0   1   0   1   1   1   1   0 
    bud_free(b);                             //  2   2   0   1   0   1   1   1   1   0 
    bud_free(x);                             //  1   1   1   1   0   1   1   1   1   0



	char* name = (char*) bud_malloc(10);
	name[0] = 'I';
	name[1] = 's';
	name[2] = 'h';
	name[3] = 'a';
	name[4] = 'n';
	name[5] = '\0';
	printf("name: %s\n", name);

    

    char* name2 = (char*) bud_malloc(10);
    name2[0] = 'I';
    name2[1] = 's';
    name2[2] = 'h';
    name2[3] = 'a';
    name2[4] = 'n';
    name2[5] = '2';
    name2[6] = '\0';
    printf("name2: %s\n", name2);
    printf("name: %s\n", name);

    bud_free(name);
    bud_free(name2);

    char* name3 = (char*) bud_malloc(10);
    name3[0] = 'I';
    name3[1] = 's';
    name3[2] = 'h';
    name3[3] = 'a';
    name3[4] = 'n';
    name3[5] = '3';
    name3[6] = '\0';
    printf("name3: %s\n", name3);
    printf("name2: %s\n", name2);
    printf("name: %s\n", name);

    char* name4 = (char*) bud_malloc(10);
    name4[0] = 'I';
    name4[1] = 's';
    name4[2] = 'h';
    name4[3] = 'a';
    name4[4] = 'n';
    name4[5] = '4';
    name4[6] = '\0';
    printf("name4: %s\n", name4);
    printf("name3: %s\n", name3);
    printf("name2: %s\n", name2);
    printf("name: %s\n", name);

    char* name5 = (char*) bud_malloc(10);
    name5[0] = 'I';
    name5[1] = 's';
    name5[2] = 'h';
    name5[3] = 'a';
    name5[4] = 'n';
    name5[5] = '5';
    name5[6] = '\0';
    printf("name5: %s\n", name5);
    printf("name4: %s\n", name4);
    printf("name3: %s\n", name3);
    printf("name2: %s\n", name2);
    printf("name: %s\n", name);


    
    int i;

    char* ptr1 = (char*)bud_malloc(10); // 32


    ptr1[0] = 'a';
    ptr1[1] = 'b';
    ptr1[2] = 'c';
    ptr1[3] = 'd';
    ptr1[4] = 'e';
    ptr1[5] = 'f';
    ptr1[6] = '\0';
    printf("ptr1 string: %s\n", ptr1);

    int* ptr2 = bud_malloc(sizeof(int) * 100); // 512
    for(i = 0; i < 100; i++)
        ptr2[i] = i;

    void* ptr3 = bud_malloc(3000); // 4192
    printf("ptr3: %p\n", ptr3);

    
    
    
    ptr2 = bud_realloc(ptr2, 124); // 128

    ptr1 = bud_malloc(200); // 256
    ptr1 = bud_realloc(ptr1, 100); // 128

    // intentional error (errno = EINVAL)
    ptr3 = bud_malloc(20000);
    printf("errno: %d (%s)\n", errno, strerror(errno));
    */

    bud_mem_fini();
    return EXIT_SUCCESS;
}
