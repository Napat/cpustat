/**
 * @file cpustate.h
 * @author Napat Rungruangbangchan
 * @date 20 June 2017 
 * @brief Get CPU info: usage percentage in total and for each core
 * @Compiler: gcc
 * @License: GNU General Public License v2.0  
 */

#ifndef __CPUSTATE_H__
#define __CPUSTATE_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define CPUSTAT_ENABLE_SHAREMEM

#define CPUSTAT_VERSION 		"0.70"
#define CPUSTAT_SHAREMEM_KEY	(1986)

#define CPUSTAT_MAXCPU			(32)
struct cpustat_info_s {
	bool debug;							/**< debug enable/disable */
	int sampling_timesec;				/**< Sampling time between any snapshot use to calculate */
	int number_cpucores;				/**< Number of cpus cores */
	int cpuload_corex[CPUSTAT_MAXCPU];	/**< CPUx load percentage, Note that index 0 is the average of all cpus percentage */
#ifdef CPUSTAT_ENABLE_SHAREMEM
	int* shmem;
#endif	
};
typedef struct cpustat_info_s cpustat_info_t;

#ifdef CPUSTAT_ENABLE_SHAREMEM
// for client apps to access cpustat info via sharemem
cpustat_info_t * cpustat_info_client_sharemem(); 
#endif

// cpustat api: need to init & start thread handler
cpustat_info_t* cpustat_info();
bool init_cpustat_monitor(int sampling_timesec);
int start_cpustat_monitor();
bool cpustat_isdebug(cpustat_info_t* info);
bool cpustat_isdebug_set(cpustat_info_t* info, bool isdebug);
int cpustat_sampling_timesec(cpustat_info_t* info);
bool cpustat_sampling_timesec_set(cpustat_info_t* info, int sampling_timesec);
int cpustat_number_cpucores(cpustat_info_t* info);
bool cpustat_number_cpucores_set(cpustat_info_t* info, int number_cpucores);
int cpustat_cpux_percentload(cpustat_info_t* info, int cpu_idx);
bool cpustat_cpux_percentload_set(cpustat_info_t* info, int cpu_idx, int load_percentage);

// cpustatsnap api: Low level api, no need to init thread but may not effectively and very complex to use.
int	number_cpucores();	// For more efficiency should use cpustat_number_cpucores()
unsigned long long **malloc_cpustatsnap( int cpucores);
void free_cpustatsnap( int cpucores, unsigned long long **cpustatsnap );
void memcpy_cpustatsnap( int cpucores, unsigned long long **dst_cpustatsnap, unsigned long long **src_cpustatsnap );
unsigned long long **sampling_cpustatsnap (int cpucores, unsigned long long **cpustatsnap );
int cal_cpu_percentage ( unsigned long long *snap1, unsigned long long *snap2 );

#endif /* __CPUSTATE_H__ */
