#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(){

    int soc = socket(AF_INET, SOCK_STREAM, 0);

    if(soc == -1) {
        printf("failed to create the socket\n");
    }
    else{
        printf("socket creatted succesfully\n");
    }
    printf("thie socket descriptor is : %d\n", soc); 



    struct sockaddr_in address;
    struct sockaddr_in *pAdress = &address;

    memset(pAdress, 0 , sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    struct sockaddr_in client_address;



    if(bind(soc, (struct sockaddr *) &address, sizeof(address)) == -1)  printf("biding failed\n");
    else printf("binding succesful\n");


    //if(connect(soc, (struct sockaddr*) pAdress, sizeof(address)) == -1) printf("connection failed\n");
    //else printf("connection establisehed");

    

    if(listen(soc, 10) == -1) printf("something wrong in the listeniing \n");
    else printf("listening ... \n");

  socklen_t socket_len = sizeof(address);

  char buffer[4096];

  const char *message =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 5\r\n"
    "Connection: close\r\n"
    "\r\n"
    "hello";
  



    while(1){

        int client_soc = accept(soc, (struct sockaddr *) pAdress, &socket_len);

        if(client_soc == -1){

            printf("connnection failed");
            continue;
        } 
        

        printf("connection established\n");
        printf("the socket of this new client is : %d\n", client_soc);
        printf("socket_len is : %d\n", socket_len);

        int bytes_read =   recv(client_soc, buffer, 4095, 0);
        buffer[bytes_read] = '\0';

        printf("%s\n" ,buffer);
        send(client_soc, message, strlen(message),0);

        close(client_soc);

    }
    
}

