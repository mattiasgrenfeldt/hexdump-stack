#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

// Colors: https://gist.github.com/iamnewton/8754917
#define COLOR_RED    "\e[0;31m"
#define COLOR_GREEN  "\e[0;32m"
#define COLOR_YELLOW "\e[0;33m"
#define COLOR_BLUE   "\e[0;34m"
#define COLOR_PURPLE "\e[0;35m"
#define COLOR_CYAN   "\e[0;36m"
#define COLOR_RESET  "\e[0m"

#define STACK_COLOR COLOR_RED
#define HEAP_COLOR COLOR_GREEN
#define BIN_COLOR COLOR_BLUE
#define LIBC_COLOR COLOR_YELLOW

struct memrange {
    size_t start;
    size_t end;
};

void hexdump_stack() {
    struct memrange stack = {};
    struct memrange heap = {};
    struct memrange bin = {0xf000000000000000, 0};
    struct memrange libc = {0xf000000000000000, 0};
    char binary[256] = {};
    memset(binary, 0, 256);

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
        size_t start, end, size;
        char perms[4] = "", path[100] = "", color[8] = "";
        sscanf(line, "%zx-%zx %3sp %*x %*x:%*x %*d %s\n", &start, &end, perms, path);
        if(binary[0] == 0) {
            strcpy(binary, path);
        }
        if(strcmp(path, "[stack]") == 0) {
            stack.start = start;
            stack.end = end;
            strcpy(color, STACK_COLOR);
        } else if(strcmp(path, "[heap]") == 0) {
            heap.start = start;
            heap.end = end;
            strcpy(color, HEAP_COLOR);
        } else if(strcmp(path, binary) == 0) {
            bin.start = start < bin.start ? start : bin.start;
            bin.end = end > bin.end ? end : bin.end;
            strcpy(color, BIN_COLOR);
        } else if(strstr(path, "/libc-") != 0) {
            libc.start = start < libc.start ? start : libc.start;
            libc.end = end > libc.end ? end : libc.end;
            strcpy(color, LIBC_COLOR);
        }
        
        size = end-start;
        printf("%s%18p %18p %8p   %s %s" COLOR_RESET "\n", color, start, end, size, perms, path);
    }
    puts("");
    fclose(f);

    // Hex-dump caller's stack
    puts("--------------------- The stack ---------------------");
    puts("                   7 6 5 4 3 2 1 0    f e d c b a 9 8");
    size_t *start = __builtin_frame_address(0) + 2*sizeof(void*);
    size_t *end = __builtin_frame_address(1) + 3*sizeof(void*);
    for(size_t *rsp = start; rsp < end; rsp += 2) {
        printf("%p:", rsp);
        for(int i = 0; i < 2; i++) {
            char color[8] = "";
            size_t val = rsp[i];
            if (stack.start <= val && val < stack.end) strcpy(color, STACK_COLOR);
            else if (heap.start <= val && val < heap.end) strcpy(color, HEAP_COLOR);
            else if (bin.start <= val && val < bin.end) strcpy(color, BIN_COLOR);
            else if (libc.start <= val && val < libc.end) strcpy(color, LIBC_COLOR);
            printf(" %s0x%016zx" COLOR_RESET, color, val);
        }
        puts("");
    }
}

void chall() {
    char name[30];
    int n = 0;
    puts("Write your name:");
    scanf("%s", name);
    puts("What is your age?");
    scanf("%d", &n);
    printf("Here is your info: %s %d\n", name, n);
    hexdump_stack();
}

int main() {
    chall();
}
