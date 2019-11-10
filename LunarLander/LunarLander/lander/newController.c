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

  
void dataLogger(char incoming){

    FILE *out_file = fopen("dataLogger.txt","a");
    fprintf(out_file,"%c\n",incoming);
    fclose(out_file);
    

}

char *controls(){
        char *out = "";
        char input;
        printf("enter your controls:.......\n");
        scanf("%c",&input);
        FILE *out_file = fopen("dataLogger.txt","a");
        fprintf(out_file,"%c\n",input);
        fclose(out_file);
        int thruster = 50;
        float rcs  = 0;

        switch(input){
                case 'w':
                        thruster = thruster+5;
                        sprintf(out,"command:\nmain-engine: %d",thruster);                                     
                        break;
                case 'd':
                        rcs = rcs +0.5;
                        sprintf(out,"command:\nrcs-roll: %f",rcs);       
                        break;
                case 's':
			            thruster = thruster-5;
                        sprintf(out,"command:\nmain-engine: %d",thruster);      
                        break;
                case 'a':
                        rcs = rcs -0.5;
                        sprintf(out,"command:\nrcs-roll: %f",rcs);        
                        break;
                
                case 'q':
                        thruster = 0;
                        sprintf(out,"command:\nmain-engine: %d",thruster);

                case 'e':
                        rcs = 0;
                        sprintf(out,"command:\nrcs-roll: %f",rcs);
                    }
                
    return out;
}


void connectionOut(char *host, char *port, char *output){

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
    char outgoing[buffsize];

    FILE *out_file = fopen("dataLogger.txt","a");
    fprintf(out_file,"%s\n",output);

    strcpy(outgoing, output);
    sendto(fd, outgoing, strlen(outgoing), 0, address->ai_addr, address->ai_addrlen);

    fclose(out_file);
}



void connectionIn(char *host, char *port, char *output){
 	
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
    char incoming[buffsize], outgoing[buffsize];
    size_t msgsize;

    FILE *out_file = fopen("dataLogger.txt","a");
    fprintf(out_file,"%s\n",output);

    strcpy(outgoing, output);
    sendto(fd, outgoing, strlen(outgoing), 0, address->ai_addr, address->ai_addrlen);

    msgsize = recvfrom(fd, incoming, buffsize, 0, NULL, 0); /* Don't need the senders address */
    incoming[msgsize] = '\0';

    
    fprintf(out_file,"%s\n",incoming);
    fclose(out_file);

}

void removeChar(char *s, int c){

    int j, n = strlen(s);
    for (int i=j=0; i<n; i++)
            if (s[i] != c)
                    s[j++] = s[i];
    s[j] = '\0';
}


void updateDash(char *str3,char *str4, char *host, char *port2){

    connectionOut(host, port2, str3);
    connectionOut(host, port2, str4);

}

char *dashUpdater(char*host, char *port2){
    
    FILE *fp;
    fp  = fopen("dataLogger.txt","r");
    static char str1[100], str2[100], str3[100], str4[100], str5[100];
    fscanf(fp,"%s %s %s %s %s\n", str1, str2, str3, str4, str5);

    printf(" %s\n %s \n %s\n", str3, str4, str5);

    fclose(fp);

    removeChar(str3, '%');

    updateDash(str3, str4, host, port2);

    return str5;
    
}

void dashUpdaterCall(void *arg){
     	
    char *host = "127.0.1.1";
    char *port2 = "65250";
    dashUpdater(host,port2);
    

}

void controlCall(void* arg) {
    char *host = "127.0.1.1";
    char *port1 = "65200";
    char *output;    
    char *test = "contact:flying";
    if (strcmp(test, arg) == 0){
        output = controls(); 
        connectionOut(host,port1,output);
        
    }
} 

void condition(void *arg){
    
    char *host = "127.0.1.1";
    char *port1 = "65200";
    char *condition = "condition:?";
    connectionIn(host,port1, condition);
}

int main(int argc, char*argv[])
{
    char *host = "127.0.1.1";
    char *port1 = "65200";
    char *port2 = "65250";
    char *status = "contact:flying";
    char *output;
    output  = controls();
    //connectionOut(host,port1,output);
    //printf("%s \n",output);


    //connectionOut(host, port1, output);
    
}


