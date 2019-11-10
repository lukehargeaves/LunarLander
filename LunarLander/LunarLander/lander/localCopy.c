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
sem_t s,f;

//Sets the commands as global variables for the commands for engine thrust and rcs-roll

	
int engineThrust = 0;
double rcs = 0.0;

char toString[50];

static char str1[100], str2[100], str3[100], str4[100];

//Initialises the host connection and ports
char *host = "000.0.0.0";
char *port = "65200";
char *port2 = "65250";

//Initialises a global variable to store the reply from the server
char *nMsg = "";



int statusCheck(){
	int value = 0;
	if(strcmp(str4,"contact:crashed") ==0){
		value = 1;
	}
	return value;
}
void *dataLogger(char *input){
	FILE *out_file = fopen("dataLogger.txt","a");
	fprintf(out_file,"%s\n",input);
    fclose(out_file);
    return NULL;
}

void *dataWritter(void *arg){
	sem_wait(&s);

	FILE *outfile = fopen("dataLogger1.txt","w");
	
    fprintf(outfile,"%s\n",nMsg);
    fclose(outfile);
	sem_post(&s);
	return NULL;
}


/*void *Engine(void *arg){
	sem_wait(&f);
	char thrust[10]; 
	printf("Please enter the percentage of engine you want to use:......");
	scanf("%s", thrust);
	strcat(engine,thrust);
	dataLogger(engine);
	sem_post(&f);

	return NULL;

}

void *Rcs(void *arg){
	sem_wait(&f);
	char rcs[10]; 
	printf("Please enter the percentage of rcs you want to use:......");
	scanf("%s", rcs);
	strcat(rcsRoll,rcs);
	dataLogger(rcsRoll);
	sem_post(&f);
	return NULL;

}*/
void *connectionDash(void *arg){

	sem_wait(&s);
    struct addrinfo *address;
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
    };
    int fd, err;
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

    /* Client message and  */
    const size_t buffsize = 4096;       /* 4k */
    char outgoing[buffsize], outgoing2[buffsize];
    struct sockaddr clientaddr;
	socklen_t addrlen = sizeof(clientaddr);

    strcpy(outgoing, str2);
    strcpy(outgoing2, str3);
    sendto(fd, outgoing, strlen(outgoing), 0, address->ai_addr, address->ai_addrlen);
    sendto(fd, outgoing2, strlen(outgoing2), 0, address->ai_addr, address->ai_addrlen);

    sem_post(&s);
    return NULL;
}

void *connectionOut(void *arg){

    struct addrinfo *address;
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
    };
    int fd, err;
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

    /* Client message and  */
    const size_t buffsize = 4096;       /* 4k */
    char incoming[buffsize],outgoing[buffsize];
	size_t msgsize;
    struct sockaddr clientaddr;
	socklen_t addrlen = sizeof(clientaddr);

    strcpy(outgoing, arg);
    sendto(fd, outgoing, strlen(outgoing), 0, address->ai_addr, address->ai_addrlen);

    msgsize = recvfrom(fd, incoming, buffsize, 0, NULL, 0); /* Don't need the senders address */
    incoming[msgsize] = '\0';
    nMsg = incoming;

	dataLogger(incoming);

    printf("reply \"%s\"\n", incoming);
  
    return NULL;
}


void removeChar(char *s, int c){

    int j, n = strlen(s);
    for (int i=j=0; i<n; i++)
            if (s[i] != c)
                    s[j++] = s[i];
    s[j] = '\0';


}



void *dashUpdater(void *arg){
	sem_wait(&s);
	FILE *infile  = fopen("dataLogger1.txt","r");
    fscanf(infile,"%s %s %s %s \n", str1, str2, str3, str4);
    printf(" %s\n %s %s\n", str2, str3,str4);
    removeChar(str2, '%');
    printf("%s",str2);
    sem_post(&s);
	return NULL;


}


void *connectionCall(void *arg){
	sem_wait(&f);
	connectionOut(arg);
	sem_post(&f);
	return NULL;
	
}



/*void *controls(void *arg){
	sem_wait(&s);
	sem_init(&f,0,1);

	pthread_t thread1,thread2,thread3,thread4,thread5;
	char *input ="condition:?";
	char in;
	printf("Enter 't' for thrust of 'r' for rcs....\n");
	scanf("%c",&in);
	if(in == 't'){
		pthread_create(&thread1,NULL, Engine, NULL);
		pthread_create(&thread2,NULL, connectionCall, engine);
		pthread_join(thread1,NULL);
		pthread_join(thread2,NULL);
		
	}
	else if(in == 'r'){
		pthread_create(&thread3,NULL, Rcs, NULL);
		pthread_create(&thread4,NULL, connectionCall, rcsRoll);
		pthread_join(thread3,NULL);
		pthread_join(thread4,NULL);
		
	}
	else{
		printf("Invalid command!");
	}
	pthread_create(&thread5,NULL, connectionCall,input );
	pthread_join(thread5,NULL);
	sem_post(&s);
	sem_destroy(&f);
	return NULL;

}*/

void *terrain(void *arg){
	char *terr = "terrain:?";
	connectionOut(terr);
	return NULL;
}

void *state(void *arg){
	char *state = "state:?";
	connectionOut(state);
	return NULL;
}

void *control(){
	
	char rcsRoll[50] = "command:!\nrcs-roll: ";
	char engine[50] = "command:!\nmain-engine: ";
	char input;
	
	printf("enter your command\n");
	
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
				}		

	printf("%s\n",toString );
	return NULL;
}



int main(int argc, char*argv[]){

	host = argv[1];
	port = argv[2];
	port2 = argv[3];
	int value = 0;
	FILE *init  = fopen("dataLogger.txt","w");
	fclose(init);
	char *input ="condition:?";

	while(value == 0){
	sem_init(&s,0,1);
	pthread_t t1,t2,t3,t4,t5,t6,t7,t8;
	pthread_create(&t1,NULL, control, NULL);
	pthread_join(t1,NULL);
	pthread_create(&t7,NULL, connectionOut, toString);
	pthread_join(t7,NULL);
	pthread_create(&t8,NULL, connectionOut, input);
	pthread_join(t8,NULL);
	pthread_create(&t2,NULL, dataWritter, NULL);
	pthread_join(t2,NULL);
	pthread_create(&t4,NULL, dashUpdater, NULL);
	pthread_join(t4,NULL);
	pthread_create(&t3,NULL, connectionDash, NULL);
	pthread_join(t3,NULL);
	pthread_create(&t5,NULL, terrain, NULL);
	pthread_join(t5,NULL);
	pthread_create(&t6,NULL, state, NULL);
	pthread_join(t6,NULL);
    value = statusCheck();
    printf("%d",value);
}

	/*control();
	connectionOut(toString);*/

}