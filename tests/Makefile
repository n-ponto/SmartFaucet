CFLAGS = -Wall -O0 -g
CC = g++

# File path to Arduino files
ARD_DIR = ../temp_controller

######################### TEMP CONTROLLER ############################
ARD_OBJ = temp_controller.o

all: simulate_tests
	python .\plotcsv.py 

unit_tests simulate_tests: % : %.o mocks.o $(ARD_OBJ)
	${CC} ${CFLAGS} $^ -o $@
	./$@.exe

# Arduino files
$(ARD_OBJ): $(ARD_DIR)/temp_controller.ino
	${CC} ${CFLAGS} -x c++ -c $< -o $@ -DTESTING

######################### GENERAL ############################

# For testing files from this directory
%.o: %.cpp
	${CC} ${CFLAGS} -c $< -o $@

.PHONY: clean
clean:
	rm -r -f *.o *.exe *.csv $(OBJS_DIR) csv plots