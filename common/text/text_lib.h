#ifndef TEXT_LIB_H
#define TEXT_LIB_H

#include <stdio.h>

typedef struct {
    size_t symbols_amount;
    unsigned char* symbols;
} text_t;

typedef enum {
    TEXT_NO_ERRORS                     = 0,
    TEXT_FILE_READ_ERROR               = 1,
    TEXT_MEMORY_ALLOCATE_ERROR         = 2,
    TEXT_INFILE_PTR_MOVING_ERROR       = 3,
    TEXT_EMPTY_FILE_ERROR              = 4,
    TEXT_PTR_POSITION_INDICATION_ERROR = 5,
} text_error_t;

text_error_t text_ctor(text_t* text, FILE* istream);
void text_dtor(text_t* text);

ssize_t find_file_size(FILE* istream);

void get_text_symbols(text_t* text, FILE* istream);

#endif /* TEXT_LIB_H */

