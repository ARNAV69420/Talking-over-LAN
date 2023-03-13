## Chat Server
### _Arnav Kumar Behera_
This source code implements a client and server model of a Chat-Server. Where multiple clients can connect to server and establish chat sessions and exit from them as well. 

### Features
- Supports multiple clients
- Command Line Interface
- Easy on the eyes and simple to use.

### How to get it running?
The client code can be compliled as :
```
    g++ client.cpp -o client -lpthread
```
and executed as :
```
    ./client
```
You can create as many clients as you want.

Similarly the server can be compiled and run using :
```
     g++ server.cpp -o server -lpthread
     ./server
```

### Commands
you can send various commands from the client.
Commands need to be preceded by a '!'. They are as:
```
	!clear : This clears the client-side screen
	!test : This shows the available users and their status
	!connect <username> : This is used to connect to another user
	!goodbye : This exits from an existing chat session
	!close : This disconnects the user from the server
```

### How does it Look?
- ##### Server looks colourful:
![Server](https://drive.google.com/uc?export=view&id=1DNrHc20wykttryKAdzyGtE7icsKbRRe6 "a server")
- ##### Client1
![C1](https://drive.google.com/uc?export=view&id=1nouvkrt6yNIBmQcDlMD1SHrCemNGgfIf "a client")
- ##### Client2
![C2](https://drive.google.com/uc?export=view&id=1LptYa0FzHBTdVAwkNDDRfEwf4icgQZyz "another client")

Further curiosities can be quenched by trying it out.

## Contact 
20CS01070@iitbbs.ac.in

