#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*
    typ följande saker borde den färga olika:
    - pekare till binärens RO-delar
    - pekare till binärens RW-delar (utom heap)
    - pekare till heap
    - pekare till övriga lib RO-delar
    - pekare till övriga lig RW-delar
    - pekare till stacken
    - värden
    - stack-cookie
*/

void hexdump_stack() {
    // Start by printing memory map.
    char line[256] = {};
    FILE* f = fopen("/proc/self/maps", "r");
    if (f == NULL) {
        puts("[*] Error when reading /proc/self/maps. Contact admin!");
        exit(-1);
    }
    puts("--------------------- Virtual memory map ---------------------");
    puts("        Start Addr           End Addr   Length Perms File/Area");
    while(fgets(line, 256, f) != NULL) {
        uintptr_t start, end, size;
        char perms[4] = {}, path[100] = {};
        sscanf(line, "%zx-%zx %3sp %*x %*x:%*x %*d %s\n", &start, &end, perms, path);
        size = end-start;
        printf("%18p %18p %8p   %s %s\n", start, end, size, perms, path);
    }
    puts("");
    fclose(f);

    // Hex-dump caller's stack
    puts("--------- The stack --------");
    void *start = __builtin_frame_address(0) + 2*sizeof(void*);
    void *end = __builtin_frame_address(1) + 3*sizeof(void*);
    for(uintptr_t *rsp = start; rsp != end; rsp++) {
        printf("%p: 0x%016zx\n", rsp, *rsp);
    }
}

int main() {
    char name[30];
    int n = 0;
    puts("Write your name:");
    scanf("%s", name);
    puts("What is your age?");
    scanf("%d", &n);
    printf("Here is your info: %s %d\n", name, n);
    hexdump_stack();
}
