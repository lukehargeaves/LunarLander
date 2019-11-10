#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <curses.h>
#include <pthread.h>
#include <semaphore.h>

//Initalise the semaphores
sem_t s;

//Sets the commands as global variables for the values for engine thrust and rcs-roll	
int engineThrust = 0;
double rcs = 0.0;
char toString[50];

//Creates the string buffers to store the fuel, altitude and status of the lander
static char str1[100], fuel[100], alt[100], status[100];

//Initialises the host connection and ports
char *host = "127.0.0.1";
char *port = "65200";
char *port2 = "65250";

//Initialises a global variable to store the reply from the server
char *nMsg = "";



//Checks the status of the lander
int statusCheck(){
	int value = 0;
	//checks if the lander is flying. If so value ==1
	if(strcmp(status,"contact:crashed") ==0){
		value = 1;
	}
	return value;
}

//Writes the reply from the UDP server to the datalogger text file
void *dataLogger(char *input){
	sem_wait(&s);
	//Adds to the datalogger file using the input
	FILE *out_file = fopen("dataLogger.txt","a");
	fprintf(out_file,"%s\n",input);
    fclose(out_file);
    sem_post(&s);
    return NULL;
}

//Writes the condition of the lander to a seperate file to be read and update the dashboard
void *dataWritter(void *arg){
	sem_wait(&s);
	FILE *outfile = fopen("dataLogger1.txt","w");
    fprintf(outfile,"%s\n",nMsg);
    fclose(outfile);
	sem_post(&s);
	return NULL;
}

//Removes the character from a the input string
void removeChar(char *s, int c){

    int j, n = strlen(s);
    for (int i=j=0; i<n; i++)
            if (s[i] != c)
                    s[j++] = s[i];
    s[j] = '\0';
}


void *dashUpdater(){
	//sem_wait(&s);
	//Creates the file to store the landers condition
	FILE *infile  = fopen("dataLogger1.txt","r");
	//Writes the lines from the file to the string variables
    fscanf(infile,"%s %s %s %s \n", str1, fuel, alt, status);
    //Removes the '%' sign from fuel
    removeChar(fuel, '%');
    //sem_post(&s);
	return NULL;
}

//UDP client for the Dashboard connection
void *connectionDash(){
    struct addrinfo *address;
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
    };
    int fd, err;
    //Gets the host and Lander dash port
    err = getaddrinfo( host, port2, &hints, &address);

    if (err) {
        fprintf(stderr, "Error getting address: %s\n", gai_strerror(err));
           exit(1);
    }  

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        fprintf(stderr, "error making socket: %s\n", strerror(errno));
        exit(1);
    }

  	//Initializes the buffer size for the sending messages
    const size_t buffsize = 4096;   
    //Creates the strings with the max message buffer   
    char outgoing[buffsize],outgoing2[buffsize];
    //Copies the variables to the outgoing strings
    strcpy(outgoing, fuel);
    strcpy(outgoing2, alt);
    //Sends the commands to the lander dash
    sendto(fd, outgoing, strlen(outgoing), 0, address->ai_addr, address->ai_addrlen);
    sendto(fd, outgoing2, strlen(outgoing2), 0, address->ai_addr, address->ai_addrlen);
	//sendto(fd, outgoing, strlen(outgoing), 0, address->ai_addr, address->ai_addrlen);
    return NULL;
}

//UDP client for the Lander connection
void *connectionOut(void *arg){
	
    struct addrinfo *address;
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
    };
    int fd, err;
    //Gets the host and Lander port
    err = getaddrinfo( host, port, &hints, &address);

    if (err) {
        fprintf(stderr, "Error getting address: %s\n", gai_strerror(err));
           exit(1);
    }  

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        fprintf(stderr, "error making socket: %s\n", strerror(errno));
        exit(1);
    }

    //Initializes the buffer size for the sending messages
    const size_t buffsize = 4096;   
    //Creates the strings with the max message buffer     
    char incoming[buffsize],outgoing[buffsize];
	size_t msgsize;

	//Copies the variables to the outgoing strings
    strcpy(outgoing, arg);
    //Sends the commands to the lander
    sendto(fd, outgoing, strlen(outgoing), 0, address->ai_addr, address->ai_addrlen);

    //Gets the message from the same connection. This is the reply from the server
    msgsize = recvfrom(fd, incoming, buffsize, 0, NULL, 0); /* Don't need the senders address */
    incoming[msgsize] = '\0';
    nMsg = incoming;
    
    //Logs the reply from the server
	dataLogger(incoming);

	//Prints the response from the server
	printf("%s\n",incoming);

    return NULL;
}



