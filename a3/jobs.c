#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "ring.h"

int word_count(RingMessage rm) {
    // Simple approach: A word is anything separated by spaces

    char[MAX_PAYLOAD] text = rm.payload;
    int num_of_spaces = 0;
    int text_length = 0;
    
    for (int i = 0; i < text_length; i++) {
	if text[i] == ' ' {
	    num_of_spaces++;
	}
    }

    return (num_of_spaces + 1);
}

int 