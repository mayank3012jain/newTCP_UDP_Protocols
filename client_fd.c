#include <stdio.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions
#include <arpa/inet.h>// different address structures are declared here
#include <stdlib.h>// atoi() which convert string to integer
#include <string.h>
#include <unistd.h> // close() function
#include<sys/select.h>
#include <errno.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <sys/time.h>
#include <netdb.h>
#include "packetDef.h"


#define PORT_NUMBER 12347

void generatePkt(packet* pkt, int isData, int channel, FILE* fptr){
    // packet* pkt = (packet*)malloc(sizeof(packet));
    pkt->channel= channel;
    pkt->isData=isData;
    pkt->seq = ftell(fptr);
    pkt->isLast=0;
    char msg[PACKET_SIZE];
    int offset = 0;
    while(!feof(fptr)){
        msg[offset] = getc(fptr);
        if(offset==PACKET_SIZE-2){
            msg[offset+1]='\0';
            pkt->size= PACKET_SIZE;
            break;
        }
        offset++;
    }
    if(feof(fptr)){
        msg[offset-1]='\0';
        pkt->isLast=1;
        pkt->size = offset;
    }
    strcpy(pkt->data, msg);

    return;
}



int main(){

    FILE *fptr1 = fopen("read1.txt", "r");
    FILE *fptr2 = fopen("read2.txt", "r");
    int stop=0;
    packet pkt1, pkt2;
    packet* rcvdPkt = (packet*)malloc(sizeof(packet));
    /* CREATE A TCP SOCKET*/
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) { printf ("Error in opening a socket"); exit (0);}
    printf ("Client Socket1 Created\n");

    /*CONSTRUCT SERVER ADDRESS STRUCTURE*/
    struct sockaddr_in serverAddr;
    memset (&serverAddr,0,sizeof(serverAddr));/*memset() is used to fill a block of memory with a particular value*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_NUMBER); //You can change port number here
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Specify server's IP address here
    printf ("Address assigned\n");


    char sendBuff[PACKET_SIZE];
    memset(sendBuff, '0', sizeof(sendBuff));

    int c1 = connect (sock, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
    printf ("%d\n",c1);
    if (c1 < 0)
        { printf ("Error while establishing connection");
        exit (0);
        }
    printf ("Connection 1 Established\n");

    int sock2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock2 < 0) { printf ("Error in opening a socket2"); exit (0);}
    printf("Client Socket2 Created\n");

    c1 = connect (sock2, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
    printf ("%d\n",c1);
    if (c1 < 0)
        { printf ("Error while establishing connection");
        exit (0);
        }
    printf ("Connection 2 Established\n");

    /*Send initial msgs*/
    // char msg[PACKET_SIZE];
    // strcpy(msg, "Hi, I'm C1!!");
    generatePkt(&pkt1, 1, 1, fptr1);
    printf("The msg was --%s--\n", pkt1.data);
    int bytesSent = send(sock, &pkt1, sizeof(pkt1), 0);
    if (bytesSent != sizeof(pkt1))
        { printf("Error while sending the message");
        exit(0);
        }
    printf ("Initial C1 Data Sent\n");

    // strcpy(msg, "Hi, I'm C2!!");
    generatePkt(&pkt2, 1, 2, fptr1);
    printf("The msg was --%s--\n", pkt2.data);
    bytesSent = send(sock2, &pkt2, sizeof(pkt2), 0);
    if (bytesSent != sizeof(pkt2))
        { printf("Error while sending the message");
        exit(0);
        }
    printf ("Initial C2 Data Sent\n");

    fd_set rset;
    FD_ZERO(&rset);
    int max_fd;
    max_fd = sock2>sock ? sock2:sock;
    
    int d=1;
    while(1){
        printf("enter 0 or 1");
        // scanf("%d", &d);
        // if(d==0){
        //     break;
        // }
        
        FD_ZERO(&rset);
        FD_SET(sock, &rset);   
        FD_SET(sock2, &rset);  
        printf("Waiting for acks\n");

        //blocking rcv on both
        int activity = select(max_fd +1, &rset, NULL, NULL, NULL);
        printf("\nOut of select\n");
        if ((activity < 0) && (errno!=EINTR)){   
            printf("select error");  
            exit(0); 
        }  

        //if ack recieved from sock1
        if(FD_ISSET(sock, &rset)){
            /*RECEIVE BYTES*/
            // char recvBuffer[PACKET_SIZE];
            int bytesRecvd = recv (sock, rcvdPkt, sizeof(packet), 0);
            if (bytesRecvd < 0)
                { printf ("Error while receiving data from server");
                exit (0);
                }
            // recvBuffer[bytesRecvd] = '\0';
            printf ("sock: %d: %s\n", rcvdPkt->seq, rcvdPkt->data);

            if(feof(fptr1)){
                
                if(stop ==1){
                    printf("\nFile 1 transfered. :)");
                    break;
                }
                stop=1;
                continue;
            }

            printf ("ENTER MESSAGE FOR SERVER sock\n");
            
            generatePkt(&pkt1, 1, 1, fptr1);
            printf("The msg was --%s--\n", pkt1.data);
            int bytesSent = send (sock, &pkt1, sizeof(pkt1), 0);
            if (bytesSent != sizeof(pkt1))
                { printf("Error while sending the message");
                exit(0);
                }
            printf ("Data Sent on channel 1\n");

            
        }
        
        //if msg recieved from client2
        else if(FD_ISSET(sock2, &rset)){
            /*RECEIVE BYTES*/
            char recvBuffer[PACKET_SIZE];
            int bytesRecvd = recv (sock2, rcvdPkt, sizeof(packet), 0);
            if (bytesRecvd < 0)
                { printf ("Error while receiving data from server");
                exit (0);
                }
            // recvBuffer[bytesRecvd] = '\0';
            printf ("sock2 %d: %s\n",rcvdPkt->seq, rcvdPkt->data);

            if(feof(fptr1)){
                
                if(stop ==1){
                    printf("\nFile 1 transfered. :)");
                    break;
                }
                stop=1;
                continue;
            }

            printf ("ENTER MESSAGE FOR SERVER with max 32 characters\n");
                        
            generatePkt(&pkt2,1, 2, fptr1);
            printf("The msg was --%s--\n", pkt2.data);
            bytesSent = send(sock2, &pkt2, sizeof(pkt2), 0);
            if (bytesSent != sizeof(pkt2))
                { printf("Error while sending the message");
                exit(0);
                }
            printf ("Data Sent on channel 2\n");
            
            
        }
    }
    close(sock);
    close(sock2);
    fclose(fptr1);
    fclose(fptr2);
}