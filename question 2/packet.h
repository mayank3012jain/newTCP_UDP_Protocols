#ifndef packetDef_h
#define packetDef_h

//no of bytes of payload except \0
#define PACKET_SIZE 10

#define PORT_SERVER 12347
#define PORT_RELAY1 12345
#define PORT_RELAY2 12346
#define PDR 20
#define WAIT_TIME 3

typedef struct packet{
    int size;
    int seq;
    int isLast;
    int isData;
    
    char data[PACKET_SIZE +1];
    int channel;
}packet;



#endif