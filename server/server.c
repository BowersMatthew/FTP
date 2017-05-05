#include<netinet/in.h>
#include<signal.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

int SIG = 0;
int CONNECTION = 0;

void clearBuffer(char *buffer, int size){
    int i;
    for (i = 0; i < size; i++){
        buffer[i] = '\000';
    }
}

void signal_handler(int no){
	if(CONNECTION == 1){
		printf("Server Killed by SIGINT!\n");
		printf("Finishing current transfer.\n"); 
    	SIG = 1;
	}
	else{
		printf("Server Killed by SIGINT\n");
		exit(-2);
	}
}

int main(int argc, char *argv[]){
    int server, client, errno, inet_len;
    char buffer[50];
    struct sockaddr_in saddr, caddr;
    FILE *file;
    char *fileName;
    char *TERM = "cmsc257";
    char *FNF = "File not found";
    char *CRASH = "Server Shut Down";
    
    signal(SIGINT, signal_handler);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(2432);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    server = socket(PF_INET, SOCK_STREAM, 0);
    if(server == -1){
        printf("Error on socket creation [%s]\n", strerror(errno));
        return (-1);
    }
    
    if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
        printf("setsockopt failed");
        return(-1);
    }   
    if(bind(server, (struct sockaddr *)&saddr, sizeof(struct sockaddr)) == -1){
        printf("Error on socket bind [%s]\n", strerror(errno));
        return (-1);
    }

    if(listen(server, 5) == -1){
        printf("Erron on socket listen [%s]\n", strerror(errno));
        return (-1);
    }

    // the business end
    while(SIG == 0){
        inet_len = sizeof(caddr);
        if((client = accept(server, (struct sockaddr*)&caddr, &inet_len)) == -1){
            printf("Error on client accept [%s]\n", strerror(errno));
            close(server);
            return(-1);
        }
		CONNECTION = 1;
        printf("Server new client connection [%s/%d]\n", inet_ntoa(caddr.sin_addr), caddr.sin_port);

        if(read(client, buffer, sizeof(buffer)) != sizeof(buffer)){
            printf("Error reading network data [%s]\n", strerror(errno));
            close(server);
            return(-1);
        }
        
        // open the file for reading
        if((file = fopen(buffer, "r")) == NULL){
            printf("Local file [%s] not found [%s]\n", buffer, strerror(errno));
            strcpy(buffer, FNF);
            if(write(client, buffer, sizeof(buffer)) != sizeof(buffer)){
                printf("Error writing terminal message to client [%s]\n", strerror(errno));
                close(server);
                return(-1);
            }
            // go to next client
            continue;
        }
        
        // reads 50 bytes from the file to the buffer
        while(1){
            size_t bytesRead;
            clearBuffer(buffer, sizeof(buffer));
            if((bytesRead=fread( buffer, 1, sizeof(buffer), file)) != sizeof(buffer)){
                // short read check for EOF
                if(feof(file) == 0){
                    printf("Error reading from file [%s]\n", strerror(errno));
                    strcpy(buffer, TERM);
                    if(write(client, buffer, sizeof(buffer)) != sizeof(buffer)){
                        printf("Error writing terminal message to client [%s]\n", strerror(errno));
                        close(server);
                        return (-1);
                    }
                }
                else{
                    if(write(client, buffer, sizeof(buffer)) != sizeof(buffer)){
                    printf("Error writing terminal message to client [%s]\n", strerror(errno));
                       close(server);
                       return (-1);
                    }
                    else{
                        printf("File written to client successfully. Closing Connection.\n");
                        clearBuffer(buffer, sizeof(buffer));
                        strcpy(buffer, TERM);
                        if(write(client, buffer, sizeof(buffer)) != sizeof(buffer)){
                            printf("Error writing terminal message to client [%s]\n", strerror(errno));
                            close(server);
                            return (-1);
                        }
						CONNECTION = 0;
                        printf("about to break\n");
                        break;
                    }
                }
            }

            // writes the buffer to the client
            if(write(client, buffer, sizeof(buffer)) != sizeof(buffer)){
                printf("Error reading from file [%s]\n", strerror(errno));
                strcpy(TERM, buffer);
                if(write(client, buffer, sizeof(buffer)) != sizeof(buffer)){
                    printf("Error writing tweminal message to client [%s]\n", strerror(errno));
                    close(server);
                    return (-1);
                }
            }
        }
        if(SIG == 1){
            printf("Server Killed");
            close(server);
            return(-1);
        }
    }
}

