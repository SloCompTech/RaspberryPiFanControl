#DO NOT CHANGE FILE
M_ARCH := $(shell g++ -dumpmachine | awk '{split($$1,a,"-");print a[1]}')
#VARIABLES
CC=g++
CC-RPI=g++-4.9

CFLAGS=-std=c++11
#Files
CORE=CPUregulator.cpp

LIBRARYS=-lwiringPi
#Main compile
all:
	@echo "You have chosen wrong option"
compile:$(CORE) 
	@echo "Compiling Core !!!"
	@$(CC) $(CFLAGS) *.o $(CORE) -o RPiFC  $(LIB)
	@echo "Done"
compile-rpi:
	@$(MAKE) compile CC='$(CC-RPI)' LIB='$(LIBRARYS)' --no-print-directory
install: install-wiringpi compile
    @echo "Done"
    #TODO move program to other folder and set autorun

#other options	
run:RPiFC 
	@sudo ./RPiFC
debug:RPiFC
	@sudo ./RPiFC -d
clean:
	@rm RPiFC
	@echo "Cleaned"
		
	
#Installations
install-wiringpi:
    @echo "Installing wiringpi"
	git clone git://git.drogon.net/wiringPi
	cd wiringPi&&./build
    @echo "Done"