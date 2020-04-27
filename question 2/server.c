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

typedef struct listNode{
    int seq;
    int nextSeq;
    char data[PACKET_SIZE+1];
    struct listNode* next;
}listNode;

void die(char *s)
{
    perror(s);
    exit(1);
}

// void bufferedWrite(packet* pkt, FILE* fptr){
//     fprintf(fptr, "MSG %d- %s", pkt->seq, pkt->data);
// }
listNode* bufferedWrite(packet* pkt, int *req, listNode* head, FILE* fptr){
    if(pkt->seq == *req){
        fprintf (fptr, "MSG %d: %s",pkt->seq, pkt->data);
        *req = pkt->seq + pkt->size;
        while(head){
            if(head->seq != *req){
                break;
            }else{
                fprintf (fptr, "MSG %d:%s", head->seq, head->data);
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
        cur->nextSeq = pkt->seq + pkt->size;
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

int main(){
    FILE* fptr = fopen("output.txt", "w");
    listNode* bufHead = NULL;
    int reqSeq= 0;

    struct sockaddr_in rcv_addr, me_addr;
    packet rcv_pkt, send_pkt;
    int addr_len = sizeof(rcv_addr);

    int sock;

    if ((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    me_addr.sin_family = AF_INET;
    me_addr.sin_port = htons(PORT_SERVER);
    me_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //bind
    if( bind(sock , (struct sockaddr*)&me_addr, sizeof(me_addr) ) == -1)
    {
        die("bind");
    }
    int recv_bytes;
    while(1){
        recv_bytes = recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *) &rcv_addr, &addr_len);
        if (recv_bytes == -1)
        {
                die("recvfrom()");
        }
        else if(recv_bytes ==0){
            printf("\nclosing server.\n");
            break;
        }
        // int channel = rcv_ack.channel;
        bufHead = bufferedWrite(&rcv_pkt, &reqSeq, bufHead, fptr);
        generatePkt(&send_pkt, &rcv_pkt);
        if (sendto(sock, &send_pkt, sizeof(send_pkt), 0 , (struct sockaddr *) &rcv_addr, addr_len)==-1)
        {
            die("sendto()");
        }
    }
    close(sock);
    fclose(fptr);
}
