/*
        Who : arnav
        When : 09:45:21 PM 
        On : Tuesday 7 March 2023 
        Why : 
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

#define PORT 42069
#define MAX_SIZE 1024
#define pb push_back 
#define eb emplace_back
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
    void closeServer()
    {
        close(sockfd);
    }
};
//common functions()
class comms
{
    int fd;
    public:
    comms(int f) {fd=f;}
    bool sendy(string s)
    {
        s=s+"~";
        char buf[MAX_SIZE];
        strcpy(buf,&s[0]);
        int byte_total = 0;
        int byte_now = 0;
        int len = s.size();
        while(byte_total<len)
        {
            if(!~(byte_now=send(fd,&buf[byte_total],len-byte_total, 0)))
            {
                perror("cannot send");
                return false;
            }
            byte_total+= byte_now;
        }
        return true;
    }
    string recieve()
    {
        string s= "";
        char last_character = '^';
        do
        {
            char buf[MAX_SIZE]={0};
            read(fd,buf,MAX_SIZE);
            string ret(buf);
            s+=ret;
            last_character = s[s.size()-1];
        }while(last_character!='~');

        return s.substr(0,s.size()-1);
    }
    void disconnect()
    {
        close(fd);
    }
};
vector<string> split (const string &s, char delim) 
{
    vector<string> result;
    stringstream ss (s);
    string item;
    while (getline (ss, item, delim)) {
        result.pb(item);
    }
    return result;
}
string name;
int isConnected,fd;
string chatter;
void * sender(void  * p)
{
    string s;
    comms O(*((int *)p));
    while(isConnected)
    {
        getline(cin,s);
        if(s=="!clear")
        {
            cout<<CLR;
            continue;
        }
        string prompt=string(8,' ')+name+"(You): ";
        if(s.size())    cout<<GOUP<<left<<GRN<<prompt<<YEL<<s<<NRM<<endl;
        O.sendy(s);
        if(s=="!close")
        {
            isConnected=0;
            exit(0);
        }
            
    }
    return NULL;
}
void *reciever(void * p)
{
    comms O(*((int *)p));
    while(isConnected)
    {
        string s=O.recieve();
        if(s.size()==0) continue;
        if(s.back()=='|')
        {
            auto f=split(s,'|');
            cout<<string(65,' ')<<CYN<<"Server : "<<endl;
            for(auto g:f)
                cout<<string(74,' ')<<g<<NRM<<endl;
            continue;

        }
        if(s!="!close")  cout<<string(65,' ')<<s<<endl;
        if(s=="!close")
        {
            cout<<string(65,' ')<<CYN<<"Server : "<<RED<<"Closed the Connection"<<endl;
            isConnected=0;
        }
    }
    return NULL;
}
void killer(int sig)
{
    comms(fd).sendy("!close");
    exit(1);
}
int main()
{
    string s;
    signal(SIGINT,killer);
    pthread_t senderThread,recieverThread;
    cout<<"Which server do you want to connect to? : ";
    cin>>s;
    connector client(0,PORT,s);
    fd=client.connectToServer();
    cout<<"write your username :\n";
    cin>>name;
    comms(fd).sendy(name);
    cout<<GRN<<"\t______________________________________________________________"<<endl;
    cout<<RED<<"\t                    Read This                                 "<<endl;
    cout<<NRM<<"\t1.commands need to be specified by a preceeding !"<<endl;
    cout<<NRM<<"\t2.Commands are as follows:\n";
    cout<<"\t>"<<GRN<<"!clear"<<NRM<<" : "<<"This clears the client-side screen"<<endl;
    cout<<"\t>"<<MAG<<"!test"<<NRM<<" : "<<"This shows the available users and their status"<<endl;
    cout<<"\t>"<<CYN<<"!connect <username>"<<NRM<<" : "<<"This is used to connect to another user"<<endl;
    cout<<"\t>"<<RED<<"!goodbye"<<NRM<<" : "<<"This exits from an existing chat session"<<endl;
    cout<<"\t>"<<RED<<"!close"<<NRM<<" : "<<"This disconnects the user from the server"<<endl;
    cout<<GRN<<"\t^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
    isConnected=1;
    pthread_create(&senderThread,NULL,sender,(void *)&fd);
    pthread_create(&recieverThread,NULL,reciever,(void *)&fd);
    pthread_join(senderThread,NULL);
    pthread_join(recieverThread,NULL);
    comms(fd).disconnect();
    return 0;
}