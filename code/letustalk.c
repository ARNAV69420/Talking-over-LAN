/*
        Who : arnav
        When : 7:59:39 pm IST 
        On : Saturday 28 January 2023 
        Why : Because i was bored on a saturday 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define PORT "5400"
#define BACKLOG 10
#define MAXDATASIZE 100

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
int main()
{
    printf("\e[2J\e[H");
    printf(KBLU"______________________INITIALIZING______________________\n");
    printf(KNRM"Would you like to connect to existing session or host a new session?\n");
    printf("Press 1 to connect and 2 to host\n");
    
    int choice; scanf("%d",&choice);
    char dest[100];
    size_t siz=100;

    if(choice==1)
    {
        printf("Enter the host name:");
        scanf("%s",dest);
    }

    struct addrinfo hints, *res, *p;
    struct sockaddr_storage their_addr;
    struct sigaction sa;
    socklen_t sin_size;
    char *buff=NULL;
    int sockfd,rv,yes,numbytes,new_fd;
    char s[INET6_ADDRSTRLEN];
    char buf[MAXDATASIZE];
    int pid;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(choice==1)
    {
        if ((rv=getaddrinfo(dest,PORT,&hints,&res))!= 0)
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }
        // loop through all the results and connect to the first we can
        for (p=res;p!=NULL;p=p->ai_next)
        {
            if ((sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
            {
                perror("client: socket");
                continue;
            }
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
            {
                close(sockfd);
                perror("client: connect");
                continue;
            }
            break;
        }
        if (p == NULL)
        {
            fprintf(stderr, "failed to connect\n");
            return 2;
        }
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
        printf("!!!!!miscellaneous commands:!!!!!\n");
        printf(KRED"exit : closes the connection\n");
        printf(KGRN"clrscr : clears the screen\n");
        printf(KNRM"connected to %s\n", s);
        freeaddrinfo(res); // all done with this structure
        pid=fork();
        if(!pid)
        {
            while(1)
            {
                if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
                {
                    perror("recv");
                    exit(1);
                }
                if(numbytes==0) continue;
                buf[numbytes] = '\0';
                printf(KCYN"remoteGuy: ");
                printf(KMAG"%s\n", buf);
                printf(KNRM"");
                if(strcmp(buf,"exit")==0) break;
                
            }
        }
        else
        {
            while(1)
            {
                int child=waitpid(-1,NULL,WNOHANG);
                if(child)
                    break;
                int len=getline(&buff,&siz,stdin);
                if(len==1) continue;
                printf("\033[A\r");
                buff[strlen(buff)-1]='\0';
                if(strcmp("clrscr",buff)==0) {printf("\e[2J\e[H"); continue;}
                if (send(sockfd,buff,strlen(buff),0) == -1)
                    perror("send");
                printf(KGRN"localGuy(you): ");
                printf(KYEL"%s\n", buff);
                printf(KNRM"");
                if(strcmp(buff,"exit")==0) 
                {
                    kill(pid,SIGKILL);
                    break;
                }
            }
        }
            
        close(sockfd);
    }
    else if(choice==2)
    {
        hints.ai_flags = AI_PASSIVE;
        if ((rv=getaddrinfo(NULL,PORT,&hints,&res)) != 0)
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }
        // loop through all the results and bind to the first we can
        for (p=res; p != NULL; p = p->ai_next)
        {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1)
            {
                perror("server: socket");
                continue;
            }
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,&yes,sizeof(int)) == -1)
            {
                perror("setsockopt");
                exit(1);
            }
            if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
            {
                close(sockfd);
                perror("server: bind");
                continue;
            }
            break;
        }
        freeaddrinfo(res);
        if (p == NULL)
        {
            fprintf(stderr,"failed to connect\n");
            return 2;
        }
        sa.sa_handler = sigchld_handler; // reap all dead processes
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, NULL) == -1)
        {
            perror("sigaction");
            exit(1);
        }
        if (listen(sockfd, BACKLOG) == -1)
        {
            perror("listen");
            exit(1);
        }
        printf("waiting for connections...\n");
        printf("!!!!!miscellaneous commands:!!!!!\n");
        printf(KRED"exit : closes the connection\n");
        printf(KGRN"clrscr : clears the screen\n");
        while (1)
        { // main accept() loop
            sin_size = sizeof their_addr;
            new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
            if (new_fd == -1)
            {
                printf("hello broskis");
                perror("accept");
                continue;
            }
            break;
        }
        inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
        printf(KNRM"got connection from %s\n", s);
        close(sockfd);//no need to listen anymore
        pid=fork();
        if(!pid)
        {
            while(1)
            {
                if ((numbytes = recv(new_fd,buf,MAXDATASIZE-1,0)) == -1)
                {
                    perror("recv");
                    exit(1);
                }
                if(numbytes==0) continue;
                buf[numbytes] = '\0';
                printf(KCYN"remoteGuy: ");
                printf(KMAG"%s\n", buf);
                printf(KNRM"");
                if(strcmp(buf,"exit")==0)
                {
                    break;
                } 
                
            }
        }
        else
        {
            while(1)
            {
                int child=waitpid(-1,NULL,WNOHANG);
                if(child)
                    break;
                int len=getline(&buff,&siz,stdin);
                if(len==1) continue;
                printf("\033[A\r");
                buff[strlen(buff)-1]='\0';
                if(strcmp("clrscr",buff)==0) {printf("\e[2J\e[H"); continue;}
                if (send(new_fd,buff,strlen(buff),0) == -1)
                    perror("send");
                printf(KGRN"localGuy(you): ");
                printf(KYEL"%s\n", buff);
                printf(KNRM"");
                if(strcmp(buff,"exit")==0) 
                {
                    kill(pid,SIGKILL);
                    break;
                }
            }
        }      
        close(new_fd);
    }
    if(pid)
    printf(KRED"_________________________EXITING________________________\n");
    return 0;
}
