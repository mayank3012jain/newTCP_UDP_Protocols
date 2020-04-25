#ifndef packetDef_h
#define packetDef_h

#define PACKET_SIZE 32

typedef struct packet{
    int size;
    int seq;
    int isLast;
    int isData;
    
    char data[PACKET_SIZE];
    int channel;
}packet;



#endif