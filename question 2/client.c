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

#define windowsize 5


typedef struct listNode
{
    packet pkt;
    int isAcked;
    time_t time;
    struct listNode* next;
}listNode;

typedef struct queue{
    int size;
    listNode* head, *tail;
    
}queue;

void die(char *s)
{
    perror(s);
    exit(1);
}
 
void generatePkt(packet* pkt, int isData, int channel, FILE* fptr){
    int offsetFile = ftell(fptr);
    channel = (offsetFile/PACKET_SIZE)%2;
    pkt->channel= channel;
    pkt->isData=isData;
    pkt->seq = ftell(fptr);
    pkt->isLast=0;
    char msg[PACKET_SIZE+1];
    int offset = 0;
    while(!feof(fptr)){
        msg[offset] = getc(fptr);
        if(offset==PACKET_SIZE-1){
            msg[offset+1]='\0';
            pkt->size= PACKET_SIZE;
            // fseek(fptr, PACKET_SIZE, SEEK_CUR);
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

void pktCopy(packet* dest, packet* src){
    dest->channel=src->channel;
    strcpy(dest->data, src->data);
    dest->isData = src->isData;
    dest->isLast = src->isLast;
    dest->seq = src->seq;
    dest->size = src->size;
    return;
}
void insertQ(packet* pkt, queue* q){
    listNode* temp = malloc(sizeof(listNode));
    temp->isAcked = 0;
    pktCopy(&temp->pkt, pkt);
    temp->time = time(NULL);

    if(q->size==0){
        temp->next=NULL;
        q->head = q->tail = temp;
    }else{
        q->tail->next=temp;
        temp->next = NULL;
        q->tail = temp;
    }
    q->size = q->size+1;
    return;
}

void ackQ(packet* pkt, queue* q){
    listNode* temp = q->head;
    while(temp){
        if(temp->pkt.seq == pkt->seq){
            temp->isAcked=1;
            break;
        }
        temp=temp->next;
    }
    while(q->size>0){
        if(q->head->isAcked==0){
            break;
        }
        listNode* temp = q->head;
        q->head = q->head->next;
        free(temp);
        q->size = q->size-1;
    }
    if(q->size==0){
        q->tail=NULL;
    }
    return;
}

int main(int argc, char* argv[]){
    FILE *fptr = fopen("input.txt", "r");
    // fptr[1] = fopen("input.txt", "r"); fseek(fptr[1], PACKET_SIZE, SEEK_SET);
    queue q;
    packet send_pkt,rcv_ack;

    struct sockaddr_in relay[2], rcv_addr;
    int s, i, slen=sizeof(relay[0]), rcvLen;

    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    //generate 2 relay addresses
    memset((char *) &relay[0], 0, sizeof(relay[0]));
    relay[0].sin_family = AF_INET;
    relay[0].sin_port = htons(PORT_RELAY1);
	relay[0].sin_addr.s_addr = inet_addr("127.0.0.1");

    memset((char *) &relay[1], 0, sizeof(relay[1]));
    relay[1].sin_family = AF_INET;
    relay[1].sin_port = htons(PORT_RELAY2);
	relay[1].sin_addr.s_addr = inet_addr("127.0.0.1");

    //send initial msgs
    while(q.size< windowsize){
        generatePkt(&send_pkt, 1, i, fptr);

        if (sendto(s, &send_pkt, sizeof(send_pkt), 0 , (struct sockaddr *) &relay[send_pkt.channel], slen)==-1)
        {
            die("sendto()");
        }
        insertQ(&send_pkt, &q);
    }
    printf("Initial msgs sent\n");

    while(1){
        if(q.size==0 && feof(fptr)==1){
            break;
        }
        if (recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *) &rcv_addr, &slen) == -1)
        {
                die("recvfrom()");
        }
        printf("Ack recieved from channel %d for seq num %d \n", rcv_ack.channel, rcv_ack.seq);
        ackQ(&rcv_ack, &q);
        // int channel = rcv_ack.channel;
        
        if(q.size<windowsize && feof(fptr)==0){
            generatePkt(&send_pkt, 1, 0, fptr);
            if (sendto(s, &send_pkt, sizeof(send_pkt), 0 , (struct sockaddr *) &relay[send_pkt.channel], slen)==-1)
            {
                die("sendto()");
            }
            insertQ(&send_pkt, &q);
            printf("Msg sent on channel %d for seq num %d \n", send_pkt.channel, send_pkt.seq);
        }
    }
    //close socket
    sendto(s, NULL, 0, 0 , (struct sockaddr *) &relay[0], slen);
    sendto(s, NULL, 0, 0 , (struct sockaddr *) &relay[1], slen);
    close(s);
    fclose(fptr);
}