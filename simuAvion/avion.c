#include "saca.h"
#include "avion.h"

// caractéristiques du déplacement de l'avion
struct deplacement dep;

// coordonnées spatiales de l'avion
struct coordonnees coord;

// numéro de vol de l'avion : code sur 5 caractéres
char buffer[1024];
char numero_vol[6];

int sockfd = 0, intVal = 0, result;
    
int ouvrir_communication() {
    int n = 0;
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: Could not create socket\n");
        return -1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORTAVION); 
 
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);    

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("Error: Connect Failed\n");
       return -1;
    } 

    printf("Connected Successfully\n");
    return 1;
}

void fermer_communication() {
    if(sockfd != 0) {
        close(sockfd);
        printf("Socket Connection Closed Successfully");
    }
}

void envoyer_caracteristiques() {
    char sendBuffer[1024];
    
    sprintf(sendBuffer, "%s,%d,%d,%d,%d,%d", numero_vol, coord.x, coord.y, coord.altitude, dep.vitesse, dep.cap);
 
    if ((send(sockfd, sendBuffer, sizeof(sendBuffer), 0)) < 0) {
        printf("Sending Data to Saca Failed\n");
    	exit(1);
    }
}

/********************************
 ***  Fonctions gérant le déplacement de l'avion : ne pas modifier
 ********************************/

// initialise aléatoirement les paramétres initiaux de l'avion

void initialiser_avion() {
    // initialisation al�atoire du compteur aléatoire
    /*int seed; 
    time(&seed);
    srand(seed); */

    srand (time(NULL));

    // intialisation des paramétres de l'avion
    coord.x = 1000 + random() % 1000;
    coord.y = 1000 + random() % 1000;
    coord.altitude = 900 + random() % 100;

    dep.cap = random() % 360;
    dep.vitesse = 600 + random() % 200;

    // initialisation du numero de l'avion : chaine de 5 caract�res 
    // formée de 2 lettres puis 3 chiffres
    numero_vol[0] = (random() % 26) + 'A';
    numero_vol[1] = (random() % 26) + 'A';
    sprintf(&numero_vol[2], "%03d", (random() % 999) + 1);
    numero_vol[5] = 0;
}

// modifie la valeur de l'avion avec la valeur pass�e en param�tre

void changer_vitesse(int vitesse) {
    if (vitesse < 0)
        dep.vitesse = 0;
    else if (vitesse > VITMAX)
        dep.vitesse = VITMAX;
    else dep.vitesse = vitesse;
}

// modifie le cap de l'avion avec la valeur passée en paramètre

void changer_cap(int cap) {
    if ((cap >= 0) && (cap < 360))
        dep.cap = cap;
}

// modifie l'altitude de l'avion avec la valeur passée en paramètre

void changer_altitude(int alt) {
    if (alt < 0)
        coord.altitude = 0;
    else if (alt > ALTMAX)
        coord.altitude = ALTMAX;
    else coord.altitude = alt;
}

// affiche les caractéristiques courantes de l'avion

void afficher_donnees() {
    printf("Avion %s -> localisation : (%d,%d), altitude : %d, vitesse : %d, cap : %d\n",
            numero_vol, coord.x, coord.y, coord.altitude, dep.vitesse, dep.cap);
}

// recalcule la localisation de l'avion en fonction de sa vitesse et de son cap

