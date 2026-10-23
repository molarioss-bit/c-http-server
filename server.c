#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>





const char *request =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 5\r\n"
    "Connection: close\r\n"
    "hello\r\n"
    "\r\n";

struct request{

    char methode[16];
    char path[256];
    char host[256];
};

int setup_server(){
//server_socket
    struct sockaddr_in server_address;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1) {
        perror("socket");   
        exit(1);
    }
    memset(&server_address, 0 , sizeof(server_address));
    //server_ipv4_socket parameters,  0.0.0.0:8080
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt failed");
    }

    //binding
        if(bind(listen_fd, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        perror("bind"); 
        exit(1); 
    }
    //lisening
    if(listen(listen_fd, 10) == -1) {
        perror("listen");
        exit(1);
    }
    else printf("listening ... \n");
    return listen_fd;
}

 int handle_client(int server_fd, int client_fd, const char* request, char* buffer){
    
    while(1){

        printf("connection established\n");
        printf("the socket of this new client is : %d\n", client_fd);
        size_t bytes_read = recv(client_fd, buffer, 4095, 0);

        if(bytes_read <= 0){
            perror("recv");
            close(client_fd);
            return -1;
        }

        buffer[bytes_read] = '\0';
        if(send(client_fd, request, strlen(request),0) == -1) perror("send");

        close(client_fd);
        break;
    }
 }
        
int readline(char* buffer, char* line, int line_n){

    char* current_buffer = buffer;
    for(int i = 1; i <= line_n; i++){

        char* end_of_line = strstr(current_buffer, "\r\n");

        if(end_of_line == NULL)  return -1;
        
        size_t n = end_of_line - current_buffer;

        if(i == line_n){

            if(n == 0){
            return 0;
            } 
            memcpy(line, current_buffer, n);
            line[n] = '\0';
            return 1;
        }
        current_buffer = end_of_line + 2;
    }
}

 int split_line(const char* line, char* left, size_t left_size, char* right, size_t right_size, char delimiter ){
    const char* delimiter_pointer = strchr(line, delimiter);
    if(strchr(line, delimiter) == NULL){
        printf("something wrong with the delimeter\n");
        return -1;
    }
    size_t left_real_size = delimiter_pointer - line;
    if(left_real_size + 1 > left_size){
        printf("left buffer is smalle\n"); return -1;
    }else{
        memcpy(left, line, left_real_size);
        left[left_real_size] = '\0';
    }
    size_t end_of_the_line = strlen(line);
    size_t right_real_size = end_of_the_line -1 - left_real_size;

    if(right_real_size + 1 > right_size){
        printf("right buffer is smalle\n"); return -1;
    }else{
        memcpy(right, line+1+left_real_size, right_real_size);
        right[right_real_size] = '\0';
    }
    return 0;
}

int parse_request(char* buffer, struct request *r){

    char line[1000];
    char name[256];
    char value[256];
    char left[256];

    if(readline(buffer, line, 1) == -1) return -1;
    if(split_line(line, r->methode, sizeof(r->methode), left, sizeof(left), ' ') == -1) return -1;
    if(split_line(left, r->path, sizeof(r->path), value, sizeof(value), ' ') == -1) return -1;

    r->host[0] = '\0';
    
    int result;

    for(int i = 2; (result= readline(buffer, line, i)) == 1; i++){

        if(split_line(line, name, sizeof(name), value, sizeof(value), ':') == -1) return -1;

        //some request can have a tab instead of a normal space, and tab = \t in ascii
        char *v = value;
        while(*v == ' ' || *v == '\t') v++;
        
        if (strcasecmp(name, "Host") == 0) {
            if (strlen(v) + 1 > sizeof(r->host)) return -1;
            strcpy(r->host, v);            
        }
 
    }
    if(result != 0) return -1;
    return 0;
}

int main(){

    char buffer[4096];

    int server_fd = setup_server();

    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int client_fd;


    
    while(1)
    {
        client_fd = accept(server_fd, (struct sockaddr *) &client_address,  &client_address_size);
        if(client_fd == -1){
            printf("connnection failed");
            continue;
        }
        struct request req;
        handle_client(server_fd, client_fd, request, buffer);


        char line[1000];

        struct request *r = &req;

        parse_request(buffer, r);

        printf("%s %s \n", r->methode, r->host);
   
    }

}

