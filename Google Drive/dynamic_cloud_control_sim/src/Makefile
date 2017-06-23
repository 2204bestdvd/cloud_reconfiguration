main: main.o packet.o node.o link.o scheduler.o testbed.o
	clang++ -std=c++11 main.o packet.o node.o link.o scheduler.o testbed.o -o test  
main.o: main.cpp             
	clang++ -std=c++11 main.cpp -c
packet.o: packet.cpp             
	clang++ -std=c++11 packet.cpp -c          
node.o: node.cpp
	clang++ -std=c++11 node.cpp -c
link.o: link.cpp
	clang++ -std=c++11 link.cpp -c
scheduler.o: scheduler.cpp
	clang++ -std=c++11 scheduler.cpp -c          
testbed.o: testbed.cpp
	clang++ -std=c++11 testbed.cpp -c          
clean:                 
	rm -rf main.o packet.o node.o link.o scheduler.o testbed.o
