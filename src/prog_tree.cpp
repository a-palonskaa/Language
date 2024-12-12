#include <assert.h>
#include "prog_tree.h"
#include "logger.h"

err_t prog_tree_t::init(FILE* data_file) {
    assert(data_file != nullptr);

    text_t text = {};
    if (text_ctor(&text, data_file) != TEXT_NO_ERRORS) {
        LOG(ERROR, "Failed to read text\n");
        return SYNTAX_ERR;
    }

    for (size_t i = 0; i < 100; i++) {
        var_nametable_[i].initialized = false;
    }

    root_ = token_init(&text);
    dump_tree();
    text_dtor(&text);
    return NO_ERR;
}

void prog_tree_t::dtor() {
    free(tokens_);
    tokens_ = nullptr;
}
