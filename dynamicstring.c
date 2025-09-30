#include "dynamicstring.h"
#include <stdio.h>

size_t strlen(const char *s) {
	size_t len = 0;  
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

// create a dynamic string
dynamicstring_t* DynamicString_Create(const char* input){
	if (!input) return NULL;
	size_t str_len = strlen(input);

	dynamicstring_t* s = malloc(sizeof(dynamicstring_t));
	if (!s) return NULL;

	s->buf = malloc(str_len + 1);
	if (!s->buf) {
        free(s);
        return NULL;
    }

	for (size_t i = 0; i < str_len; i++) {
        s->buf[i] = input[i];
    }
	s->buf[str_len] = '\0';

	s->length = str_len;
    s->capacity = str_len + 1;

	return s;
}

// free's the dynamic string and free's the underlying memory of the dynamicstring_t.
int DynamicString_Free(dynamicstring_t* str){
	if (!str) return 0;

	if (str->buf) {
		free (str->buf);
		str->buf = NULL;
	}

	free(str);
	return 1;
}

// retrieve the string length
size_t DynamicString_GetLength(dynamicstring_t* str){
	if (!str) return 0;
	
	return str->length;
}

// retrieve the string capacity
size_t DynamicString_GetCapacity(dynamicstring_t* str){
	if (!str) return 0;
	
	return str->capacity;
}

// get Character at a specific index
char DynamicString_Get(dynamicstring_t* str, int index){
	if (!str || !index) return ' ';

	return str->buf[index];
}

// retrieve the string buffer
const char* DynamicString_GetCString(dynamicstring_t* str){
	if (!str) return NULL;
	return str->buf;
}

// set the string to a specific new string
void DynamicString_Set(dynamicstring_t* str, char* input){
	if (!str || !input) return;

	size_t new_len = strlen(input);
	free(str->buf);

	str->buf = malloc(new_len + 1);
    if (!str->buf) {
        str->length = 0;
        str->capacity = 0;
        return;
    }

    for (size_t i = 0; i < new_len; i++) {
        str->buf[i] = input[i];
    }
    str->buf[new_len] = '\0';

    str->length = new_len;
    str->capacity = new_len + 1;
}

// append a single character to our dynamic string
int DynamicString_AppendChar(dynamicstring_t* str, char c){
	if (!str) return 0;

	if (str->length + 1 >= str->capacity) {
        size_t new_capacity;
        if (str->capacity > 0) {
            new_capacity = str->capacity * 2;
        } else {
            new_capacity = 2;
        }

        char* new_buf = realloc(str->buf, new_capacity);
        if (!new_buf) return 0;  

        str->buf = new_buf;
        str->capacity = new_capacity;
    }

    str->buf[str->length] = c;
    str->length++;
    str->buf[str->length] = '\0';

	int result = 1; 

	return result;
}

// append a c-style string to our dynamic string
int DynamicString_AppendDynamicString(dynamicstring_t* str, const char* input){
	if (!str || !input) return 0;
	size_t input_len = strlen(input);

	if (str->length + input_len >= str->capacity) {
        size_t new_capacity;
        if (str->capacity > 0) {
            new_capacity = str->capacity * 2 * input_len;
        } else {
            new_capacity = input_len + 1;
        }

        char* new_buf = realloc(str->buf, new_capacity);
        if (!new_buf) return 0;  

        str->buf = new_buf;
        str->capacity = new_capacity;
    }


	for (size_t i = 0; i < input_len; i++) {
        str->buf[str->length + i] = input[i];
    }
    str->length += input_len;
    str->buf[str->length] = '\0';

	int result = 1; 

	return result;
}

// allocates a new 'string'
dynamicstring_t* DynamicString_NewStringFromSlice(dynamicstring_t* str, int start, int end){
	if (!str || start >=end || start < 0 || end < 0 || end > (int)str->length) return NULL;

    size_t slice_len = end - start;

    char* slice = malloc(slice_len + 1);
    if (!slice) return NULL;


    for (size_t i = 0; i < slice_len; i++) {
        slice[i] = str->buf[start + i];
    }
    slice[slice_len] = '\0';

    dynamicstring_t* new_str = DynamicString_Create(slice);
    free(slice);

    return new_str;
}

// split a dynamic string into multiple dynamic strings that are returned in the output parameter.
int DynamicString_Split(dynamicstring_t* input, const char* delimeters, dynamicstring_t*** array, int* size){
    if (!input || !delimeters || !array || !size) return 0;

    *array = NULL;
    *size = 0;

    size_t start = 0;
    size_t len = input->length;

    for (size_t i = 0; i <= len; i++) {
        char c;
        if (i < len) {
            c = input->buf[i];
        } else {
            c = '\0';
        }

        int is_delim = 0;
        for (const char *d = delimeters; *d; d++) {
            if (c == *d) {
                is_delim = 1;
                break;
            }
        }

        if (c == '\0' || is_delim) {
            if (i > start) {
                dynamicstring_t *token = DynamicString_NewStringFromSlice(input, (int)start, (int)i);
                if (!token) return 0;

                dynamicstring_t **tmp =
                    realloc(*array, sizeof(dynamicstring_t *) * (*size + 1));
                if (!tmp) {
                    DynamicString_Free(token);
                    return 0;
                }

                *array = tmp;
                (*array)[*size] = token;
                (*size)++;
            }
            start = i + 1;
        }
    }

	int result = 1; 

	return result;
}

