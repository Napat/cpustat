/**
 * @file cpustat.c
 * @author Napat Rungruangbangchan
 * @date 20 June 2017 
 * @brief Get CPU info: usage percentage in total and for each core
 * @Compiler: gcc
 * @License: GNU General Public License v2.0
 */

#include <pthread.h>
#include <math.h>
#include <sys/mman.h>

#include "cpustat.h"

#define PROCSTAT "/proc/stat"
//#define DEBUG_CPUSTAT

#ifdef ENABLE_SHAREMEM	

static void shm_init_val(cpustat_info_t *info){
	memcpy((cpustat_info_t *)info->shmem, info, sizeof(info));
}

static int shm_init_server(cpustat_info_t *info, int key) {
	int shmid;
	printf("%s(%d) Initializing shared memory.\r\n", __FUNCTION__, __LINE__);

	//system_sync_printf("echo %d > /tmp/ipc_sharemem_size", sizeof(cpustat_info_t));
	shmid = shmget(key, sizeof(*info), 0666 | IPC_CREAT);
	if(shmid < 0) {
		printf("%s(%d): Fail to allocate shared memory.\r\n", __FUNCTION__, __LINE__);
		return -1;
	}
	info->shmem = shmat(shmid, (void *)0, 0);
	memset(info->shmem, 0, sizeof(*info));
	return 0;
}
#endif	/* ENABLE_SHAREMEM */

cpustat_info_t * cpustat_info_client_sharemem() {
	static cpustat_info_t* info = NULL;
#ifdef ENABLE_SHAREMEM
	if(info == NULL){
		int shmid;
		printf("%s(%d) Initializing shared memory.\r\n", __FUNCTION__, __LINE__);

		//system_sync_printf("echo %d > /tmp/ipc_sharemem_size", sizeof(cpustat_info_t));
		shmid = shmget(CPUSTAT_SHAREMEM_KEY, sizeof(*info), 0666);
		if(shmid < 0) {
			printf("%s(%d): Fail to get shared memory.\r\n", __FUNCTION__, __LINE__);
			return NULL;
		}
		info = (cpustat_info_t*)shmat(shmid, (void *)0, 0);
	}
#endif	/* ENABLE_SHAREMEM */
	return info;
}

static cpustat_info_t *new_cpustat_info() {
	cpustat_info_t *info;
	info = calloc(1, sizeof(cpustat_info_t));

	// init default values
	info->sampling_timesec = 1;	
	info->number_cpucores = number_cpucores();	

#ifdef ENABLE_SHAREMEM	
	shm_init_server(info, CPUSTAT_SHAREMEM_KEY);
	shm_init_val(info);	
#endif
	return info;
}

/**
 * @brief Singleton of cpu status infomation
 * 
 */
cpustat_info_t* cpustat_info() {
	static cpustat_info_t* info = NULL;
	
	if(info == NULL){
		info = new_cpustat_info();
	}	
	return info;
}

// cpustat api

static void *cpuinfo_monitor_thread_func(void *ptr) {
	cpustat_info_t *info = (cpustat_info_t*)ptr;
	int idx;
	int cpucores = cpustat_number_cpucores(info);	
	double *percentage;	
	unsigned long long **pre_cpustatsnap = malloc_cpustatsnap( cpucores );
	unsigned long long **now_cpustatsnap = malloc_cpustatsnap( cpucores );
	int cpux_load;
	
	#ifdef DEBUG_CPUSTAT		
		printf("CPU");	
		for(idx = 1; idx <= cpucores; idx++) {
			printf("\tCPU#%d", idx);
		}	
		printf("\r\n");
	#endif
	
	sampling_cpustatsnap(cpucores, now_cpustatsnap);
	while(1) {
		//sampling_cpustatsnap(cpucores, pre_cpustatsnap);
		memcpy_cpustatsnap(cpucores, pre_cpustatsnap, now_cpustatsnap);
		sleep(cpustat_sampling_timesec(info));
		sampling_cpustatsnap(cpucores, now_cpustatsnap);

		for(idx = 0; idx<=cpucores; idx++){
			cpux_load = cal_cpu_percentage(now_cpustatsnap[idx], pre_cpustatsnap[idx]);
			cpustat_cpux_perentload_set(info, idx, cpux_load);
			#ifdef DEBUG_CPUSTAT
				printf("%.2d\t", cpustat_cpux_perentload(info, idx));
			#endif
		}
		printf("\r\n");
	}
	free_cpustatsnap( cpucores, pre_cpustatsnap );
	free_cpustatsnap( cpucores, now_cpustatsnap );
	return NULL;
}



/**
 * @brief init cpustat function
 * 
 */
bool init_cpustat_monitor(int sampling_timesec){
	return cpustat_sampling_timesec_set(cpustat_info(), sampling_timesec);
}

/**
 * @brief Start cpustat thread handler
 * 
 */
int start_cpustat_monitor(){
	pthread_t thread;
	if(pthread_create(&thread, NULL, cpuinfo_monitor_thread_func, (void*)cpustat_info())){
		printf("%s(%d) pthread_create failed\r\n", __FUNCTION__, __LINE__);
		return -1;
	}
	return 0;
}

/**
 * @brief Get sampling cpu stat time in second
 * 
 */
int cpustat_sampling_timesec(cpustat_info_t* info){
	return info->sampling_timesec;
}

/**
 * @brief Set sampling cpustat time in second
 * 
 */
bool cpustat_sampling_timesec_set(cpustat_info_t* info, int sampling_timesec){
#ifdef ENABLE_SHAREMEM
	cpustat_info_t *shmem_ptr = (cpustat_info_t *)info->shmem;
	shmem_ptr->sampling_timesec = sampling_timesec;
#endif	
	info->sampling_timesec = sampling_timesec;
	
	return true;
}

