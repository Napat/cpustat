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

#include "cpustat.h"


int main(){	
	int idx;

	init_cpustat_monitor(3);
	start_cpustat_monitor();

	printf("CPU");	
	for(idx = 1; idx <= cpustat_number_cpucores(cpustat_info()); idx++){
		printf("\tCPU#%d", idx);
	}	
	printf("\r\n");

	while(1){
		for(idx = 0; idx<=cpustat_number_cpucores(cpustat_info()); idx++){
			printf("%.2d\t", cpustat_cpux_perentload(cpustat_info(), idx));
		}
		printf("\r\n");

		sleep(1);
	}
	return 0;
}
