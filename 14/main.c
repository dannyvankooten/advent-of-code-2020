#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "hashmap.h"

struct instruction {
    enum {
        SET_MASK,
        SET_VALUE,
    } type;
    char mask[36];
    unsigned long address;
    unsigned long value;
};

void print_binary(unsigned long n);

unsigned long apply_bitmask(char mask[36], unsigned long n) {
    char k;
    unsigned long r = 0;

    for (int i=0; i < 36; i++) {
        // get bit in n at position i
        k = (n >> (35-i)) & 1;

        // if mask[i] equals 'X', take bit from number
        if ((mask[i] == 'X' && k == 1) || mask[i] == '1') {
            r += (unsigned long) pow(2.00, (double) 35-i);
        } 
    }

    return r;
}

unsigned long long bin2dec(char mask[36]) {
    unsigned long long r = 0;

    for (int i=0; i < 36; i++) {
        if (mask[i] == '1') {
            r += (unsigned long long) pow(2.00, (double) 35-i);
        } 
    }
    return r;
}

struct vec {
    unsigned long *values;
    int size;
    int cap;
};

void 
apply_address_mask(struct vec *v, char mask[36], unsigned long n) {
    // generate all possible combinations
    char k;
    int m;

    // reset vector
    v->size = 1;
    v->values[0] = 0;

    for (int i=0; i < 36; i++) {        
        switch (mask[i]) {
            case '0':
                k = (n >> (35-i)) & 1;
                if (k == 1) {
                    for (m=0; m < v->size; m++) {
                        v->values[m] += (unsigned long) pow(2, 35-i);
                    }
                }
                break;
            case '1':
                for (m=0; m < v->size; m++) {
                    v->values[m] += (unsigned long) pow(2, 35-i);
                }
                break;
            case 'X': {
                int size_before = v->size; 

                // add each variation with i'th bit set to 0
                for (m=0; m < size_before; m++) {
                    v->values[v->size++] = v->values[m];
                }

                // and add each variation with it's bit set to 1
                for (m=0; m < size_before; m++) {
                    v->values[m] += (unsigned long) pow(2, 35-i);
                }
              
                }
                break;
        }
    }
}

void print_binary(unsigned long n) {
    int c, k;
    for (c = 35; c >= 0; c--) {
        k = n >> c;

        if (k & 1) {
            printf("1");
        } else {
            printf("0");
        }
    }
}

int main() {    
   // assert(apply_bitmask("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX1XXXX0X", 11) == 73);
    FILE *f = fopen("input.txt", "r");
    if (!f) err(EXIT_FAILURE, "error reading input file");
    char linebuf[BUFSIZ] = {0};
    char nbuf[32] = {0};

    // parse program
    int cap = 592;
    int size = 0;
    struct instruction *instructions = malloc(cap * sizeof(struct instruction));
    while (fgets(linebuf, BUFSIZ, f) != NULL) {
        struct instruction ins;
        char *s = linebuf;
        char *tmp = nbuf;

        if (strncmp(s, "mask = ", 7) == 0) {
            ins.type = SET_MASK;
            char *mask = ins.mask;
            s += 7; // skip "mask" 
            while (*s != 0 && *(s - 2) != '=') s++;
            while (*s != '\0' && *s != '\n') {
                *mask++ = *s++;
            }
        } else {
            ins.type = SET_VALUE;
            s += 3; // skip "mem"
            while (*(s-1) != '[') s++;
            while (*s != ']') *tmp++ = *s++;
            *tmp = '\0';
            ins.address = strtol(nbuf, NULL, 10);

            tmp = nbuf;
            while (*s != '=' && *s != 0) s++;
            while (*s < '0' || *s > '9') s++;
            while (*s >= '0' && *s <= '9') *tmp++ = *s++;
            *tmp = '\0';
            ins.value = strtol(nbuf, NULL, 10);
        }

        // add instruction to list
        instructions[size++] = ins;
        if (size == cap) {
            cap *= 2;
            instructions = realloc(instructions, cap * sizeof(struct instruction));
        }
    }
    fclose(f);

    // initialize hashmap for storing memory
    struct hashmap hm = hashmap_new();
    struct vec addresses = {
        .size = 0,
        .cap = 1024,
        .values = malloc(1024 * sizeof(unsigned long)),
    };

    // walk through instructions
    char mask[36];
    int j;
    for (int i = 0; i < size; i++) {
        #ifdef STEP
        printf("\n%d  \tmask=\t\t%.36s\n", i, mask);
        #endif

        switch (instructions[i].type) {
            case SET_MASK:
                for (j=0; j < 36; j++) {
                    mask[j] = instructions[i].mask[j];
                }
            break;

            case SET_VALUE: {
                #ifdef STEP
                printf("address= \t\t%ld \t", instructions[i].address);
                print_binary(instructions[i].address);
                printf("\n");
                #endif

                apply_address_mask(&addresses, mask, instructions[i].address);
                for (j=0; j < addresses.size; j++) {
                    hashmap_set(&hm, addresses.values[j], instructions[i].value);
                }
            }
            break;
        }        
    }

    unsigned long sum = hashmap_sum(&hm);
    printf("Sum: %ld\n", sum);

    free(instructions);
    free(addresses.values);
    assert(sum == 4173715962894);
    //printf("Memory size at finish: %ld\n", mem_size);
}