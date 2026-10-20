#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>



char buffer[4096];

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

void handle_client(int server_fd, const char* request){
    struct sockaddr_in client_address;
    socklen_t client_address_size;
     while(1){
        int client_fd = accept(server_fd, (struct sockaddr *) &client_address,  &client_address_size);
        if(client_fd == -1){
            printf("connnection failed");
            continue;;
        }
        printf("connection established\n");
        printf("the socket of this new client is : %d\n", client_fd);
        int bytes_read = recv(client_fd, buffer, 4095, 0);
        buffer[bytes_read] = '\0';
        send(client_fd, request, strlen(request),0);

        close(client_fd);
        break;

        }
        
    }



int split_line(const char* line, char* left, size_t left_size, char* right, size_t right_size, char delimiter ){
    const char* delimiter_pointer = strchr(line, delimiter);
    if(strchr(line, delimiter) == NULL){
        printf("something wrong with the delimeter");
        return -1;
    }
    size_t left_real_size = delimiter_pointer - line;
    if(left_real_size + 1 > left_size){
        printf("left buffer is smalle"); return -1;
    }else{
        memcpy(left, line, left_real_size);
        left[left_real_size] = '\0';
    }
    size_t end_of_the_line = strlen(line);
    size_t right_real_size = end_of_the_line -1 - left_real_size;

    if(right_real_size + 1 > right_size){
        printf("right buffer is smalle"); return -1;
    }else{
        memcpy(right, line+1+left_real_size, right_real_size);
        right[right_real_size] = '\0';
    }
    return 0;

}

int main(){


    const char *message =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 5\r\n"
    "Connection: close\r\n"
    "\r\n"
    "hellofadsjjdsalfdjsifjdsifjdsafjsdajfoidsjfasdjfiasdjfjsdoifjsadlkfhiasdufjasdifu noaisdfhiadshfsdufnbgdfnbguesidnufoksadhnfauesdnhfasdfsdafnaesudfsdfgbaesidufbadsulnfbeasdnfbuasndufbgsindfhgaesindufuieasndufuaeusdnfcuesdnufceaisdnufugeuhainsdfohainsedfuhiaseundfhuaesudfhiuaeudshfiomesudahinfuoeaisudnfuheiansudfouhiweaucyfuhiuashiuaeysuodfeysdfnychiausihuoaesyfiuneafuehisdjufchueiajdsnufyuhwieajusndyfuhieacsudfyuhicunaeysdhiuawesdhifucyeasudhifuywcaehfyucheuhoiweacdsyfuhiueacfhiucaweyohfiuncaweufhweaiusndfycuchwiaesufhiawueufhuncweuadhifuceawuhisunfuwehiaunfuwhenufchtaha";

    handle_client(setup_server(), message);

    char* current_buffer = buffer;
 char line[1000];
 size_t n;
 char* determinator_pointer = strstr(current_buffer, "\r\n") ;

 while(strstr(current_buffer, "\r\n") != NULL){

    if(determinator_pointer == NULL){
        break;
    }

  char* determinator_pointer = strstr(current_buffer, "\r\n") ;

   if(current_buffer == buffer){
    n = determinator_pointer - current_buffer;
     memcpy(line, current_buffer, n);
     line[n] = '\0';
     current_buffer = determinator_pointer + 2;
     printf("goof%s\n" , line);
     continue;}
   
   n = determinator_pointer - current_buffer;
   if(n == 0) break;
   memcpy(line, current_buffer, n);
   line[n] = '\0';
   printf("This is the while output: %s\n" , line);
   current_buffer = determinator_pointer + 2;
}
   
}



