/*
        Who : arnav
        When : 7:50:16 pm IST 
        On : Thursday 2 February 2023 
        Why : 
*/
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/file.h>
#include<bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <ftw.h>
#include <dirent.h>
#define _XOPEN_SOURCE 500
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
long long get_file_size(string filename) // path to file
{
    FILE *p_file = NULL;
    p_file = fopen(filename.c_str(),"rb");
    fseek(p_file,0,SEEK_END);
    long long size = ftell(p_file);
    fclose(p_file);
    return size;
}
void progressbar(float progress, int barwidth)
{
        if(progress>=1.0)
        {
            progress=1.0;
        }
        cout << "[";
        int pos = barwidth * progress;
        for (int i = 0; i < barwidth; ++i) {
            if (i < pos) cout << "=";
            else if (i == pos) cout << ">";
            else cout << " ";
        }
        cout << "] " << int(progress * 100.0) << " %\r";
        cout.flush();
        if(abs(progress-1.0)<=1e-9)
            cout<<endl;
}

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
    void sendfile(string path)
    {
        long long size=get_file_size(path);
        char buffer[65536];
        long long bytes_read=0,bytes_written=0,tot_bytes=0;
        int f=open(path.c_str(),O_RDONLY);
        while (1) 
        {
            bytes_read = read(f, buffer, sizeof(buffer));
            if (bytes_read == 0) // We're done reading from the file
                break;
            if (bytes_read < 0) {
                // handle errors
            }
            char *p = buffer;
            while (bytes_read > 0) 
            {
                bytes_written = send(fd,p,bytes_read,0);
                if (bytes_written <= 0) {
                    // handle errors
                }
                bytes_read -= bytes_written;
                p += bytes_written;
                tot_bytes+=bytes_written;
            }
            progressbar((float)tot_bytes/size,60);
        }
        close(f);
    }
    void recvfile(string path,long long size)
    {
        char buffer[65536];
        long long bytes_read=0,bytes_written=0,tot_bytes=0;
        int f=open(path.c_str(),O_WRONLY|O_APPEND|O_CREAT|O_TRUNC,0744);
        while (tot_bytes<size) 
        {
            bytes_read = recv(fd,buffer, sizeof(buffer)-1,0);
            if (bytes_read == 0) // We're done reading from the file
                break;
            if (bytes_read < 0) 
            {
                perror("could not read :");
                continue;
            }
            buffer[bytes_read]='\0';
            char *p = buffer;
            
            while (bytes_read > 0) 
            {
                bytes_written = write(f,p,bytes_read);
                if (bytes_written < 0) 
                {
                    perror("write error:");
                    continue;
                }
                bytes_read -= bytes_written;
                p += bytes_written;
                tot_bytes+=bytes_written;
            }
            progressbar((float)tot_bytes/size,60);
        }
    }
    void disconnect()
    {
        close(fd);
    }
};
pthread_t server_stop,server_calc;
bool serverflag;
vector<pthread_t> workers;
vector<int> fd;
vector<int> flag;
vector<int> num;
queue<pair<int,string>> q;

//ftp utilities
string pwd()
{
    char BUFF[MAX_SIZE];
    getcwd(BUFF,sizeof(BUFF));
    return string(BUFF);
}
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);
    if (rv)
        perror(fpath);
    return rv;
}
int rmrf(string path)
{
    return nftw(path.c_str(), unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}
string listFiles(string path)
{
    string res;
    struct dirent *dp;
    DIR *dir = opendir(path.c_str());

    // Unable to open directory stream
    if (!dir) 
        return res; 

    while ((dp = readdir(dir)) != NULL)
    {
        res+=(string(dp->d_name)+"\n");
    }
    // Close directory stream
    closedir(dir);
    return res;
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
bool handshake;
void *handleClient(void *p)
{
    int idx=*((int *)p);
    comms O(fd[idx]);
    while(flag[idx])
    {
        string s=O.recieve();
        if(s=="exit")
        {
            flag[idx]=0;
            continue;
        }
        if(s.size())  
        {
            q.push({fd[idx],s}); auto f=split(s,' ');
            if(f[0]=="send"||f[0]=="recieve") handshake=1;
            while(handshake);
            cout<<"Client "<<num[idx]<<" sent request \n";
        }
    }
    O.disconnect();
    cout<<"Client "<<num[idx]<<" disconnected\n";
    return NULL;
}
void *serverkill(void *p)
{
    string s;
    while(serverflag)
    {
        getline(cin,s);
        if(s=="exit")
        {
            serverflag=0;
        }
    }
    for(auto i:num)
    {
        comms O(fd[i]);
        if(flag[i]) O.sendy("exit");
        flag[i]=0;
    }
    return NULL;
}

void *serve(void *p)
{
    while(serverflag)
    {
        if(q.empty()) continue;
        auto [a,b]=q.front(); q.pop();
        comms O(a);
        auto f=split(b,' ');
        if(f[0]=="pwd")
        {
            O.sendy(pwd());
        }
        else if(f[0]=="cd")
        {
            if(!~chdir(f[1].c_str()))
                O.sendy("Cannot change Directory");
            else
                O.sendy("^^^!");
        }
        else if(f[0]=="mkdir")
        {
            if(!~mkdir(f[1].c_str(),0700))
                O.sendy("Cannot create Directory");
            else
                O.sendy("^^^!");
        }
        else if(f[0]=="rmdir")
        {
            if(!~rmrf(f[1]))
                O.sendy("Cannot remove Dir(It may not exist)");
            else
                O.sendy("^^^!");
        }
        else if(f[0]=="rm")
        {
            if(!~remove(f[1].c_str()))
                O.sendy("Cannot remove file(It may not exist)");
            else
                O.sendy("^^^!");
        }
        else if(f[0]=="send") //server recieves "send filename"
        {
            O.sendy(to_string(get_file_size(f[1])));
            O.sendfile(f[1]);
            handshake=0;
        }
        else if(f[0]=="recieve")//server recieves "recieve filename size"
        {
            long long size=stoll(f[2]);
            O.recvfile(f[1],size);
            handshake=0;
        }
        else if(f[0]=="ls")
        {
            string res=listFiles(pwd());
            O.sendy(res);
        }
        else
            O.sendy("wrong format");
    }
    return NULL;
}
int main()
{
    string pass="arnavpass";
    serverflag=1;
    handshake=0;
    connector server(1,PORT);
    server.listenForClients();
    pthread_create(&server_stop,NULL,serverkill,NULL);
    pthread_create(&server_calc,NULL,serve,NULL);
    while(serverflag)
    {
        int f=server.acceptNow();
        comms O(f);
        O.sendy("please enter the password correctly(you only get one chance)");
        if(pass!=O.recieve())
        {
            O.sendy("wrong password");
            O.disconnect();
            continue;
        }
        O.sendy("correct password");
        fd.pb(f);
        workers.eb();
        num.pb(num.size());
        cout<<"Client "<<num.back()<<" connected\n";
        flag.pb(1);
        pthread_create(&workers.back(),NULL,handleClient,&num.back());
    }
    for(auto g:workers) pthread_join(g,NULL);
    pthread_join(server_stop,NULL);
    pthread_join(server_calc,NULL);
    server.closeServer();
}
