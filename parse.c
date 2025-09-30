#include <stdio.h>
#include "dynamicstring.h"

void lexer(char *str) {
    dynamicstring_t* dynamic_str = DynamicString_Create(str);
    const char *delimeters = " ";

    dynamicstring_t **tokens = NULL;
    int token_count = 0;

    int result = DynamicString_Split(dynamic_str, delimeters, &tokens, &token_count);

    if (result != 1) {
        printf("error splitting the string\n");
        return;
    }

    for (int i = 0; i < token_count; i++) {
        printf("%s\n", DynamicString_GetCString(tokens[i]));
        DynamicString_Free(tokens[i]); 
    }

    free(tokens);
    DynamicString_Free(dynamic_str);
}


int main(int argc, char** argv){
    if (argc != 2) return 1;
    lexer(argv[1]);
    
    return 0;
}