
# cpustat 
C api to get CPU information from Linux, number of cores, utilization, etc. 
This project provide simple applications based on cpustat api.
The server, gathering and copy information from linux to sharemem.
The client, read sharemem and display at standard output.   

## Setup
The default complier is gcc. For cross-compile project, edit `CROSS_COMPILE` in `make.rules`.  

## Compile
Binary will place to `<cpustat_dir>/bin` after compilation.   
```
$ cd <cpustat_dir>
$ make
$ ls ./bin
client_sample.out  cpustat.out
```

## Server(cpustat.out)
The cpustat(server) will get cpu and copy information to sharemem(if enable) then any applications can access data using sharemem key define in cpustat.h   
`#define CPUSTAT_SHAREMEM_KEY   (1986)`

## Client(client_sample.out)
The simple application to read and print cpustat data to standard output.
