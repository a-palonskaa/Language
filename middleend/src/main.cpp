#include <cstdlib>
#include <errno.h>
#include <string.h>
#include "prog_tree.h"
#include "middleend.h"
#include "logger.h"

int main() {
    FILE* logger = fopen("logs/middleend_logger.txt", "w");
    if (logger == nullptr) {
        LOG(ERROR, "Failed to open a logger ostream\n");
        return 1;
    }

    LoggerSetFile(logger);
    LoggerSetLevel(INFO);

    FILE* istream = fopen("out.txt", "r");
    if (istream == nullptr) {
        LOG(ERROR, "Failed to open an input data file\n");
        return 1;
    }

    FILE* dump_file = fopen("m_out.txt", "w");
    if (istream == nullptr) {
        LOG(ERROR, "Failed to open an input data file\n");
        return 1;
    }

    FILE* dump = fopen("data/dump.html", "w");
    if (istream == nullptr) {
        LOG(ERROR, "Failed to open an input data file\n");
        return 1;
    }

    middleend_t prog = {};

    prog.set_dump_ostream(dump);
    prog.init(istream);
    prog.dump();
    prog.deserialization(dump_file);
    prog.dtor();

    if (fclose(istream) == EOF) {
        LOG(ERROR, "Failed to close data file\n" STRERROR(errno));
        return 1;
    }

    if (fclose(dump_file) == EOF) {
        LOG(ERROR, "Failed to close data file\n" STRERROR(errno));
        return 1;
    }

    if (fclose(logger) == EOF) {
        fprintf(stderr, "Failed to close tex file\n" STRERROR(errno));
        return 1;
    }
    return 0;
}
