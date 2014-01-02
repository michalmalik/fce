CC = g++
CFLAGS = -Wall -O2 -g --std=c++11

t1 t2 t3: CFLAGS += -lboost_system

t3: CFLAGS += -lboost_chrono -lboost_thread

all: t1 t2 t3 t4

t1:
	$(CC) $(CFLAGS) t1.cpp -o t1
t2:
	$(CC) $(CFLAGS) t2.cpp -o t2
t3:
	$(CC) $(CFLAGS) t3.cpp -o t3
t4:
	$(CC) $(CFLAGS) t4.cpp -o t4

clean:
	rm t1 t2 t3 t4 
