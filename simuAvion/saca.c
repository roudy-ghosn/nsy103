#include "saca.h"

char reponse[1024];
char **listeNomAvions = 0;
int listeAvionsSocket[300];
char **listeAvionsUtilise = 0;

int compteurAvion = 0;

void imprimer_avion_detail(void *param){
    int x, y, a, v, c;
    char printBuffer[1024] = {0};
    
    while(1){
        for(int i=0; i< compteurAvion; i++){
            int r = read(listeAvionsSocket[i], printBuffer, sizeof(printBuffer));
   
	    if (r < 0) {
	        perror("ERROR reading from socket");
		exit(1);
	    }
            printf("Plane Message: %s\n", printBuffer);
        }
    }
}

void ecouter_avion(void *param){
    char buffer[1024];
    int counter = 0, comaCounter = 0;
    int listenfd = 0, avionfd = 0, pid;
    struct sockaddr_in serv_addr;
    listeNomAvions = (char**)malloc(sizeof(char*)*300);

    int serv_addr_len = sizeof(serv_addr); 

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Socket Failed\n");
        exit(EXIT_FAILURE);  
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORTAVION); 

    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        printf("Bind Failed\n");
        exit(EXIT_FAILURE);  
    } 
    
    if(listen(listenfd, 10)){
        printf("Listen Failed\n");
        exit(EXIT_FAILURE);
    } 
    
    char *x = (char*)malloc(sizeof(char)*5);
    char *y = (char*)malloc(sizeof(char)*5);
    char *cap = (char*)malloc(sizeof(char)*5);
    char *vitesse = (char*)malloc(sizeof(char)*5);
    char *altitude = (char*)malloc(sizeof(char)*5);

    while(1)
    {
        avionfd = accept(listenfd, (struct sockaddr*)&serv_addr, (socklen_t*)&serv_addr_len); 
	
	listeAvionsSocket[compteurAvion] = avionfd;

	read(avionfd,  buffer, 1024); 
        listeNomAvions[compteurAvion] = (char*)malloc(sizeof(char)*5);
        strncpy( listeNomAvions[compteurAvion], (char*)buffer, 5);

        for(int i=5; i< strlen(buffer); i++){
  
            if(buffer[i] == ','){
                comaCounter++;
                counter = 0;
            }
            else{
	        if(comaCounter == 1){
	            x[counter] = buffer[i]; 
	        }
                else if (comaCounter == 2){
	            y[counter] = buffer[i]; 
	        }   
	        else if (comaCounter == 3){
	            altitude[counter] = buffer[i]; 
                }  
	        else if (comaCounter == 4){
	            vitesse[counter] = buffer[i]; 
	        }   
	        else if (comaCounter == 5){
	            cap[counter] = buffer[i]; 
	        } 
                counter++;   
            }
        }
        printf("%s - %s - %s - %s - %s\n",x,y,altitude,vitesse,cap);
        compteurAvion++;
    }
}

void ecouter_controlleur(void *param){
    int listenfd = 0, contfd = 0 , pid, compteurControlleur = 0, listeControlleursSockets[300];
    struct sockaddr_in serv_addr;
    listeAvionsUtilise = (char**)malloc(sizeof(char*)*300);

    int serv_addr_len = sizeof(serv_addr); 

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Socket Failed\n");
        exit(EXIT_FAILURE);  
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORTCONTROLLEUR); 

    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        printf("Bind Failed\n");
        exit(EXIT_FAILURE);  
    } 
    
    if(listen(listenfd, 10)){
        printf("Listen Failed\n");
        exit(EXIT_FAILURE);
    } 
    
    while(1)
    {
        contfd = accept(listenfd, (struct sockaddr*)&serv_addr, (socklen_t*)&serv_addr_len); 
	
	listeControlleursSockets[compteurControlleur] = contfd;

        pid = fork();
		
        if (pid < 0) {
           perror("ERROR on fork");
           exit(1);
        }
        
        if (pid == 0) {
           close(listenfd);
           char buffer[1024];
           char *action = 0;
           char *valeur = 0;
           char actionBuffer[1024];
           char *avionControlle = 0;
           int  avionfd = 0, dejaUtilise = 0;
           
           while(1){
               int r = read(contfd, buffer, sizeof(buffer));
   
	       if (r < 0) {
		   perror("ERROR reading from socket");
		   exit(1);
	       }

               int compteurBuffer = 0, afterEqual = 0, result = 0;
               valeur = (char*)malloc(sizeof(char)*20);
               action = (char*)malloc(sizeof(char)*5);
               avionControlle = (char*)malloc(sizeof(char)*5);
               strncpy(action, (char*)buffer, 5);
               
               if((strcmp(action, "avion")) == 0){
                   memmove(buffer, buffer+6, strlen(buffer));
                   strncpy(valeur, (char*)buffer, 5);
                   sprintf(reponse, "Plane doesn't exists !");
                  
                   if(dejaUtilise == 1) {
                       sprintf(reponse, "Plane already being controlled !");
                   } else {

                       for(int i=0; i<sizeof(listeNomAvions); i++){
                           if((strcmp(valeur, listeNomAvions[i])) == 0){
                               sprintf(reponse, "N");
                               avionfd = listeAvionsSocket[i];
                               sprintf(avionControlle, listeNomAvions[i]);
                               listeAvionsUtilise[compteurControlleur] = listeNomAvions[i];
                               break;
                           }
                       }
                   }

                   if ((send(contfd, reponse, strlen(reponse), 0)) < 0) {
                       printf("Sending Data to Controller Failed\n");
    	               exit(1);
                   }

               } else if(avionControlle != '\0' && strlen(buffer) != 0) {
                   if ((send(avionfd, buffer, sizeof(buffer), 0)) < 0) {
                       printf("Sending Data to Airplane Failed\n");
    	               exit(1);
                   }

                   int r = read(avionfd, actionBuffer, sizeof(actionBuffer));
   
	           if (r < 0) {
		       perror("ERROR reading from Airplane");
		       exit(1);
	           }

                   if ((send(contfd, actionBuffer, strlen(actionBuffer), 0)) < 0) {
                       printf("Sending Data to Controller Failed\n");
    	               exit(1);
                   }
               } else {
                   sprintf(reponse, "Please enter the flight number\n");
               }
           }
           exit(0);
        }
        close(contfd);
    }
}

int main(int argc, char *argv[])
{
    pthread_t ecouteurAvion;
    pthread_create(&ecouteurAvion, NULL, ecouter_avion, NULL); 
 
    pthread_t imprimerAvionDetail;
    pthread_create(&imprimerAvionDetail, NULL, imprimer_avion_detail, NULL); 

    pthread_t ecouteurControlleur;
    pthread_create(&ecouteurControlleur, NULL, ecouter_controlleur, NULL);

    getchar();
}
