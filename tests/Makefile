CFLAGS = -Wall -O0 -g
CC = g++

# File path to Arduino files
ARD_DIR = ../temp_controller

######################### TEMP CONTROLLER ############################
TEST_OBJS = tests.o mocks.o
ARD_OBJ = temp_controller.o
OBJS = $(TEST_OBJS) $(ARD_OBJ)

tests: $(OBJS)
	${CC} ${CFLAGS} $^ -o $@
	./tests.exe
	python .\plotcsv.py .\test3.csv

# Arduino files
$(ARD_OBJ): $(ARD_DIR)/temp_controller.ino
	${CC} ${CFLAGS} -x c++ -c $< -o $@ -DTESTING

######################### GENERAL ############################

# For testing files from this directory
%.o: %.cpp
	${CC} ${CFLAGS} -c $< -o $@

.PHONY: clean
clean:
	rm -r -f *.o *.exe *.csv $(OBJS_DIR)