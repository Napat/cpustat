# /**
#  * @file Makefile
#  * @author Napat Rungruangbangchan
#  * @date 20 June 2017 
#  * @brief Get CPU info: usage percentage in total and for each core
#  * @Compiler: gcc
#  * @License: GNU General Public License v2.0 
#  */

LDFLAGS += -lpthread -lm
BIN_DIR := bin

all: makedir cpustat mips_cpustat client_sample

FORCE:

makedir: FORCE
	mkdir -p ${BIN_DIR}

cpustat: FORCE
	gcc main_serv.c cpustat.c ${LDFLAGS} -o ${BIN_DIR}/$@.out

mips_cpustat: FORCE
	/opt/codefidence/bin/mipsel-linux-gcc main_serv.c cpustat.c ${LDFLAGS} -o ${BIN_DIR}/$@.out	

client_sample: FORCE
	gcc main_client.c cpustat.c ${LDFLAGS} -o ${BIN_DIR}/$@.out

clean: FORCE
	rm -rf *.o *.out  ${BIN_DIR}

loadtest: FORCE
	yes > /dev/null &

loadtest_stop: FORCE
	killall yes
