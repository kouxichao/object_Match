
MODE = 
CFLAGS += -I$(PWD) \
	-I$(PWD)/../ncnn \
	-I$(PWD)/../sqlite \
	-I$(PWD)/../dlib \
	-L$(PWD)/../lib -lsqlite3 -ldlib -lncnn -ldl
CFLAGS += $(MODE) -O3 -fopenmp -mcpu=cortex-a53+simd -mcpu=cortex-a53+fp 
#SRCS := $(wildcard *.cpp)
SRCS := interface_obj.cpp 

ifeq ($(MODE), -DJPG_DEMO)
all:interface_obj demo_obj
else 
all:interface_obj
endif

interface_obj:
	aarch64-himix100-linux-g++ $(SRCS) $(CFLAGS) -c -o interface_obj.cpp.o 
	ar r libobj.a interface_obj.cpp.o

demo_obj:interface_obj
	aarch64-himix100-linux-g++ demo_obj.cpp  libobj.a $(CFLAGS) -o demo_obj


