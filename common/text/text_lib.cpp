#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "logger.h"
#include "text_lib.h"

//----------------------------------------------------------------------------------------------

ssize_t find_file_size(FILE* istream) {
    assert(istream != nullptr);

    struct stat file_data = {};

    if (fstat(fileno(istream), &file_data) == -1) {
        LOG(ERROR, "Failed to read a file\n" STRERROR(errno));
        return -1;
    }

    if (file_data.st_size == 0) {
        LOG(WARNING, "Empty file\n" STRERROR(errno));
    }

    LOG(INFO, "File size: %zu\n", file_data.st_size);
    return (ssize_t) file_data.st_size;
}

//----------------------------------------------------------------------------------------------

text_error_t text_ctor(text_t* text, FILE* istream) {
    assert(istream != nullptr);
    assert(text != nullptr);

    ssize_t symbols_amount = find_file_size(istream);

    if (symbols_amount == -1) {
        return TEXT_FILE_READ_ERROR;
    }

    symbols_amount++;

    text->symbols_amount = (size_t) symbols_amount;

    text->symbols = (unsigned char*) calloc(text->symbols_amount, sizeof(char));
    if (text->symbols == nullptr) {
        LOG(ERROR, "FAILED TO ALLOCATE THE MEMORY\n" STRERROR(errno));
        return TEXT_MEMORY_ALLOCATE_ERROR;
    }

    ssize_t position_in_file = ftell(istream);
    if (position_in_file == -1) {
        return TEXT_PTR_POSITION_INDICATION_ERROR;
    }

    if (fseek(istream, 0, SEEK_SET)) {
        LOG(ERROR, "FAILED TO MOVE POINTER IN THE FILE\n" STRERROR(errno));
        text_dtor(text);
        return TEXT_INFILE_PTR_MOVING_ERROR;
    }

    if ((fread(text->symbols, sizeof(char), text->symbols_amount, istream) != text->symbols_amount) &&
         !feof(istream) &&
         ferror(istream)) {
        LOG(ERROR, "FILE READ ERROR\n" STRERROR(errno));
        text_dtor(text);
        return TEXT_FILE_READ_ERROR;
    }

    if (fseek(istream, position_in_file, SEEK_SET)) {
        LOG(ERROR, "FAILED TO MOVE POINTER IN THE FILE\n" STRERROR(errno));
        text_dtor(text);
        return TEXT_INFILE_PTR_MOVING_ERROR;
    }

    LOG(INFO, "Text structure was successfully created and initialized\n");
    return TEXT_NO_ERRORS;
}

void text_dtor(text_t* text) {
    assert(text != nullptr);

    free(text->symbols);
    text->symbols = nullptr;

    text->symbols_amount = 0;
    LOG(INFO, "Text stracture was sucessfully destructed\n");
}

//----------------------------------------------------------------------------------------------

void get_text_symbols(text_t* text, FILE* istream) {
    assert(text != nullptr);
    assert(istream != nullptr);

    int c = 0;

    for (size_t i = 0; i < text->symbols_amount && ((c = fgetc(istream)) != EOF); i++) {
        text->symbols[i] = (unsigned char) c;
    }

    text->symbols[text->symbols_amount - 1] = '\0';
    LOG(INFO, "TEXT WAS SUCCESSFULLY READ\n");
}
