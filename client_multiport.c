#include <stdio.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions
#include <arpa/inet.h>// different address structures are declared here
#include <stdlib.h>// atoi() which convert string to integer
#include <string.h>
#include <unistd.h> // close() function

#define PACKET_SIZE 32
#define PORT_NUMBER 12347


#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>

int main(){
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

    
    /* Open the file that we wish to transfer */
    // FILE *fp = fopen("source_file.txt","rb");
    // if(fp==NULL)
    // {
    //     printf("File opern error");
    //     return 1;   
    // }

    /* Read data from file and send it */
    // fseek(fp, 0, SEEK_SET);

    while(1)
        {
            // /* First read file in chunks of 256 bytes */
            // unsigned char buff[256]={0};
            // int nread = fread(buff,1,256,fp);
            // printf("Bytes read %d \n", nread);        

            // /* If read was success, send data. */
            // if(nread > 0)
            // {
            //     printf("Sending \n");
            //     write(connfd, buff, nread);
            // }

            // /*
            //  * There is something tricky going on with read .. 
            //  * Either there was error, or we reached end of file.
            //  */
            // if (nread < 256)
            // {
            //     if (feof(fp))
            //         printf("End of file\n");
            //     if (ferror(fp))
            //         printf("Error reading\n");
            //     break;
            // }
            printf ("ENTER MESSAGE FOR SERVER with max 32 characters\n");
            char msg[PACKET_SIZE];
            scanf("%[^\n]%*c", msg);
            printf("The msg was --%s--\n", msg);
            // gets(msg);
            int bytesSent = send (sock, msg, strlen(msg), 0);
            if (bytesSent != strlen(msg))
                { printf("Error while sending the message");
                exit(0);
                }
            printf ("Data Sent\n");

            /*RECEIVE BYTES*/
            char recvBuffer[PACKET_SIZE];
            int bytesRecvd = recv (sock2, recvBuffer, PACKET_SIZE-1, 0);
            if (bytesRecvd < 0)
                { printf ("Error while receiving data from server");
                exit (0);
                }
            recvBuffer[bytesRecvd] = '\0';
            printf ("sock 2 %s\n",recvBuffer);

            
            
            printf ("ENTER MESSAGE FOR SERVER with max 32 characters\n");
            // char msg[PACKET_SIZE];
            scanf("%[^\n]%*c", msg);
            printf("The msg was --%s--\n", msg);
            // gets(msg);
            bytesSent = send(sock2, msg, strlen(msg), 0);
            if (bytesSent != strlen(msg))
                { printf("Error while sending the message");
                exit(0);
                }
            printf ("Data Sent\n");
            
            /*RECEIVE BYTES*/
            // char recvBuffer[PACKET_SIZE];
            bytesRecvd = recv (sock, recvBuffer, PACKET_SIZE-1, 0);
            if (bytesRecvd < 0)
                { printf ("Error while receiving data from server");
                exit (0);
                }
            recvBuffer[bytesRecvd] = '\0';
            printf ("sock1 %s\n",recvBuffer);

        }
        close(sock);
}