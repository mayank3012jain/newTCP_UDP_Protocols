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
#include <time.h>
#include "packet.h"


#define PORT_NUMBER 12347
#define WAIT_TIME_SEC 2
#define WAIT_TIME_MS 0

void generatePkt(packet* pkt, int isData, int channel, FILE* fptr){
    pkt->channel= channel;
    pkt->isData=isData;
    pkt->seq = ftell(fptr);
    pkt->isLast=0;
    char msg[PACKET_SIZE];
    int offset = 0;
    while(!feof(fptr)){
        msg[offset] = getc(fptr);
        if(offset==PACKET_SIZE-1){
            msg[offset+1]='\0';
            pkt->size= PACKET_SIZE;
            break;
        }
        offset++;
    }
    if(feof(fptr)){
        msg[offset-1]='\0';
        pkt->isLast=1;
        pkt->size = offset-1;
    }
    strcpy(pkt->data, msg);

    return;
}



int main(int argc, char* argv[]){

    FILE *fptr1 = fopen("input.txt", "r");
    // FILE *fptr2 = fopen("read2.txt", "r");
    int stop=0;
    packet pkt[2];
    time_t sendTime[2];

    packet* rcvdPkt = (packet*)malloc(sizeof(packet));
    /* CREATE A TCP SOCKET*/
    int sock[2];
    sock[0] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock[0] < 0) { printf ("Error in opening a socket"); exit (0);}
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

    int c1 = connect (sock[0], (struct sockaddr*) &serverAddr, sizeof(serverAddr));
    printf ("%d\n",c1);
    if (c1 < 0)
        { printf ("Error while establishing connection");
        exit (0);
        }
    printf ("Connection 1 Established\n");

    sock[1] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock[1] < 0) { printf ("Error in opening a socket2"); exit (0);}
    printf("Client Socket2 Created\n");

    c1 = connect (sock[1], (struct sockaddr*) &serverAddr, sizeof(serverAddr));
    printf ("%d\n",c1);
    if (c1 < 0)
        { printf ("Error while establishing connection");
        exit (0);
        }
    printf ("Connection 2 Established\n");

    /*Send initial msgs*/

    generatePkt(&pkt[0], 1, 1, fptr1);
    // printf("The msg was --%s--\n", pkt[0].data);
    int bytesSent = send(sock[0], &pkt[0], sizeof(pkt[0]), 0);
    sendTime[0] = time(NULL);
    if (bytesSent != sizeof(pkt[0]))
        { printf("Error while sending the message");
        exit(0);
        }
    fprintf (stdout, "SENT PKT: Seq. No %d of size %d Bytes from channel %d\n", pkt[0].seq, pkt[0].size, pkt[0].channel);


    // strcpy(msg, "Hi, I'm C2!!");
    generatePkt(&pkt[1], 1, 2, fptr1);
    // printf("The msg was --%s--\n", pkt[1].data);
    bytesSent = send(sock[1], &pkt[1], sizeof(pkt[1]), 0);
    sendTime[1] = time(NULL);
    if (bytesSent != sizeof(pkt[1]))
        { printf("Error while sending the message");
        exit(0);
        }
    fprintf (stdout, "SENT PKT: Seq. No %d of size %d Bytes from channel %d\n", pkt[1].seq, pkt[1].size, pkt[1].channel);

    fd_set rset;
    FD_ZERO(&rset);
    int max_fd;
    max_fd = sock[1]>sock[0] ? sock[1]:sock[0];

    struct timeval tv;
    
    int d=1;
    while(1){
              
        FD_ZERO(&rset);
        FD_SET(sock[0], &rset);   
        FD_SET(sock[1], &rset);  
        tv.tv_sec = WAIT_TIME_SEC;
        tv.tv_usec = WAIT_TIME_MS;
        printf("Waiting for acks\n");

        //blocking rcv on both
        int activity = select(max_fd +1, &rset, NULL, NULL, &tv);
        // printf("\nOut of select\n");
        time_t curTime= time(NULL);
        double delT[2];
        delT[0] = ((double) (curTime - sendTime[0]));
        delT[1] = ((double) (curTime - sendTime[1]));
        if(0==activity || delT[0]>=2 || delT[1]>=2){
            printf("\nTIMEOUT\n");
            for(int i=0; i<2; i++){
                // printf("\ndeltime: %d %lf ",i, delT[i]);
                if(delT[i]>=2){
                    int bytesSent = send (sock[i], &pkt[i], sizeof(pkt[i]), 0);
                    sendTime[i] = time(NULL);
                    if (bytesSent != sizeof(pkt[i]))
                        { printf("Error while sending the message");
                        exit(0);
                        }
                    fprintf (stdout, "SENT PKT: Seq. No %d of size %d Bytes from channel %d\n", pkt[i].seq, pkt[i].size, pkt[i].channel);
                }
            }
        }
        if ((activity < 0) && (errno!=EINTR)){   
            printf("select error");  
            exit(0); 
        }  

        //if ack recieved from sock1
        for(int i=0; i<2; i++){
            if(FD_ISSET(sock[i], &rset)){
                /*RECEIVE BYTES*/
                // char recvBuffer[PACKET_SIZE];
                int bytesRecvd = recv (sock[i], rcvdPkt, sizeof(packet), 0);
                if (bytesRecvd < 0)
                    { printf ("Error while receiving data from server");
                    exit (0);
                    }
                // recvBuffer[bytesRecvd] = '\0';
                fprintf (stdout, "RCVD PKT: Seq. No %d of size %d Bytes from channel %d\n", rcvdPkt->seq, rcvdPkt->size, rcvdPkt->channel);
                // printf ("sock %d: %d: %s\n",i, rcvdPkt->seq, rcvdPkt->data);

                if(feof(fptr1)){
                    // printf("\n%d in end", i);
                    if(stop ==1){
                        printf("\nFile 1 transfered. :)");
                        close(sock[0]);
                        close(sock[1]);
                        fclose(fptr1);
                        // fclose(fptr2);
                        return 0;
                    }
                    stop=1;
                    continue;
                }

                // printf ("ENTER MESSAGE FOR SERVER sock\n");
                
                generatePkt(&pkt[i], 1, i, fptr1);
                // printf("The msg was --%s--\n", pkt[i].data);
                int bytesSent = send (sock[i], &pkt[i], sizeof(pkt[i]), 0);
                sendTime[i] = time(NULL);
                if (bytesSent != sizeof(pkt[i]))
                    { printf("Error while sending the message");
                    exit(0);
                    }
                fprintf (stdout, "SENT PKT: Seq. No %d of size %d Bytes from channel %d\n", pkt[i].seq, pkt[i].size, pkt[i].channel);
                // printf ("Data Sent on channel %d\n", i);
                break;
                
            }
        }
        
    }
    close(sock[0]);
    close(sock[1]);
    fclose(fptr1);
    // fclose(fptr2);
}