void calcul_deplacement() {
    float cosinus, sinus;
    float dep_x, dep_y;
    int nb;

    if (dep.vitesse < VITMIN) {
        printf("Vitesse trop faible : crash de l'avion\n");
        fermer_communication();
        exit(2);
    }
    if (coord.altitude == 0) {
        printf("L'avion s'est ecrase au sol\n");
        fermer_communication();
        exit(3);
    }
    //cos et sin ont un paramétre en radian, dep.cap en degré nos habitudes francophone
    /* Angle en radian = pi * (angle en degré) / 180 
       Angle en radian = pi * (angle en grade) / 200 

       Angle en grade = 200 * (angle en degré) / 180 
       Angle en grade = 200 * (angle en radian) / pi 

       Angle en degré = 180 * (angle en radian) / pi 
       Angle en degré = 180 * (angle en grade) / 200 
     */

    cosinus = cos(dep.cap * 2 * M_PI / 360);
    sinus = sin(dep.cap * 2 * M_PI / 360);

    //newPOS = oldPOS + Vt
    dep_x = cosinus * dep.vitesse * 10 / VITMIN;
    dep_y = sinus * dep.vitesse * 10 / VITMIN;

    // on se d�place d'au moins une case quels que soient le cap et la vitesse
    // sauf si cap est un des angles droit
    if ((dep_x > 0) && (dep_x < 1)) dep_x = 1;
    if ((dep_x < 0) && (dep_x > -1)) dep_x = -1;

    if ((dep_y > 0) && (dep_y < 1)) dep_y = 1;
    if ((dep_y < 0) && (dep_y > -1)) dep_y = -1;

    //printf(" x : %f y : %f\n", dep_x, dep_y);

    coord.x = coord.x + (int) dep_x;
    coord.y = coord.y + (int) dep_y;

    // afficher_donnees();
}

// fonction principale : gère l'exécution de l'avion au fil du temps

void se_deplacer() {
    while (1) {
        sleep(PAUSE);
        calcul_deplacement();
        envoyer_caracteristiques();
    }
}

void deplacer_avion(void *param){
    se_deplacer();
}

void modifier_avion(void *param){
   int afterEqual = 0, compteur = 0; 
   while(1){
        char *action = 0;
        char *valeur = 0;
        char commandBuffer[1024];
        afterEqual = 0, compteur = 0; 
        
        action = (char*)malloc(sizeof(char)*20);
        valeur = (char*)malloc(sizeof(char)*20);
    
	int r = read(sockfd, commandBuffer, sizeof(commandBuffer));

	if (r < 0) {
	    perror("ERROR reading from socket");
	    exit(1);
	}
	
        printf("Received Message From SACA %s\n", commandBuffer);

	for(int i=0; i< strlen(commandBuffer); i++){
	    if(commandBuffer[i] == '='){
		compteur = 0;
		afterEqual = 1;
	    } else {
		if (afterEqual == 1){
		    valeur[compteur] = commandBuffer[i]; 
		} else {	
		    action[compteur] = commandBuffer[i];
		}compteur++;
	    }
	}
        
        intVal = atoi(valeur);

	printf("%s - %d\n", action, intVal);

	if((result = strncmp(action, "vitesse", 7)) == 0){
	    changer_vitesse(intVal);
            sprintf(commandBuffer, "Vitesse Changes Successfully");
	} else if((result = strncmp(action, "cap", 3)) == 0){
	    changer_cap(intVal);
            sprintf(commandBuffer, "Cap Changes Successfully");
	} else if((result = strncmp(action, "altitude", 8)) == 0){
	    changer_altitude(intVal);
            sprintf(commandBuffer, "Altitude Changes Successfully");
        } else {
            sprintf(commandBuffer, "Action Not Found !"); 
        }
        
        if ((send(sockfd, commandBuffer, sizeof(commandBuffer), 0)) < 0) {
            printf("Sending Data to Saca Failed\n");
    	    exit(1);
        }
    }	
}

int main() {
    // on initialise l'avion
    initialiser_avion();

    afficher_donnees();

    // on quitte si on arrive à pas contacter le gestionnaire de vols
    if (!ouvrir_communication()) {
       printf("Impossible de contacter le gestionnaire de vols\n");
       exit(1);
    }
    
    // Utiliser pour deplacer l'avion
    pthread_t deplacerThread;
    pthread_create(&deplacerThread, NULL, deplacer_avion, NULL);

    // Utiliser pour ecouter les changements demander venant du controlleur
    pthread_t modiferThread;
    pthread_create(&modiferThread, NULL, modifier_avion, NULL);
 
    getchar();
    return 1;
}
