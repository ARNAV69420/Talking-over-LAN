/*
        Who : arnav
        When : 02:10:03 PM 
        On : Sunday 5 February 2023 
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
        cout<<"Waiting for Connections on port "<<PORT<<endl;
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
            progressbar((float)tot_bytes/size,50);
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
            progressbar((float)tot_bytes/size,50);
        }
        close(f);
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
bool flag;
bool flag_file;
string filename;
long long filesz;
void *sendingworker(void * p)
{
    string s;
    comms O(*((int *)p));
    while(flag)
    {
        getline(cin,s);
        if(!s.empty()) cout<<GOUP<<GRN<<"LocalGuy(You): "<<YEL<<s<<NRM<<endl;
        auto f=split(s,' ');
        if(s[0]=='!')
        {
            if(f[0]=="!pwd")
            {
                cout<<pwd()<<endl;
            }
            else if(f[0]=="!cd")
            {
                if(!~chdir(f[1].c_str()))
                    perror("Cannot change Directory");
            }
            else if(f[0]=="!mkdir")
            {
                if(!~mkdir(f[1].c_str(),0700))
                    perror("Cannot create Directory");
            }
            else if(f[0]=="!rmdir")
            {   
                if(!~rmrf(f[1]))
                    perror("Cannot remove Dir(It may not exist)");
            }
            else if(f[0]=="!rm")
            {
                if(!~remove(f[1].c_str()))
                    perror("Cannot remove file(It may not exist)");
            }
            else if(f[0]=="!put")//client sends "recieve filename size"
            {
                O.sendy("!recieve "+f[1]+" "+to_string(get_file_size(f[1])));
                flag_file=1;
                filename=f[1];
                filesz=get_file_size(f[1]);
                while(flag_file);
            }
            else if(f[0]=="!ls")
            {
                cout<<listFiles(pwd());
            }
            else if(f[0]=="!yes"&&flag_file)
            {
                O.sendy("!yes");
                O.recvfile(filename,filesz);
                flag_file=0;
            }
            else if(f[0]=="!no"&&flag_file)
            {
                O.sendy("!no");
                flag_file=0;
            }
            else if(f[0]=="!goodbye")
            {
                O.sendy(s);
                flag=0;
            }
        }
        else
            O.sendy(s);
        
    }
    O.disconnect();
    return NULL;
}
void *recievingworker(void * p)
{
    comms O(*((int *)p));
    while(flag)
    {
        string s=O.recieve();
        if(s.empty()) continue;
        auto f=split(s,' ');
        if(f[0]=="!recieve")//server recieves "recieve filename size"
        {
            cout<<"Do you want to recieve a file named "<<f[1]<<" having size "<<f[2]<<" bytes"<<endl;
            cout<<"press !yes or !no"<<endl;
            flag_file=1;
            filename=f[1];
            filesz=stoll(f[2]);
            while(flag_file);
        }
        else if(s=="!yes"&&flag_file)
        {
            O.sendfile(filename);
            flag_file=0;
        }
        else if(s=="!no"&&flag_file)
        {
            cout<<"The recipient refused"<<endl;
            flag_file=0;
        }
        else if(s[0]!='!')
            cout<<MAG<<"RemoteGuy: "<<BLU<<s<<NRM<<endl;
        if(f[0]=="!goodbye") 
        {
            cout<<MAG<<"RemoteGuy: "<<BLU<<s<<NRM<<endl;
            flag=0;
        }
    }
    O.disconnect();
    return NULL;
}
int main()
{
    cout<<"Let us remind you of a few tips and rules you need to keep in mind:\n";
    cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
    cout<<"1.Be Respectful and maintain Boundaries\n";
    cout<<"2.You can also issue commands to run at your own machine, commands begin with !\n";
    cout<<"3.Press "<<RED<<"!goodbye"<<NRM<<" to terminate the chat\n";
    cout<<"4.Press "<<CYN<<"!clear"<<NRM<<" to terminate the chat\n";
    cout<<"\t(this simply clears the page and does not terminate the connection)\n";
    cout<<"5.Other commands include \""<<YEL<<"cd,ls,mkdir,rm,rmdir"<<NRM<<"\" with a preceeding !\n";
    cout<<"6.Have Fun\n\n";
    pthread_t thr[2];
    int fd,choice;
    cout<<"Now, Do you want to host(press1) or connect(press2): ";
    cin>>choice;
    flag=1;
    flag_file=0;
    if(choice==1)
    {
        connector server(1,PORT);
        server.listenForClients();
        fd=server.acceptNow();
        pthread_create(&thr[0],NULL,sendingworker,(void *)&fd);
        pthread_create(&thr[1],NULL,recievingworker,(void *)&fd);
        pthread_join(thr[0],NULL);
        pthread_join(thr[1],NULL);
        server.closeServer();
    }
    else if(choice==2)
    {
        string addr;
        cout<<"Who do you want to connect to?"<<endl;
        cin>>addr;
        connector client(0,PORT,addr);
        fd=client.connectToServer();
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
