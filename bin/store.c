#include "store.h"
#include "config.h"
#include <dirent.h>

Store   *g_store;

int     
store_load()
{
    g_store = zc_malloct(Store);
    g_store->data = zc_dict_new(100000);

    char datapath[256] = {0}; 
    snprintf(datapath, sizeof(datapath), "%s/data", g_conf->homedir);


	DIR *d; 
    struct dirent *file;
    struct stat sb;    
    char    filename[256];
	char 	path[256] = {0};
	char i;
	FILE	*f;
	char	line[1024];

	for (i=97; i<123; i++) {
		snprintf(path, sizeof(path), "%s/%c", datapath, i);
		d = opendir(path); 
		if (!d) {
			ZCERROR("opendir error: %s %d!", path, errno);
			return ZC_ERR;
		}   
		while ((file = readdir(d)) != NULL) {
			if (strncmp(file->d_name, ".", 1) == 0)
				continue;
			snprintf(filename, sizeof(filename), "%s/%s", path, file->d_name);
			if (stat(filename, &sb) < 0) {
				ZCERROR("stat error: %s", filename);
				continue;
			}   
			//strcpy(filename[len++], file->d_name);
			//func(filename, file->d_name, sb.st_mode);

			if (S_ISREG(sb.st_mode)) {
				f = fopen(filename, "r");	

				while (fgets(line, 1000, f) != NULL) {
					// name,ip,port,timeout,weight,load	
                    
						
				}

				fclose(f);
			}
		}   
		closedir(d);
	}


    return ZC_OK;
}

int     
store_save()
{
    return ZC_OK;
}



