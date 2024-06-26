# Measuring latency for data transmission 

This is a pair of test programs for unicast or multicast data transmission.  The server can send a frame at a given rate. Each frame is time-stamped. The destination can be either a multicast or single host.

The client listens either on the multicast address or all its unicast addresses and prints the received timestamp as well as the sender timestamp from the packet.

A simple AWK script computes the difference, which can then be plotted over time. 


## Build the programs

On both machines to be used for the test, either repeat the procedure below, or copy the resulting directory from one machine to the second: 

1. clone the repository and change working directory:

        git clone https://github.com/mherrb/multicast
        cd multicast
	
1. configure and build the project:

        autoreconf -iv
	    ./configure
	    make
	

On the "server" machine, start `mcasts` on an arbitrarily chosen multicast address and port, with some payload (here the message "Test 1 2 3" is sent every 50ms to `224.4.3.2` port 3000):

    ./mcasts -i 50 224.4.3.2 3000 "Test 1 2 3"
	
On the "client" machine, start `mcastc` to listen on the same address and port as above. Pipe the output to the AWK script to compute the differences in transit time and save the result: 

    ./castc -t 120 224.4.3.2 3000 | awk -f analyse.awk > data.txt
	
This will let it run for 2mn (120s). Then plot the results:

	gnuplot -p gnuplot.cmd

The result should be 2 horizontal lines separated by the transit time of the packets (Expected to be constant). If there are spikes or other irregularities, then the network has some latency or retention issues. 

 
	
