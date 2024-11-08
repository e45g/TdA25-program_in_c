#include <stdio.h>

void fast_strcat(char *dest, const char *src){
    while (*dest) dest++;
    while((*dest++ = *src++));
}

void strnum(char *dest, double num){
    while (*dest) dest++;
    snprintf(dest, 32, "%g", num);
}
