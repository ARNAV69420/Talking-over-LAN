/*
        Who : arnav
        When : 09:45:26 PM 
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
                        int reuse = 1;
                        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
                                perror("setsockopt(SO_REUSEADDR) failed");
                        #ifdef SO_REUSEPORT
                                if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
                                        perror("setsockopt(SO_REUSEPORT) failed");
                        #endif
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
        }
        int acceptNow()
        {
                int newsockfd=accept(sockfd,NULL,NULL);
                if(!~newsockfd) err("cannot accept");
                return newsockfd;
        }
        int getSocket()
        {
                return sockfd;
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
        int byte_total = 0,byte_now = 0,len = s.size();
        while(byte_total<len)
        {
            if(!~(byte_now=send(fd,&buf[byte_total],len-byte_total, 0)))
            {
                perror("cannot send");
                return false;
            }
            byte_total+=byte_now;
        }
        return true;
    }
    string recieve()
    {
        string s;
        char last=';';
        do
        {
            char buf[MAX_SIZE]={0};
            read(fd,buf,MAX_SIZE);
            string ret(buf);
            s+=ret;
            last=s[s.size()-1];
        }while(last!='~');
        return s.substr(0,s.size()-1);
    }
    void disconnect()
    {
        close(fd);
    }
};
int serverflag;
vector<int> num;
vector<int> flag;
vector<int> fd;
vector<int> isChatting;
vector<string> names;
vector<pthread_t> workers;
map<string,int> nameToId;
map<string,string> chattingwith;
string status()
{
    string res;
    for(auto a:nameToId)
        if(flag[a.second])
            res+=string(YEL)+a.first+" : "+(isChatting[a.second]?string(RED)+"BUSY":string(GRN)+"FREE")+"|";
    return res;
}
void connect(string a,string b)
{
    string response;
    if(a==b)
    {
        response=string(CYN)+"Server : "+string(RED)+"You cannot connect to yourself"+string(NRM);
        comms(fd[nameToId[a]]).sendy(response);
    }
    else if(chattingwith.find(a)!=chattingwith.end())
    {
        response=string(CYN)+"Server : "+string(RED)+"You are already connected to "+chattingwith[a]+string(NRM);
        comms(fd[nameToId[a]]).sendy(response);
    }
    else if(chattingwith.find(b)!=chattingwith.end())
    {
        response=string(CYN)+"Server : "+string(RED)+b+" is already connected to "+chattingwith[b]+string(NRM);
        comms(fd[nameToId[a]]).sendy(response);
    }
    else if(nameToId.find(b)==nameToId.end())
    {
        response=string(CYN)+"Server : "+string(RED)+b+" is not online"+string(NRM);
        comms(fd[nameToId[a]]).sendy(response);
    }
    else
    {
        chattingwith[a]=b;
        chattingwith[b]=a;
        isChatting[nameToId[a]]=1;
        isChatting[nameToId[b]]=1;
        response=string(BLU)+a+" is now connected to "+b+string(NRM);
        cout<<GRN<<string(15,' ')<<response<<NRM<<endl;
        response=string(CYN)+"Server : "+response;
        comms(fd[nameToId[a]]).sendy(response);
        comms(fd[nameToId[b]]).sendy(response);
    }
}
void closeSess(string s)
{
    if(chattingwith.find(s)==chattingwith.end())
        return;
    string b=chattingwith[s];
    string response=s+" is now disconnected from "+b;
    cout<<string(15,' ')<<RED<<response<<NRM<<endl;
    response=string(CYN)+"Server"+string(NRM)+" : "+string(RED)+s+" closed the connection"+string(NRM);
    comms(fd[nameToId[b]]).sendy(response);
    if(chattingwith.find(s)!=chattingwith.end())
        chattingwith.erase(s);
    if(chattingwith.find(b)!=chattingwith.end())
        chattingwith.erase(b);
    isChatting[nameToId[s]]=isChatting[nameToId[b]]=0;
}
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
void *handleClient(void *p)
{
        int i=*(int *)p;
        comms O(fd[i]);
        string name=O.recieve(); 
        if(nameToId.find(name)!=nameToId.end()&&!nameToId.empty())
        {
            O.sendy(string(CYN)+"Server"+" : "+string(RED)+"user with same name already present (exit and try again)"+string(NRM));
            flag[i]=0;
            return NULL;
        }
        cout<<GRN<<string(15,' ')<<name<<" connected\n"<<NRM;
        string s;
        nameToId[name]=i;
        while(flag[i])
        {
            
            s=O.recieve();
            if(s.size()==0) continue;
            auto f=split(s,' ');
            if(s=="!test")
            {
                string response=status();
                cout<<YEL<<string(15,' ')<<"sending the list of users to "<<name<<NRM<<endl;
                O.sendy(response);
            }
            else if(f[0]=="!connect")
            {
                connect(name,f[1]);
            }
            else if(s=="!close")
            {
                closeSess(name);
                cout<<RED<<string(15,' ')<<name<<" disconnected from the server"<<NRM<<endl;
                flag[i]=0;
            }
            else if(chattingwith.find(name)!=chattingwith.end())
            {
                string recipient=chattingwith[name];
                if(s=="!goodbye")
                {
                    closeSess(name);
                    continue;
                }
                s=string(MAG)+name+" : "+string(YEL)+s+string(NRM);  
                comms(fd[nameToId[recipient]]).sendy(s);
            }
            else
            {
                O.sendy(string(CYN)+"Server"+" : "+string(RED)+"command in wrong format"+string(NRM));
            }
        }
        O.disconnect();
        return NULL;
}
void killer(int sig)
{
    for(auto g:num)
        if(flag[g])
            comms(fd[g]).sendy("!close");
    exit(1);
}
int main()
{
    signal(SIGINT,killer);
    connector server(1,PORT);
    server.listenForClients();
    cout<<"\tServer setup and listening for clients..."<<endl;
    while(1)
    {
        fd.pb(server.acceptNow());
        workers.eb();
        num.pb(num.size());
        flag.pb(1);
        isChatting.pb(0);
        pthread_create(&workers.back(),NULL,handleClient,&num.back());
    }
}
                                                                
