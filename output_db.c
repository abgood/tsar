#include "tsar.h"

/* mod->record输出到db */
void output_db(void) {
    printf("%s\n", conf.output_db_mod);
    printf("%s\n", conf.output_db_addr);
}
