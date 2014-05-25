CXX= g++
CFLAGS= -Wall -g -std=c++11
LDFLAGS= `pkg-config --libs --cflags opencv` -lm -lasound -lpthread

all: wind_waker

wind_waker: wind_waker.o k_cam.o k_sound.o 
	$(CXX) $(LDFLAGS) $< k_cam.o k_sound.o -o $@
 
clean:
	rm -f *.o wind_waker

info:
	pkg-config --libs --cflags opencv
