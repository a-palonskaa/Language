#include <cstdlib>
#include <errno.h>
#include <string.h>
#include "prog_tree.h"
#include "logger.h"

const char* del_images = "./del_images.sh";

int main() {
    FILE* logger = fopen("logger.txt", "w");
    if (logger == nullptr) {
        LOG(ERROR, "Failed to open a logger ostream\n");
        return 1;
    }

    LoggerSetFile(logger);
    LoggerSetLevel(INFO);

    int system_execution_status = system(del_images);
    if (system_execution_status == -1 || system_execution_status == 127) {
        LOG(ERROR, "Failed to execute bash script %s\n", del_images);
    }

    FILE* file = fopen("data/dump.html", "wb");
    if (file == nullptr) {
        LOG(ERROR, "Failed to open a dump ostream\n");
        return 1;
    }

    FILE* istream = fopen("data/input/data.txt", "r");
    if (istream == nullptr) {
        LOG(ERROR, "Failed to open an input data file\n");
        return 1;
    }

    prog_tree_t tree = {};

    tree.set_dump_ostream(file);
    tree.init(istream);

    tree.dump_tree();
    tree.dtor();

    if (fclose(file) == EOF) {
        LOG(ERROR, "Failed to close html file\n" STRERROR(errno));
        return 1;
    }

    if (fclose(istream) == EOF) {
        LOG(ERROR, "Failed to close data file\n" STRERROR(errno));
        return 1;
    }

    if (fclose(logger) == EOF) {
        fprintf(stderr, "Failed to close tex file\n" STRERROR(errno));
        return 1;
    }
    return 0;
}
