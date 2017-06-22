/**
 * @file main_client.c
 * @author Napat Rungruangbangchan
 * @date 20 June 2017 
 * @brief Get CPU info: usage percentage in total and for each core
 * @Compiler: gcc
 * @License: GNU General Public License v2.0 
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "cpustat.h"

int main(int argc, char const *argv[]){
#ifdef ENABLE_SHAREMEM	
	int idx;
	cpustat_info_t * cpustat_info = cpustat_info_client_sharemem(); 
	printf("\t Number of cpu cores: %d\r\n", cpustat_number_cpucores(cpustat_info) );
	printf("\t Set sampling rate to: %d\r\n", cpustat_sampling_timesec(cpustat_info) );
	printf("\t Averaage load of all cpus is %d %%\r\n", cpustat_cpux_perentload(cpustat_info, 0));
	
	for(idx=1; idx<cpustat_number_cpucores(cpustat_info)+1; idx++){
		printf("\t cpu[%d] loading %d %%\r\n", idx, cpustat_cpux_perentload(cpustat_info, idx));
	}
#else
	printf("No way to get info...\r\n");
#endif	
	return 0;
}
