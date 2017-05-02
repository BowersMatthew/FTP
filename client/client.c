#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char *argv[]){

    int errno;
    int socket_fd;
    char *fileName;
    char buffer[50];
    const char *TERM = "cmsc257";
    const char *FNF = "File not found";
    struct sockaddr_in caddr;
    char *ip = "127.0.0.1";
    FILE *file;

    if(argc != 2){
        fprintf(stderr, "You must specify the file name are a commmand line argument.\n");
        exit(-1);
    }

    fileName = argv[1];
    if(strlen(fileName) > 49 ){
        fprintf(stderr, "filename number be less than 50 characters long.\n");
        exit(-1);
    }

    // allocate the space for the buffer
    //buffer = calloc(50, 1);

    // put the message into the buffer
    strcpy(buffer, fileName);


    caddr.sin_family = AF_INET;
    caddr.sin_port = htons(2432);
    if(inet_aton(ip, &caddr.sin_addr) == 0){
        return(-1);
    }

    socket_fd = socket(PF_INET,SOCK_STREAM, 0);
    if(socket_fd == -1){
        printf("Error on socket creation [%s]\n", strerror(errno));
        return(-1);
    }

    if(connect(socket_fd, (const struct sockaddr *)&caddr, sizeof(struct sockaddr)) == -1){
        printf("Error on socket connect [%s]\n", strerror(errno));
        return(-1);
    }

    if(write(socket_fd, buffer, sizeof(buffer)) != sizeof(buffer)) {
        printf("Error writing network data [%s]\n", strerror(errno));
        return(-1);
    }
    
    if((file = fopen(fileName, "w")) < 0){
        printf("Error opening file [%s].\n", fileName);
        return(-1);
    }

    printf("Sent a value of [%s]\n", buffer);
    while(1){
        if(read(socket_fd, buffer, sizeof(buffer)) != sizeof(buffer)){
            printf("Error reading network data [%s]\n", strerror(errno));
            fclose(file);
            return(-1);
        }
        if(strncmp(buffer, TERM, 50) == 0){
            printf("Termination string recieved. Thank you!\n");
            break;
        }
        else if(strncmp(buffer, FNF, 50) == 0){
            printf("That file does not exist! Please try again!\n");
            remove(argv[1]);
            break;
        }

        else if(fputs(buffer, file) < 0){
            printf("Error writing to file\n");
            fclose(file);
            return(-1);
        }
    }
    fclose(file);
    close(socket_fd);
    return(0);
}
