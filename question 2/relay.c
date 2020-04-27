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

void die(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char* argv[]){
    int s, count=0, drop = 100/PDR, relayNumber = argv[1][0]-'0';
    packet pkt;

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    struct sockaddr_in server_addr, rcv_addr, client_addr, me_addr;
    int rcvLen, sLen = sizeof(server_addr);
    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_SERVER);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    memset((char *) &me_addr, 0, sizeof(server_addr));
    me_addr.sin_family = AF_INET;
    //check if working correctly
    if(argv[1][0]=='1'){    printf("relay 1\n"); me_addr.sin_port = htons(PORT_RELAY1);}
    if(argv[1][0]=='2'){    printf("relay 2\n");me_addr.sin_port = htons(PORT_RELAY2);}
    me_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //bind
    if( bind(s , (struct sockaddr*)&me_addr, sizeof(me_addr) ) == -1)
    {
        die("bind");
    }

    struct timeval tv;
    int activity;
    fd_set rset;
    FD_ZERO(&rset);
    //store client address
    if (recvfrom(s, &pkt, sizeof(pkt), 0, (struct sockaddr *) &client_addr, &sLen) == -1)
    {
            die("recvfrom()");
    }
    count++;
    printf("Pkt recieved from server:%d for seq num %d \n", pkt.isData-1, pkt.seq);
    // printf("Received packet from %s:%d\n", inet_ntoa(server_addr.sin_addr),ntohs(client_addr.sin_port));
    tv.tv_usec = ((rand()%2)+1) *1000;
    tv.tv_sec = 0;
    printf("Delay of %d ms introduced", (int)tv.tv_usec/1000);
    fflush(stdout);
    activity = select(0, &rset, NULL, NULL, &tv);
    
    if (sendto(s, &pkt, sizeof(pkt), 0 , (struct sockaddr *) &server_addr, sLen)==-1)
    {
        die("sendto()");
    }
    printf("Pkt sent to server:%d for seq num %d \n", pkt.isData , pkt.seq);

    int rcv_bytes;
    while(1){
        rcv_bytes = recvfrom(s, &pkt, sizeof(pkt), 0, (struct sockaddr *) &rcv_addr, &sLen);
        if (rcv_bytes == -1){
            die("recvfrom()");
        }else if(rcv_bytes==0){
            sendto(s, &pkt, 0, 0 , (struct sockaddr *) &server_addr, sLen);
            printf("Closing relay.");
            break;
        }
        printf("Pkt recieved from server:%d for seq num %d \n", 1- pkt.isData , pkt.seq);
        printf("Received packet from %s:%d\n", inet_ntoa(rcv_addr.sin_addr),ntohs(rcv_addr.sin_port));
        //send to server
        if(pkt.isData==1){
            count++;
            if(count% drop ==0){
                printf("Relay %d  D  time  DATA  %d  client relay\n", relayNumber, pkt.seq);
                continue;
            }
            tv.tv_usec = (rand()%3) *1000;
            printf("Delay of %d ms introduced", (int)tv.tv_usec/1000);
            activity = select(0, &rset, NULL, NULL, &tv);
            if (sendto(s, &pkt, sizeof(pkt), 0 , (struct sockaddr *) &server_addr, sLen)==-1)
            {
                die("sendto()");
            }
            printf("Pkt sent to server for seq num %d \n", pkt.seq);
        }
        //send to client
        else{
            if (sendto(s, &pkt, sizeof(pkt), 0 , (struct sockaddr *) &client_addr, sLen)==-1)
            {
                die("sendto()");
            }
            printf("Pkt sent to client for seq num %d \n", pkt.seq);
        }
    }
    close(s);

}