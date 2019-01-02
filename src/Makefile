
MODE = 
CC = aarch64-himix100-linux-g++
db = 
CFLAGS += -I$(PWD) \
	-I$(PWD)/../ncnn \
	-I$(PWD)/../sqlite \
	-I$(PWD)/../dlib \
	-L$(PWD)/../lib -lsqlite3 -ldlib -lncnn -ldl 
CFLAGS += $(MODE) $(db) -O3 -fopenmp  -std=c++11 -mcpu=cortex-a53+simd -mcpu=cortex-a53+fp
#SRCS := $(wildcard *.cpp)
SRCS := interface_obj.cpp 

ifeq ($(MODE), -DJPG_DEMO)
all:interface_obj demo_obj evaluate
else 
all:interface_obj
endif

interface_obj:
	$(CC) -c -o interface_obj.cpp.o $(SRCS) $(CFLAGS)  
	ar r libobj.a interface_obj.cpp.o

demo_obj:interface_obj
	$(CC) demo_obj.cpp  libobj.a $(CFLAGS) -o demo_obj

evaluate:interface_obj
	$(CC)  evaluate.cpp    libobj.a $(CFLAGS) -o evaluate
