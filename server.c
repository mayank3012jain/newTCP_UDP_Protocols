#include "packetDef.h"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>
#include<sys/select.h>
#include <errno.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <sys/time.h>

#define MAXPENDING 5
#define BUFFERSIZE 1024

packet* generatePkt(packet* datapkt){
    packet* pkt = (packet*)malloc(sizeof(packet));
    pkt->size = 0;
    pkt->seq = datapkt->seq;
    pkt->isLast = datapkt->isLast;
    pkt->isData= 0;
    pkt->channel= datapkt->channel;
    // strcpy(pkt->data, "ABC");
    return pkt;
}


int main (){

    FILE* fptr1 = fopen("copy1.txt", "w");
    // FILE* fptr2 = fopen("copy2.txt", "w");
    int f[2] = {0, 0};
    // packet *rcvdPkt = (packet*)malloc(sizeof(packet));
    packet rcvdPkt;
    packet *ackPkt = (packet*)malloc(sizeof(packet));

    fd_set rset;
    FD_ZERO(&rset);
    int max_fd;

    /*CREATE A TCP SOCKET*/
    int serverSocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0){
        printf ("Error while server socket creation");
        exit (0);
        }
    printf ("Server Socket Created\n");
    int yes = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) <0) {
        perror("setsockopt");
        exit(1);
    }
    
    /*CONSTRUCT LOCAL ADDRESS STRUCTURE*/
    struct sockaddr_in serverAddress, clientAddress, clientAddress2;
    memset (&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12347);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    printf ("Server address assigned\n");
    

    int temp = bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    if (temp < 0){
        
        close(serverSocket);
        printf ("Error while binding\n");
        exit (0);
        }
    printf ("Binding successful\n");

    int temp1 = listen(serverSocket, MAXPENDING);
    if (temp1 < 0)
        { printf ("Error in listen");
        exit (0);
        }
    printf ("Now Listening\n");
    char msg[BUFFERSIZE];
    char msg2[BUFFERSIZE];
    printf ("Waiting for Client %s\n", inet_ntoa(clientAddress.sin_addr));

    //clear the socket set  
    FD_ZERO(&rset);   
    
    /*CONNECT TO BOTH CLIENT*/
    int clientLength = sizeof(clientAddress);
    int clientSocket[2];
    clientSocket[0] = clientSocket[1] = 0;
    clientSocket[0] = accept(serverSocket, (struct sockaddr*) &clientAddress, &clientLength);
    if (clientSocket[0] < 0) {printf ("Error in client socket1"); exit(0);}
    printf ("Handling Client %s\n", inet_ntoa(clientAddress.sin_addr));
    clientSocket[1] = accept(serverSocket, (struct sockaddr*) &clientAddress2, &clientLength);
    if (clientSocket[1] < 0) {printf ("Error in client socket2"); exit(0);}
    printf ("Handling Client %s\n", inet_ntoa(clientAddress2.sin_addr));
    
    /*INSERT BOTH CLIENT SOCKETS INTO FD SET*/
    FD_SET(clientSocket[0], &rset);   
    FD_SET(clientSocket[1], &rset);   
    max_fd = clientSocket[0]>=clientSocket[1] ? clientSocket[0]: clientSocket[1];
    printf("----%d--%d--%d", max_fd, clientSocket[0], clientSocket[1]);
    
    int d=1;
    while(d==1){
        printf("enter 0 or 1");
        // scanf("%d", &d);
        // if(d==0){
        //     break;
        // }
        if(f[0]==1 && f[1]==1){
            fclose(fptr1);
            // fclose(fptr2);
            return 0;
        }
        FD_ZERO(&rset);
        if(f[0]==0)FD_SET(clientSocket[0], &rset);   
        if(f[1]==0)FD_SET(clientSocket[1], &rset);  
        printf("Waiting for client\n");

        //blocking rcv on both
        int activity = select(max_fd +1, &rset, NULL, NULL, NULL);
        printf("\nOut of select\n");
        if ((activity < 0) && (errno!=EINTR)){   
            printf("select error");  
            exit(0); 
        }  

        //if msg recieved from client1
        for(int i=0; i<2; i++){
            if(FD_ISSET(clientSocket[i], &rset)){
                int temp2 = recv(clientSocket[i], &rcvdPkt, sizeof(rcvdPkt), 0);
                if (temp2 < 0)
                    { printf ("problem in reading from client %d", i);
                    exit (0);
                }else if(temp2==0){
                    printf("Closing channel %d.", i);
                    close(clientSocket[i]);
                    f[i] = 1;
                    continue;
                }
                msg[temp2] = '\0';
                fprintf (stdout, "Msg recieved:%d- %s\n", rcvdPkt.seq, rcvdPkt.data);
                fprintf (fptr1, "%s", rcvdPkt.data);
                ackPkt = generatePkt(&rcvdPkt);
                int bytesSent = send(clientSocket[i], ackPkt, sizeof(ackPkt),0);

            }
        }
        //if msg recieved from client2
        // else if(FD_ISSET(clientSocket[1], &rset)){                
        //     int temp3 = recv(clientSocket[1], rcvdPkt, sizeof(rcvdPkt), 0);
        //     if (temp3 < 0)
        //         { printf ("problem in temp 3");
        //         exit (0);
        //     }else if(temp3==0){
        //         printf("Closing channel 2.");
        //         close(clientSocket[1]);
        //         f2 = 1;
        //         continue;
        //     }
        //     msg2[temp3] = '\0';
        //     fprintf (fptr2, "%s\n", msg2);
        //     strcpy(msg2, "ACK");
        //     int bytesSent = send(clientSocket[1],msg2,strlen(msg2),0);
            
        // }
        memset(msg, 0, BUFFERSIZE);
        memset(msg2, 0, BUFFERSIZE);

        
    }
    close(serverSocket);
    close(clientSocket[0]);
    close(clientSocket[1]);
    fclose(fptr1);
    // fclose(fptr2);
}