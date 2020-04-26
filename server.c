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
#define PDR 10

typedef struct listNode{
    int seq;
    int nextSeq;
    char data[BUFFERSIZE];
    struct listNode* next;
}listNode;

void generatePkt(packet* pkt, packet* datapkt){
    // packet* pkt = (packet*)malloc(sizeof(packet));
    pkt->size = 0;
    pkt->seq = datapkt->seq;
    pkt->isLast = datapkt->isLast;
    pkt->isData= 0;
    pkt->channel= datapkt->channel;
    // strcpy(pkt->data, "ABC");
    return;
}

listNode* bufferedWrite(packet* pkt, int *req, listNode* head, FILE* fptr){
    if(pkt->seq == *req){
        fprintf (fptr, "%s", pkt->data);
        *req = pkt->seq + pkt->size -1;
        while(head){
            if(head->seq != *req){
                break;
            }else{
                fprintf (fptr, "%s", head->data);
                *req = head->nextSeq;
            }
            listNode* temp = head;
            head = head->next;
            free(temp);
        }  
    }else{
        //insert
        listNode* temp = head, *prev = NULL;
        listNode* cur = (listNode*)malloc(sizeof(listNode));
        cur->seq= pkt->seq;
        cur->nextSeq = pkt->seq + pkt->size -1;
        strcpy(cur->data, pkt->data);
        while(temp){
            if(temp->seq>= cur->nextSeq){
                cur->next = temp;
                if(prev){
                    prev->next = cur;
                    return head;
                }else{
                    return cur;
                }
            }
            prev = temp;
            temp = temp->next;
        }
        cur->next = temp;
        if(prev){
            prev->next = cur;
            return head;
        }else{
            return cur;
        }

    }
    return head;
}

int main (){

    FILE* fptr1 = fopen("copy1.txt", "w");
    int f[2] = {0, 0};
    packet rcvdPkt;
    packet ackPkt;

    listNode* bufferHead = NULL;
    int reqSeq =0;

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
    
    int d=1, i=0;
    while(d==1){
        printf("enter 0 or 1");
        // scanf("%d", &d);
        // if(d==0){
        //     break;
        // }
        if(f[0]==1 && f[1]==1){
            fclose(fptr1);
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
        
        i++;
        int drop = 100/PDR;
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
                bufferHead = bufferedWrite(&rcvdPkt, &reqSeq, bufferHead, fptr1);
                fprintf (stdout, "Msg recieved:%d- %s\n", rcvdPkt.seq, rcvdPkt.data);
                // fprintf (fptr1, "%s", rcvdPkt.data);
                generatePkt(&ackPkt, &rcvdPkt);
                if(i%drop != 0){
                    int bytesSent = send(clientSocket[i], &ackPkt, sizeof(ackPkt),0);
                }
                break;
            }
        }
     
    }
    listNode* tempN = bufferHead;
    while (tempN){
        fprintf (fptr1, "%s", tempN->data);
        tempN= tempN->next;
    }
    
    close(serverSocket);
    close(clientSocket[0]);
    close(clientSocket[1]);
    fclose(fptr1);
    return 0;
}