//Prints the terrain for the lander
void *terrain(){
	//sem_wait(&s);
	char *terr = "terrain:?";
	printf("Terrain\n");
	connectionOut(terr);
	//sem_post(&s);
	return NULL;
}

//Prints the state of the lander
void *state(){
	//sem_wait(&s);
	char *state = "state:?";
	printf("Lander state\n");
	connectionOut(state);
	//sem_post(&s);
	return NULL;
}

//Controls for the Lander
void *control(){
	sem_wait(&s);
	//Commands for the lander
	char rcsRoll[50] = "command:!\nrcs-roll: ";
	char engine[50] = "command:!\nmain-engine: ";
	char input;
	printf("\n.....Controls.....\n");
	printf("1.Thrust +10...[w]\n");
	printf("2.Thrust -10...[s]\n");
	printf("3.Rcs-Roll +0.1[a]\n");
	printf("4.Rcs-Roll -0.1[d]\n");
	printf("5.Rcs-Roll 0...[q]\n");
	printf("6.Thrust 0.....[e]\n");
	//printf("7.Exit.........[x]\n");
	printf("Enter your Controls\n");
	scanf("%c",&input);
	switch(input){
                case 'w':
					engineThrust = engineThrust+10;
					sprintf(toString,"command:!\nmain-engine: %d",engineThrust);
					strcat(engine,toString);
					break;
				
				case 's':
					engineThrust = engineThrust-10;
					sprintf(toString,"command:!\nmain-engine: %d",engineThrust);
					strcat(engine,toString);
					break;
				case 'a':
					rcs = rcs-0.1;
					sprintf(toString,"command:!\nrcs-roll: %f",rcs);
					strcat(rcsRoll,toString);
					break;
				case 'd':
					rcs = rcs+0.1;
					sprintf(toString,"command:!\nrcs-roll: %f",rcs);
					strcat(rcsRoll,toString);
					break;
				case 'q':
					rcs = 0;
					sprintf(toString,"command:!\nrcs-roll: %f",rcs);
					strcat(rcsRoll,toString);
					break;
				case 'e':
					engineThrust = 0;
					sprintf(toString,"command:!\nmain-engine: %d",engineThrust);
					strcat(engine,toString);
					break;
				case 'x':
					exit(0);
					break;	
				}
	sem_post(&s);	
	dataLogger(toString);
	return NULL;
}


//Main file which controls the lander
int main(int argc, char*argv[]){

	//Stores the host and 2 port addresses for the UDP server 
	host = argv[1];
	port = argv[2];
	port2 = argv[3];
	/*initialises the value to 0. Used for the while loop to determine the
	state of the lander*/
	int value = 0;
	//Creates the datalogger file
	FILE *init  = fopen("dataLogger.txt","w");
	fclose(init);
	char *input ="condition:?";

	while(value == 0 ){
	//Inintalise the semaphores
	sem_init(&s,0,1);
	//Creates the threads
	pthread_t t1,t2,t3,t4,t5,t6,t7,t8;
	pthread_create(&t1,NULL, control, NULL);
	pthread_join(t1,NULL);
	pthread_create(&t2,NULL, connectionOut, toString);
	pthread_join(t2,NULL);
	pthread_create(&t3,NULL, connectionOut, input);
	pthread_join(t3,NULL);
	pthread_create(&t4,NULL, dataWritter, NULL);
	pthread_join(t4,NULL);
	pthread_create(&t5,NULL, dashUpdater, NULL);
	pthread_join(t5,NULL);
	pthread_create(&t6,NULL, connectionDash, NULL);
	pthread_join(t6,NULL);
	pthread_create(&t7,NULL, terrain, NULL);
	pthread_join(t7,NULL);
	pthread_create(&t8,NULL, state, NULL);
	pthread_join(t8,NULL);
	//Gets the status of the lander
    value = statusCheck();
}
	printf("Lander Crashed......\nProgram ending......\n");


}