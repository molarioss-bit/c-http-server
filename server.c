#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define PORT 8081
#define MAX_CLIENTS 5

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
    server_address.sin_port = htons(PORT);
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

int readline(char* buffer, char* line, int line_n, size_t max_line_size){

    char* current_buffer = buffer;
    for(int i = 1; i <= line_n; i++){

        char* end_of_line = strstr(current_buffer, "\r\n");
        if(end_of_line == NULL)  return -1;
        
        size_t n = end_of_line - current_buffer;

        if(i == line_n){

            if(n == 0) return 0;
            if(n >= max_line_size) return -1;
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

    if(readline(buffer, line, 1, sizeof(line)) == -1){
        printf("here where the error\n");
        return -1;
    }   
    if(split_line(line, r->methode, sizeof(r->methode), left, sizeof(left), ' ') == -1) return -1;
    if(split_line(left, r->path, sizeof(r->path), value, sizeof(value), ' ') == -1) return -1;

    r->host[0] = '\0';
    
    int result;

    for(int i = 2; (result= readline(buffer, line, i, sizeof(line))) == 1; i++){

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
int check_path(char *path){

    char* sus = strstr(path, "..");

    if(strcmp(path, "/") == 0) return -1;
    else if(sus == NULL) return 0;
    else return -1;
}
int read_file(char* path, char* file_content , size_t file_bytes) {

    FILE *f = fopen(path, "rb"); 
    if(f ==NULL) {perror("fopen"); return -1;}
    size_t n = fread(file_content, 1, file_bytes, f);
    file_content[n] = '\0';
    fclose(f);
    return n;

}

int status_code(struct request *r, int parse_request_return, char* file_content, size_t file_size_bytes, size_t *file_real_size){

    if(parse_request_return == -1) return 400;
    if(strcmp(r->methode, "GET") != 0) return 405; //unkown methode
    int i = check_path(r->path);  printf("check path return is :%d\n", i);

    if(check_path(r->path) == -1) return 404;
    if(check_path(r->path) != -1) {
        
        //constructing the directory from r.path for read_file to read
        char directory[264];
        snprintf(directory, 264, "www%s", r->path);
        printf("%s\n", r->path);
        printf("%s\n", directory);

        //we are taking file_content and place write on it the content of the directory, readfile return -1 or the file size,
        *file_real_size = read_file(directory, file_content, file_size_bytes);
        if(*file_real_size != -1) return 200;
    }
    return 404;
}

int respond(char* buffer, struct request *r, int parse_request_return){

    char *header_something ;
    size_t file_size_limit = 200000;
    size_t file_real_size;
    char file_content[file_size_limit];
    char *body;
    int header_length;
    int result;
  
    int status_cod = status_code(r, parse_request_return, file_content, file_size_limit, &file_real_size);
    printf("%d and %zu\n", status_cod, file_real_size);


    if(status_cod == 404){
        header_something = "Not Found";
        body = "404 page not found";
    }

    if(status_cod == 405){
        header_something = "Method Not Allowed";
        body = "Error 405 (Method Not Allowed)!!1";
    }
    if(status_cod == 400){
         header_something = "Bad Request";
        body = "Server Error";
    }
    if(status_cod == 200) {
        body = file_content;
        header_something = "OK";
        result = snprintf(buffer,250000, "HTTP/1.1 %d %s\r\nContent-Type: text/html\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s", status_cod, header_something, file_real_size, body);
        return result;
    }
    result = snprintf(buffer,250000, "HTTP/1.1 %d %s\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s", status_cod, header_something, strlen(body), body);
   
    return result;
    
}

 int handle_client(int server_fd, int client_fd, char* buffer_request, struct request *r){
    
    while(1){

        printf("connection established\n");
        printf("the socket of this new client is : %d\n", client_fd);
        ssize_t bytes_read = recv(client_fd, buffer_request, 4095, 0);

        if(bytes_read <= 0){
            perror("recv");
            close(client_fd);
            return -1;
        }

        buffer_request[bytes_read] = '\0';
        memset(r, 0, sizeof(*r));

        int parse_request_return = parse_request(buffer_request, r);
        char buffer_respond[250000];

        respond(buffer_respond, r, parse_request_return);

        if(send(client_fd,buffer_respond , strlen(buffer_respond),0) == -1) {
            perror("send");
            close(client_fd);
            return -1;
        }
        if(close(client_fd) == 0) return 0;
    }
 }
//this function will buld the path, after it make sure there is no payload, now it checks just for normal text "..", the attacker can bypass it by write it in ASCII.
//so we need to decode the ascii then check.


int main(){


    char buffer_request[4096];
    int server_fd = setup_server();

    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int client_fd;

    pid_t pid;
    int active = 0;
    
    
    while(1)
    {
        client_fd = accept(server_fd, (struct sockaddr *) &client_address,  &client_address_size);

        if(client_fd < 0){
            perror("accept");
            continue;
        }

        while(waitpid(-1, NULL, WNOHANG) > 0) active --;
        if(active >= MAX_CLIENTS) {
            printf("max_client amount reached\n");
            char buffer2[1000];
            snprintf(buffer2, 1000, "HTTP/1.1 503 Service Unavailable\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: 42\r\n\r\n503 Service Unavailable: Server overloaded");
            send(client_fd,buffer2 , strlen(buffer2),0);
            close(client_fd);
           continue;
        }
        
        pid = fork();
        
        //jere we are implimenting a limit to the clients that connect to our server
        //I'm trying to defend agains ja ddos attack, so the server dont run out of processes


        if(pid > 0){
            close(client_fd);
            active ++;
        }

        //child procces
        if(pid ==  0){
            
            struct request req;
            int i = handle_client(server_fd, client_fd, buffer_request, &req);
            close(server_fd);
            if(i == 0) exit(0);
            if(i == -1) exit(1);
        }
       
        
    }
}



