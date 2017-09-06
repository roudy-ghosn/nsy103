#include "saca.h"

char *input[1024];
char *avion[1024];
char *reponse[1024];

int sockfd = 0;

int main(int argc, char *argv[]){
    int n = 0, sockfd = 0;
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: Could not create socket\n");
        return -1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORTCONTROLLEUR); 
 
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);    

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("Error: Connect Failed\n");
       return -1;
    } 

    printf("Connected Successfully\n");
    int x = 1;

    printf("Please enter the plane's number (avion={number}): ");

    while(x){

        scanf("%s", input);

        if ((send(sockfd, input, strlen(input), 0)) < 0) {
            printf("Sending Data to Saca Failed\n");
    	    exit(1);
        }

        int r = read(sockfd, reponse, sizeof(reponse));
   
	if (r < 0) {
	    perror("ERROR reading from socket\n");
	    exit(1);
        }
        
        if(strcmp(reponse, "N") == 0){
	    x = 0;
            sprintf(avion, input);
	} else {
            printf("%s !! Please re-enter the plane's number (avion={number}): ", reponse);
        } 
    }
	
    while(1){
        printf("Please enter the command to execute on %s :\n", avion);

        scanf("%s", input);
       
        if ((send(sockfd, input, sizeof(input), 0)) < 0) {
            printf("Sending Data to Saca Failed\n");
    	    exit(1);
        }

        int r = read(sockfd, reponse, sizeof(reponse));
   
	if (r < 0) {
	    perror("ERROR reading from socket\n");
	    exit(1);
        } 
    }
}