/**
 * @brief Get number of cpu cores
 * 
 */
int cpustat_number_cpucores(cpustat_info_t* info){
	return info->number_cpucores;
}

/**
 * @brief Set number of cpu cores
 * 
 */
bool cpustat_number_cpucores_set(cpustat_info_t* info, int number_cpucores){
#ifdef ENABLE_SHAREMEM
	cpustat_info_t *shmem_ptr = (cpustat_info_t *)info->shmem;
	shmem_ptr->number_cpucores = number_cpucores;
#endif	
	info->number_cpucores = number_cpucores;
	return true;
}

/**
 * @brief Get percent of cpu load
 * 
 * Note that index 0 is the average of all cpus percentage
 */
int cpustat_cpux_perentload(cpustat_info_t* info, int cpu_idx){
	return info->cpuload_corex[cpu_idx];
}

/**
 * @brief Set percent of cpu load
 * 
 * Note that index 0 is the average of all cpus percentage
 */
bool cpustat_cpux_perentload_set(cpustat_info_t* info, int cpu_idx, int load_percentage){
#ifdef ENABLE_SHAREMEM
	cpustat_info_t *shmem_ptr = (cpustat_info_t *)info->shmem;
	shmem_ptr->cpuload_corex[cpu_idx] = load_percentage;
#endif	
	info->cpuload_corex[cpu_idx] = load_percentage;
	return true;
}


// cpustatsnap api

/**
 * @brief Get how many of CPU cores
 * 
 */
int	number_cpucores(){
	FILE *fp = fopen(PROCSTAT, "r");
	int n = 0;
	char line[100];
				
	while(!feof(fp)){
		fgets(line, 100, fp); 
		if(line[0] == 'c' && line[1] == 'p' && line[2] == 'u') n++;
	}
	fclose(fp);
	return n - 1;
}

/**
 * @brief Allocate cpustatsnap buffer
 * @param cpucores Number of CPU cores, see cpustat_number_cpucores() or number_cpucores().
 * 
 */
unsigned long long **malloc_cpustatsnap( int cpucores){
	unsigned long long **cpustatsnap;
	int idx;

	cpustatsnap = (unsigned long long **)malloc((cpucores + 1) * sizeof(unsigned long long));
	for(idx = 0; idx< (cpucores + 1); idx++){
		cpustatsnap[idx] = (unsigned long long *)malloc(4 * sizeof(unsigned long long));
	}
	return cpustatsnap;
}

/**
 * @brief Deallocate cpustatsnap buffer
 * @param cpucores Number of CPU cores, see cpustat_number_cpucores() or number_cpucores().
 * @param cpustatsnap to deallocate.
 */
void free_cpustatsnap( int cpucores, unsigned long long **cpustatsnap ){
	int idx;
	for(idx = 0; idx < (cpucores + 1); idx++){
		free(cpustatsnap[idx]);
		cpustatsnap[idx] = NULL;
	}
	return;
}

/**
 * @brief To copy cpustatsnap values from source to destination buffer
 * @param cpucores Number of CPU cores, see cpustat_number_cpucores() or number_cpucores().
 * @param dst_cpustatsnap destination buffer.
 * @param src_cpustatsnap source buffer.
 */
void memcpy_cpustatsnap( int cpucores, unsigned long long **dst_cpustatsnap, unsigned long long **src_cpustatsnap ){
	int idx;
	for(idx = 0; idx < (cpucores + 1); idx++){
		dst_cpustatsnap[idx][0] = src_cpustatsnap[idx][0];
		dst_cpustatsnap[idx][1] = src_cpustatsnap[idx][1];
		dst_cpustatsnap[idx][2] = src_cpustatsnap[idx][2];
		dst_cpustatsnap[idx][3] = src_cpustatsnap[idx][3];
	}
	return;
}

/**
 * @brief To reads /proc/stat info to cpustatsnap buffer
 * @param cpucores Number of CPU cores, see cpustat_number_cpucores() or number_cpucores().
 * @param cpustatsnap Buffer to keep snapshot values. 
 * @return cpustatsnap[cpu_num][process_type] of process count
 */
unsigned long long **sampling_cpustatsnap (int cpucores, unsigned long long **cpustatsnap ){
	FILE *fp;
	int idx;
	unsigned long long ignore[6];

	fp = fopen(PROCSTAT, "r");
	for(idx = 0; idx < (cpucores + 1); idx++){
		fscanf(fp, "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu ",
				&cpustatsnap[idx][0], &cpustatsnap[idx][1], &cpustatsnap[idx][2], &cpustatsnap[idx][3], 
				&ignore[0], &ignore[1], &ignore[2], &ignore[3], &ignore[4], &ignore[5]);
	}
	fclose(fp);
	return cpustatsnap;
}

/**
 * @brief Calculate cpu percentage from two point of malloc_cpustat() data
 * @param snap1 cpustatsnap buffer at time1
 * @param snap2 cpustatsnap buffer at time2 
 * @return A double value of CPU usage percentage from given array[4] = {USER_PROC, NICE_PROC, SYSTEM_PROC, IDLE_PROC}
 */
int cal_cpu_percentage ( unsigned long long *snap1, unsigned long long *snap2 ) {
	return ceil(((double)((snap1[0] - snap2[0]) + (snap1[1] - snap2[1]) + (snap1[2] - snap2[2])) / 
				(double)((snap1[0] - snap2[0]) + (snap1[1] - snap2[1]) + (snap1[2] - snap2[2]) + (snap1[3] - snap2[3]))) * 100);
}
