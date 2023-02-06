#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <regex>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "httpserver.h"
#include "queue.h"
#define PORT 80
#define BUFFER_SIZE 16384 

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

//Starter code taken from cse130 discussion

int fileOpener(char *fileToOpen){
    //taken from coding guidelines and standards. Daniel Bittman, Darrell Long, Ethan Miller
    int fileDescriptor;
    if((fileDescriptor = open(fileToOpen, O_WRONLY | O_CREAT | O_TRUNC)) == -1){fprintf(stderr, "dog updog: %s: %s\n", fileToOpen, strerror(errno));}
    return fileDescriptor;
}

int fileCloser(int fileToClose){
    return close(fileToClose);
}

void * doGetPut(void * p_new_socket){
    int valread;
    struct stat fileInfo;
    char buffer[BUFFER_SIZE] = {0};
    char protocol[5];
    char ascii[28];
    char status[10];
    char host[6];
    char hostAddress[15];
    char agent[12];
    char curl[12];
    char accepted[8];
    char ast[4];
    char conLen[16];
    char lenVal[32];
    char const *response = "HTTP/1.1 200 OK\n\r\n";
    char const *response2 = "HTTP/1.1 200 OK\n";
    char const *created = "HTTP/1.1 201 Created \r\n";
    char const *badRequest = "HTTP/1.1 400 Bad Request\r\n";
    char const *Forbidden = "HTTP/1.1 403 Forbidden\r\n";//
    char const *NotFound = "HTTP/1.1 404 Not Found\r\n";
    char const *internal = "HTTP/1.1 500 Internal Server Error\r\n";
    char const *zeroLength = "Content-Length: 0\r\n\r\n";
    char const *test = "";
    int new_socket = *((int*)p_new_socket);
    free(p_new_socket);
        std::regex b("[A-za-z0-9-_]{27}");
    
        memset(buffer, '\0', sizeof(buffer));
        (valread = recv(new_socket, buffer, sizeof(buffer)/sizeof(buffer[0]), 0));
        //scan header for data needed
        sscanf(buffer, "%s %s %s %s %s %s %s %s %s %s %s", protocol, ascii, status, host, hostAddress, agent, curl, accepted, ast, conLen, lenVal);
        //fprintf(stdout, "Length of Content: %s\n", lenVal);
        int total_data = atoi(lenVal);
        if(regex_match(ascii, b)){
        test = ascii;
        write(1, buffer, strlen(buffer));
        memset(buffer, '\0', sizeof(buffer));
        
            //code to handle PUT request
            if(strcmp(protocol, "PUT")==0){ 
                //write(1, buffer, strlen(buffer));
                //memset(buffer, '\0', sizeof(buffer));//THIS BUFFER WAS COMMENTED OUT
                //check if file exists
                if(access(test, F_OK)==-1){
                    int fd = open(test, O_WRONLY | O_CREAT );
                    //while(((valread = recv(new_socket, buffer, sizeof(buffer)/*sizeof(buffer[0])*/, 0))>0)&&(total_data > 0)){
                    send(new_socket, created, strlen(created), 0);
                    while((total_data>0)){ 
                        valread = recv(new_socket, buffer, sizeof(buffer)/sizeof(buffer[0]), 0);
                        write(fd, buffer, valread);
                        total_data -= valread;    
                    }
                    memset(buffer, '\0', sizeof(buffer));
                    fileCloser(fd);
                //check for write access
                }else if(access(test, W_OK)==-1){
                    send(new_socket, Forbidden, strlen(Forbidden), 0);
                }else{
                //able to open/write to file
                    int fd = open(test, O_WRONLY | O_TRUNC );
                    
                    //while(((valread = recv(new_socket, buffer, sizeof(buffer)/*sizeof(buffer[0])*/, 0))>0)&&(total_data > 0)){
                    send(new_socket, response, strlen(response), 0);
                    while((total_data > 0)){
                        valread = recv(new_socket, buffer, sizeof(buffer)/sizeof(buffer[0]), 0);
                        write(fd, buffer, valread);
                        total_data -= valread;    
                    }
                    memset(buffer, '\0', sizeof(buffer));
                    fileCloser(fd);
                }
                //write(1, nl, strlen(nl));
            //code to handle get requests    
            }else if(strcmp(protocol, "GET")==0){
                //write(1, buffer, strlen(buffer));
                //memset(buffer, '\0', sizeof(buffer));
                int fd = open(ascii, O_RDONLY);
                //check if able to open file
                if(fd == -1){
                    send(new_socket, NotFound, strlen(NotFound), 0);
                    send(new_socket, zeroLength, strlen(zeroLength), 0);
                    
                //check for read access
                }else if(access(ascii, R_OK) == -1){
                    send(new_socket, Forbidden, strlen(Forbidden), 0);
                    send(new_socket, zeroLength, strlen(zeroLength), 0);
                }else{
                //else 200 ok
                    if (fstat(fd, &fileInfo) < 0){
                        fprintf(stderr, "Error fstat --> %s", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    char tempBuffer[50]; 
                    sprintf(tempBuffer, "Content-Length: %ld \r\n\r\n", fileInfo.st_size);
                    //totalRead = 0;
                    send(new_socket, response2, strlen(response2), 0);
                    send(new_socket, tempBuffer, strlen(tempBuffer), 0);
                    //continue until nothing else can be read.
                    while((valread = read(fd, buffer, sizeof(buffer)))>0){

                        write(new_socket, buffer, valread);
                    }
                    fileCloser(fd);
                }
            }else{
               send(new_socket, badRequest, strlen(badRequest), 0); 
               send(new_socket, zeroLength, strlen(zeroLength), 0);
            }
        }else{
            send(new_socket, internal, strlen(internal), 0);
            send(new_socket, zeroLength, strlen(zeroLength), 0);
        } 
        memset(buffer, '\0', sizeof(buffer));
        close(new_socket);
        return NULL; 
}


void * dispatcher_thread(void *arg){
    while(true){
        int *pclient;
        pthread_mutex_lock(&mutex);
        if((pclient = dequeue()) == NULL){
            pthread_cond_wait(&cond, &mutex);
            pclient = dequeue();
        }
        pthread_mutex_unlock(&mutex);
        if(pclient != NULL){
            doGetPut(pclient);
        }
    }
}


int main(int argc, char *argv[]){
    int server_fd, new_socket;

    struct sockaddr_in address;
    int addrlen = sizeof(address);


    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    char *filename;
    int threads = 4;
    int opt;
    int argCounter = 0;
    while((opt = getopt(argc, argv, "l:N:")) != -1)
        switch(opt){
            case 'l':
                filename = optarg;
                break; 
            case 'N':
                threads = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Usage: ./httpserver [-l log_file] [-N numThreads] address [port]\n");
                exit(EXIT_FAILURE);
        default:
            abort();
        }
    
    for(int i = optind; i < argc; i++){
        argCounter++; 
    }
    
    if(argCounter == 1){ 
        if((strcmp(argv[optind], "localhost"))==0){
            address.sin_addr.s_addr = inet_addr("127.0.0.1");
        }else{
            address.sin_addr.s_addr = inet_addr(argv[optind]);
        }
        address.sin_port = htons( PORT );
    }else if(argCounter == 2){
        if((strcmp(argv[optind], "localhost"))==0){
            address.sin_addr.s_addr = inet_addr("127.0.0.1");
        }else{
            address.sin_addr.s_addr = inet_addr(argv[optind]);
        } 
        int p = atoi(argv[optind+1]);
        address.sin_port = htons(p);
    }else{
        perror("USAGE: ./httpserver localhost port");
        exit(EXIT_FAILURE);
    }
    //create threads
    pthread_t thread_pool[threads];
    for(int x=0; x < threads; x++){
    
    pthread_create(&thread_pool[x], NULL, dispatcher_thread, NULL);
    }

    //bind socket to fd
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    //listen on fd
    if(listen(server_fd, 100) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    //code to handle PUT/GET requests 
    while(true){ 
        if((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
            perror("accept");
            exit(EXIT_FAILURE);
        }

        int *pclient = (int*)malloc(sizeof(int));
        *pclient = new_socket;
         pthread_mutex_lock(&mutex);
         enqueue(pclient);
         pthread_cond_signal(&cond);

         pthread_mutex_unlock(&mutex);
        
    } 
}
