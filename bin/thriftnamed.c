#include <zocle/zocle.h>
#include "config.h"

int main(int argc, char * argv[]) 
{
    zc_mem_init(ZC_MEM_GLIBC|ZC_MEM_DBG_OVERFLOW);

    if (argc != 2) {
        printf("usage:\n\tthriftnamed configfile\n\n");
        return -1;
    }

	char *filename = argv[1];
    config_load(filename);

    zc_log_new(g_conf->logfile, g_conf->loglevel);
    config_print();

    return 0;
}
