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
#include <string.h>
#include <stdbool.h>

#include <pthread.h>
#include <semaphore.h>

#define BUFF_SIZE 100
 
sem_t s;

char rc[BUFF_SIZE] = "command:!\nrcs-roll: ";
char engine[BUFF_SIZE] = "command:!\nmain-engine: ";

char *host = "---.0.-.1";
char *port1 = "65200";
//char *port2 = "65250";



void *controls(void *arg){

        char input,toString[BUFF_SIZE];
        printf("Enter your controls:.......\n");
        scanf("%c", &input);
        //sem_wait(&s);
        //FILE *out_file = fopen("dataLogger.txt","a");
        //fprintf(out_file,"%c\n",input);
        //fclose(out_file);
        int thruster = 50;
        double rcs = 0;
        
        switch(input){
                case 'w':
                		thruster = thruster +10;
                		sprintf(toString,"%d",thruster);
                		strcat(engine, toString);
        				break;
                case 's':
                		thruster = thruster -10;
                		sprintf(toString,"%d",thruster);
                		strcat(engine, toString);
        				break;
              
                case 'd':
			            rcs = rcs +5;
                        sprintf(toString,"%f",rcs);
                		strcat(rc, toString);    
                        break;
                case 'a':
                        rcs = rcs-5;
                        sprintf(toString,"%f",rcs);
                		strcat(rc, toString);       
                        break;
                
                case 'q':
                        thruster =0;
                        sprintf(toString,"%d",thruster);
                		strcat(engine, toString);
                        break;
                case 'e':
                        rcs=0;
                        sprintf(toString,"%f",rcs);
                		strcat(rc, toString);
                        break;
                    }
        //sem_post(&s);
        return NULL;
}

void *connectionIn(void *arg){

 	
    struct addrinfo *address;
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
    };
    int fd, err;
    err = getaddrinfo( host, port1, &hints, &address);

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
    char incoming[buffsize], outgoing[buffsize];
    size_t msgsize;

    /*FILE *out_file = fopen("dataLogger.txt","a");
    fprintf(out_file,"%s\n",output);*/

    //strcpy(outgoing, message);
    //sendto(fd, outgoing, strlen(outgoing), 0, address->ai_addr, address->ai_addrlen);

    msgsize = recvfrom(fd, incoming, buffsize, 0, NULL, 0); /* Don't need the senders address */
    incoming[msgsize] = '\0';

    printf("%s",incoming);

    
    /*fprintf(out_file,"%s\n",incoming);
    fclose(out_file);*/

    return 0;

}


int main(int argc, char*argv[])
{
	host = argv[1];
	//connectionIn(host);
    //char *port1 = argv[2];
    //char *port2 = "65250";
    sem_init(&s,0,1);
	pthread_t t1,t2;
	pthread_create(&t1,NULL, controls, NULL);
	pthread_create(&t2,NULL, connectionIn, NULL);
	pthread_join(t1,NULL);
	printf("%s\n",engine);
	pthread_join(t2,NULL);
	printf("finished thread");

	 
	sem_destroy(&s);
    //connectionIn(host,port1,engine);
    //char *status = "contact:flying";
    //char * output = controls();
    connectionIn(host);
    //printf("%s \n",output);
	 printf("%s\n",engine);

    //connectionOut(host, port1, output);
    
}