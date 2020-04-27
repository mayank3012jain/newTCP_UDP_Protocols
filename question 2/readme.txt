EXECUTION
*****************************

Compile the server.c, relay.c and client.c files.
(3 executable files are also provided (client, relay and server))
--------------------
Create a "input.txt" file in the directory of client.
Use the following 4 commands to run both on different terminals. 
(There is a single .c file for relay which needs to be run in 2 different terminals with commandline arguments 1 or 2)
Note- execute in the same order
./server
./relay 1
./relay 2
./client
--------------------
input.txt will be copied to output.txt
trace will be given in log.txt
(since there is no output on console it may look the program is stuck but it sometimes takes time due to timeout. 
The program will exit itself on all 4 terminals simultaneously after some time in normal conditions.)


IMPLEMENTATION DETAILS
*******************************

PACKET_SIZE = 100 is defined in packet.h
PDR = 10 is defined in packet.h
WAIT_TIME = 3 sec defined in packet.h
windowsize = 5 defined in client.c

Basic working and flow
    - Initially total n=5 (=windowsize) packets are sent from the client to both the relays according to odd or even.
    - When an ack is recieved, the client checks if it is for the first packet in the window.
        -If for first pkt, it slides the window until the first packet is unacked.
        -If for any other packet it stores that the pkt is acked.
    -Packets are sent until number of pkts from first unacked becomes n=5 (=windowsize). 
    -When no ack is recieved for WAIT_TIME secs, the first packet is retransmitted and timer restarted.


Used timer in select() in client for retransmission.
    -If no ack is recieved the select() times out and the first packet is retransmitted.

When the client closes, the server and relays automatically close their sockets and exit.

Out of order msgs are handled using a buffer at server.
    -The server buffers any out of order packet inorder of seq.
    -When the req packet is recieved all the inorder packets in buffer are written on the file (until there is any other hole).

Same packet is used as in Q1 to make it universal.
