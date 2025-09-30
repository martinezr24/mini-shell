#ifndef DYNAMICSTRING_H
#define DYNAMICSTRING_H
#include <stdlib.h> 
#include <stddef.h> 

typedef struct dynamicstring{
	char* buf;  // buffer
	size_t capacity; // how big buffer is, including the '\0' 
	size_t length;   // length of string not including the '\0' 

}dynamicstring_t;

// input is a legal NULL terminated C style string
dynamicstring_t* DynamicString_Create(const char* input);

// free's the dynamic string and free's the underlying memory of the dynamicstring_t.
// the underlying memory should be freed first in 'buf'
// prior to freeing a heap allocated dynamicstring.
// returns '1' on success, or 0 in failure.
int DynamicString_Free(dynamicstring_t* str);

// retrieve the string length
// operations on NULL dynamicstrings return a length of 0.
size_t DynamicString_GetLength(dynamicstring_t* str);

// retrieve the string capacity
// operations on NULL dynamicstrings return a capacity of 0.
size_t DynamicString_GetCapacity(dynamicstring_t* str);


// get Character at a specific index
// operations on NULL dynamicstrings return the null character.
char DynamicString_Get(dynamicstring_t* str, int index);

// retrieve the string buffer
// operations on NULL dynamicstrings return the null character.
const char* DynamicString_GetCString(dynamicstring_t* str);

// set the string to a specific new string
// operations on NULL dynamicstrings return the null character.
void DynamicString_Set(dynamicstring_t* str, char* input);

// append a single 'char' to a dynamicstring_t.
int DynamicString_AppendChar(dynamicstring_t* str, char c);

// append a c-style string to a dynamicstring_t 
int DynamicString_AppendDynamicString(dynamicstring_t* str, const char* input);

// allocates a new 'dynamicstring' based from a substring of an existing string.
dynamicstring_t* DynamicString_NewStringFromSlice(dynamicstring_t* str, int start, int end);

// split a dynamicstring based on delimeters. 
int DynamicString_Split(dynamicstring_t* input, const char* delimeters, dynamicstring_t*** array, int* size);

#endif
