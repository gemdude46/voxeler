
class tcp_client {
private:
    int sock;
    std::string address;
    int port;
    struct sockaddr_in server;
    bool nb;
    
public:
    tcp_client();
    bool Conn(std::string, int);
    bool Send(std::string data);
    std::string Recv(int,bool);
    void Close(){
        close(sock);
    }
    bool peak() {

        /// Got here because iSelectReturn > 0 thus data available on at least one descriptor
        // Is our socket in the return list of readable sockets
        bool             res;
        fd_set          sready;
        struct timeval  nowait;

        FD_ZERO(&sready);
        FD_SET(sock,&sready);
        //bzero((char *)&nowait,sizeof(nowait));
        memset((char *)&nowait,0,sizeof(nowait));

        res = select(sock+1,&sready,NULL,NULL,&nowait);
        if( FD_ISSET(sock,&sready) )
            res = true;
        else
            res = false;


        return res;

    }
};
 
tcp_client::tcp_client() {
    sock = -1;
    port = 0;
    address = "";
}
 
/**
    Connect to a host on a certain port number
*/
bool tcp_client::Conn(std::string address , int port) {
    //create socket if it is not already created
    this->nb = false;
    if(sock == -1)
    {
        //Create socket
        sock = socket(AF_INET , SOCK_STREAM , 0);
        if (sock == -1)
        {
            throw ("Could not create socket");
        }
         
        //cout<<"Socket created\n";
    }
    else    {   /* OK , nothing */  }
     
    //setup address structure
    if(inet_addr(address.c_str()) == -1)
    {
        struct hostent *he;
        struct in_addr **addr_list;
         
        //resolve the hostname, its not an ip address
        if ( (he = gethostbyname( address.c_str() ) ) == NULL)
        {
            //gethostbyname failed
            herror("gethostbyname");
            //cout<<"Failed to resolve hostname\n";
             
            return false;
        }
         
        //Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
        addr_list = (struct in_addr **) he->h_addr_list;
 
        for(int i = 0; addr_list[i] != NULL; i++)
        {
            //strcpy(ip , inet_ntoa(*addr_list[i]) );
            server.sin_addr = *addr_list[i];
             
            //cout<<address<<" resolved to "<<inet_ntoa(*addr_list[i])<<endl;
             
            break;
        }
    }
     
    //plain ip address
    else
    {
        server.sin_addr.s_addr = inet_addr( address.c_str() );
    }
     
    server.sin_family = AF_INET;
    server.sin_port = htons( port );
     
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        throw ("connect failed. Error");
        return 1;
    }
     
    //cout<<"Connected\n";
    return true;
}
 
/**
    Send data to the connected host
*/
bool tcp_client::Send(std::string data) {
    //Send some data
    if( send(sock , data.c_str() , strlen( data.c_str() ) , 0) < 0)
    {
        throw ("Send failed : ");
        return false;
    }
    //cout<<"Data send\n";
     
    return true;
}
 
/**
    Receive data from the connected host
*/
std::string tcp_client::Recv(int size=512, bool forcelen = false) {
    char buffer[size];
    std::string reply;
     
    //Receive a reply from the server
    if( recv(sock , buffer , sizeof(buffer) , this->nb?MSG_DONTWAIT:0) < 0)
    {
        puts("recv failed");
        return std::string();
    }
    
    if (forcelen)
        reply = std::string(buffer, size);
    else
        reply = buffer;
    return reply;
}
