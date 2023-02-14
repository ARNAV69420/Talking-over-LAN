/*
        Who : arnav
        When : 7:49:55 pm IST 
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
long long get_file_size(std::string filename) // path to file
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
            //cout<<bytes_written<<" "<<size<<endl;
            progressbar((float)tot_bytes/size,60);
        }
    }
    void disconnect()
    {
        close(fd);
    }
};
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
int flag;

void * worker(void * p)
{
    string cmd;
    comms O(*((int *)p));
    while(flag)
    {
        string prompt="server";
        cout<<YEL<<prompt<<NRM<<">"<<CYN;
        getline(cin,cmd);
        cout<<NRM;
        if(!cmd.empty())
        {
            auto f=split(cmd,' ');
            if(f[0]=="lpwd")
            {
                cout<<pwd()<<endl;
            }
            else if(f[0]=="lcd")
            {
                if(!~chdir(f[1].c_str()))
                    perror("Cannot change Directory");
            }
            else if(f[0]=="lmkdir")
            {
                if(!~mkdir(f[1].c_str(),0700))
                    perror("Cannot create Directory");
            }
            else if(f[0]=="lrmdir")
            {
                if(!~rmrf(f[1]))
                    perror("Cannot remove Dir(It may not exist)");
            }
            else if(f[0]=="lrm")
            {
                if(!~remove(f[1].c_str()))
                    perror("Cannot remove file(It may not exist)");
            }
            else if(f[0]=="get") //client sends "send filename" to server
            {
                O.sendy("send "+f[1]);
                long long size=stoll(O.recieve());
                O.recvfile(f[1],size);
            }
            else if(f[0]=="put")//client sends "recieve filename size"
            {
                O.sendy("recieve "+f[1]+" "+to_string(get_file_size(f[1])));
                O.sendfile(f[1]);
            }
            else if(f[0]=="ldir")
            {
                cout<<listFiles(pwd());
            }
            else
            {
                O.sendy(cmd);
                string response=O.recieve();
                if(response!="^^^!")
                    cout<<response<<endl;
                if(cmd=="exit")
                {
                    flag=0;
                }
                
            }
                
        }
    }
    O.disconnect();
    return NULL;
}
int main()
{
    string s;
    cout<<"Which server do you want to connect to? : ";
    cin>>s;

    connector client(0,PORT,s);
    int fd=client.connectToServer();
    
    comms O(fd);
    char *pass;
    pass=getpass((O.recieve()+"\n").c_str());
    cin.get();
    O.sendy(pass); //sending password
    string response=O.recieve();
    cout<<response<<endl;
    if(response=="wrong password")
    {
        O.disconnect();
        exit(1);
    }
    flag=1;
    pthread_t a;
    pthread_create(&a,NULL,worker,&fd);
    pthread_join(a,NULL);
    return 0;
}
