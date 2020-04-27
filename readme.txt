EXECUTION
*****************************

Compile the server.c and client.c files.
(2 executable files are also provided (client and server))
--------------------
Create a "input.txt" file in the directory of client.
Use the following 2 commands to run both on different terminals.
Note- execute server before client
./server
./client
--------------------
input.txt will be copied to output.txt
trace will be given on console.


IMPLEMENTATION DETAILS
*******************************

PACKET_SIZE = 100 is defined in packet.h
PDR = 10 is defined in server.c

Used select() function for managing 2 simultaneous connections. 
    -Initially a packet is sent on both the channels.
    -Whenever ack is recieved on a channel the next packet is sent on it, unless end of file. 

Used timer in select() and comparison with send time for retransmission.
    -Whenever an ack is recieved, the programs also chceks the time elapsed since a packet was sent on the other channel. If req, pakcet is retransmitted on it.
    -If no ack is recieved on both the channels the select() times out and both are retransmitted.

When the client closes, the server automatically closes its sockets.

Out of order msgs are handled using a buffer at server.
    -The server buffers any out of order packet inorder of seq.
    -When the req packet is recieved all the inorder packets in buffer are written on the file (until there is any other hole).
