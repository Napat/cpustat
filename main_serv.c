/**
 * @file main_serv.c
 * @author Napat Rungruangbangchan
 * @date 20 June 2017 
 * @brief Get CPU info: usage percentage in total and for each core
 * @Compiler: gcc
 * @License: GNU General Public License v2.0 
 */

#include <fcntl.h>
#include <unistd.h>
#include <cpustat.h>

#define PRINTF(...)		do {  if(cpustat_isdebug(cpustat_info())==true) fprintf(stderr, __VA_ARGS__); 	} while (0)

void show_version( char *argv[]){
	fprintf(stdout, "%s version %s\n", argv[0], CPUSTAT_VERSION);
	return;
}

int main(int argc, char *argv[]){	
	int idx;

	int opt;
    while ((opt = getopt(argc, argv, "dv")) != -1) {
        switch (opt) {
        case 'd': cpustat_isdebug_set(cpustat_info(), true); break;
        case 'v': 
        	show_version(argv);
        	exit(EXIT_SUCCESS);
        	break;        
        default:
            fprintf(stderr, "Usage: %s [-dv] [file...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    PRINTF("Debug: %s\r\n", (cpustat_isdebug(cpustat_info())==false)?"DISABLE":"ENABLE" );

	init_cpustat_monitor(3);
	start_cpustat_monitor();

	PRINTF("CPU");	
	for(idx = 1; idx <= cpustat_number_cpucores(cpustat_info()); idx++){
		PRINTF("\tCPU#%d", idx);
	}	
	PRINTF("\r\n");

	while(1){
		for(idx = 0; idx<=cpustat_number_cpucores(cpustat_info()); idx++){
			PRINTF("%.2d\t", cpustat_cpux_percentload(cpustat_info(), idx));
		}
		PRINTF("\r\n");

		sleep(1);
	}
	return 0;
}
