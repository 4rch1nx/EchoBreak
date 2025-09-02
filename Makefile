# this is a Makefile for the Worker
Worker:
	g++ source/Worker.cpp source/worker_utils.cpp -pthread -o eb.net

clean:
	rm -f eb.net
