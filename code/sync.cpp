/*
        Who : arnav
        When : 5:28:49 pm IST 
        On : Wednesday 1 February 2023 
        Why : I was bored again
*/
#include<bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#define NRM  "\x1B[0m"
#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define YEL  "\x1B[33m"
#define BLU  "\x1B[34m"
#define MAG  "\x1B[35m"
#define CYN  "\x1B[36m"
#define WHT  "\x1B[37m"
#define GOUP "\033[A\r"
#define CLR "\e[2J\e[H"

#define PORT 8083
#define MAX_SIZE 1024
using namespace std;
void err(string s)
{
    perror(s.c_str()); exit(1);
}
class connector
{
    sockaddr_in saddr;
    int sockfd;
    public:
    connector(bool s_c,uint16_t port,string addr="") //s_c=0 for client ,=1 for server
    {
        sockfd=socket(AF_INET,SOCK_STREAM,0);
        if(!~sockfd)    err("cant create socket");
        memset(&saddr,0,sizeof(sockaddr));
        saddr.sin_family=AF_INET;
        saddr.sin_port=htons(port);
        if(s_c)
        {
            saddr.sin_addr.s_addr=htonl(INADDR_ANY);
            if(!~bind(sockfd,(sockaddr *)&saddr,sizeof saddr))  err("cannot bind socket");
        }
        else
            if(!~inet_pton(AF_INET,addr.c_str(),&saddr.sin_addr))    err("wrong server address");
    }
    //client side functions
    int connectToServer()
    {
        if(!~connect(sockfd,(sockaddr *)&saddr,sizeof saddr))   err("cannot connect");
        return sockfd;
    }

    //server side functions
    void listenForClients()
    {
        if(!~listen(sockfd,5))  err("cannot listen");
        cout<<"Waiting for Connections on port"<<PORT<<endl;
    }
    int acceptNow()
    {
        int new_fd;
        socklen_t a=sizeof(saddr);
        if(!~(new_fd=accept(sockfd,(sockaddr *)&saddr,(socklen_t *)&a))) err("cannot accept");
        return new_fd;
    }
};
//common functions()
class comms
{
    int fd;
    public:
    comms(int f) {fd=f;}
    string recieve()
    {
        char buf[MAX_SIZE];
        int num=0;
        if(!~(num=recv(fd,buf,MAX_SIZE-1,0))) {/* perror("cannot read"); */ return string();}
        buf[num]='\0';
        return string(buf);
    }
    bool sendy(string s)
    {
        if(!~send(fd,&s[0],s.size(),0)) {/* perror("cannot send"); */ return 0;}
        return 1;
    }
    void disconnect()
    {
        close(fd);
    }
};

bool flag;
void *sendingworker(void * p)
{
    string s;
    comms O(*((int *)p));
    while(flag)
    {
        getline(cin,s);
        if(s=="clear")
        {
            cout<<CLR;
            continue;
        }
        if(s.size())    cout<<GOUP<<GRN<<"LocalGuy(You): "<<YEL<<s<<NRM<<endl;
        O.sendy(s);
        if(s=="goodbye")
        {
            O.disconnect();
            flag=0;
        }
    }
    return NULL;
}
void *recievingworker(void * p)
{
    comms O(*((int *)p));
    while(flag)
    {
        while(flag)
        {
            string s=O.recieve();
            if(s.size())  cout<<MAG<<"RemoteGuy: "<<BLU<<s<<NRM<<endl;
            if(s=="goodbye")
            {
                O.disconnect();
                flag=0;
            }
        }
    }
    return NULL;
}
int main()
{
    cout<<"Let us remind you of a few tips and rules you need to keep in mind:\n";
    cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
    cout<<"1.Be Respectful and maintain Boundaries\n";
    cout<<"2.Press \""<<RED<<"goodbye"<<NRM<<"\" (without the quotes) to terminate the chat\n";
    cout<<"3.Press \""<<CYN<<"clear"<<NRM<<"\" (without the quotes) to clear the chat\n";
    cout<<"\t(this simply clears the page and does not terminate the connection)\n";
    cout<<"4.Have Fun\n\n";
    pthread_t thr[2];
    int fd,choice;
    cout<<"Now, Do you want to host(press1) or connect(press2): ";
    cin>>choice;
    if(choice==1)
    {
        connector server(1,PORT);
        server.listenForClients();
        fd=server.acceptNow();
        flag=1;
        pthread_create(&thr[0],NULL,sendingworker,(void *)&fd);
        pthread_create(&thr[1],NULL,recievingworker,(void *)&fd);
        pthread_join(thr[0],NULL);
        pthread_join(thr[1],NULL);
    }
    else if(choice==2)
    {
        string addr;
        cout<<"Who do you want to connect to?"<<endl;
        cin>>addr;
        connector client(0,PORT,addr);
        fd=client.connectToServer();
        flag=1;
        pthread_create(&thr[0],NULL,sendingworker,(void *)&fd);
        pthread_create(&thr[1],NULL,recievingworker,(void *)&fd);
        pthread_join(thr[0],NULL);
        pthread_join(thr[1],NULL);
    }
    else 
        cout<<"Try learning english before you chat with others"<<endl;
    cout<<"Thank you for using our services!"<<endl;
    return 0;
}
