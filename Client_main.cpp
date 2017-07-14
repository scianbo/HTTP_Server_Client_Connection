
/*Header Files*/
#include <cstdlib>
#include <stdio.h>
#include<iostream>
#include <unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<fstream>

/*This class contains variables, accessible to all the client functions*/
using namespace std;
class tcp_client{
    private:
    int sock;
    std::string address;
    int port;
    struct sockaddr_in server;
     
public:
    tcp_client();
    bool conn(string, int);
    bool send_data(string command,string filePath,string data);
    string receive(int);
    
};
tcp_client::tcp_client()
{
    sock = -1;
    port = 0;
    address = "";
}

/**
 * 
 * @param address--client IP address, passed from main();
 * @param port--Port info from the main()
 * @return boolean value
 */
bool tcp_client::conn(string address, int port)
{
    if(sock == -1)
    {
        //Create socket
        sock = socket(AF_INET , SOCK_STREAM , 0);
        if (sock == -1)
        {
            perror("Could not create socket");
            exit(0);
        }
        cout<<"Socket created\n";
    }
    else    {   /* OK , nothing */  }
     //setting up address structure
    if(inet_addr(address.c_str()) == -1)
    {
        struct hostent *hostEntity;
        struct in_addr **addr_list;
        //resolves the host name;handles the errors
        if ( (hostEntity = gethostbyname( address.c_str() ) ) == NULL)
        {
            //gethostbyname failed
            herror("gethostbyname");
            cout<<"Failed to resolve hostname\n";  
            exit(0);
            return false;
        }
         
        //Cast the h_addr_list to in_addr , since h_addr_list also has the IP address in long format
        addr_list = (struct in_addr **) hostEntity->h_addr_list;
        for(int i = 0; addr_list[i] != NULL; i++)
        {
            server.sin_addr = *addr_list[i];
            cout<<address<<" resolved to "<<inet_ntoa(*addr_list[i])<<endl;
            break;
        }
    }
    //In case of a correct IP address format
    else
    {
        server.sin_addr.s_addr = inet_addr( address.c_str() );
    } 
    server.sin_family = AF_INET;
    server.sin_port = htons( port );
    
    //Connect to remote server and handle the error, if any
    if (connect(sock ,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        close(sock);
        exit(0);
        return 1;
    }
    cout<<"Connected\n";
    return true;
}

/**
 * @param command--GET or PUT; from main()
 * @param file--the file to be passed to the server; from main()
 * @param protocol--HTTP/1.1 protocol
 * @return boolean value
 */
bool tcp_client::send_data(string command,string file,string protocol )
{
    //If command is GET, execute this function
    if(!command.compare("GET"))
    {
        string data=command+" "+file+" "+protocol;
        
        //send all the basic client details to server through the socket
        if(send(sock,data.c_str(),strlen( data.c_str() ),0)<0)
        {
            perror("Send failed : ");
            close(sock);
            exit(0);
            return false;
        }
        cout<<"Data sent\n";
    }
    
    //If command is PUT, execute this function
    else if(!command.compare("PUT"))
    {
        /*concatenate the passed parameters to "data" variable and use
         *input file stream object to read the corresponding file*/
        string data=command+" "+file+" "+protocol;
        ifstream openfile (file.c_str());
        string getcontent="";
        
        /*If the file is present*/
        if(openfile.is_open())
        {  
           //stays inside the loop till the end of the file is reached;
           //getline() function gets characters till a CRLF is met in 
           //the file--in short gets each line of the file
           while(!openfile.eof())
           {
               getline(openfile, getcontent);
               cout<<getcontent<<endl;
               data=data+getcontent;
           }
           
           //send the file data through the socket and handle the error, if any
           if( send(sock , data.c_str() , strlen( data.c_str() ) , 0) < 0)
           {
               perror("Sending failed : ");
               close(sock);
               exit(0);
               return false;
           }
           cout<<"Data sent\n";
        } 
        else
        {
            cout<<"File Not Found";
            close(sock);
            exit(0);
        }
    }
    return true;
}

/*
 * This function receives the response from the server 
 *and puts them in the buffer for display
 *--hence the function returns string
 */
string tcp_client::receive(int size=512)
{
    char buffer[size];
    string reply;
    //Receive a reply from the server
    if( recv(sock , buffer , sizeof(buffer) , 0) < 0)
    {
        puts("Reception failed");
    }
    reply = buffer;
    return reply;
}

/**
 * main() function of client
 * @param argc--takes integer value; gives the number of arguments
 * @param argv--double character pointer--has five parameters
 * executable file
 * Server IP address--client IP address
 * Port Number--should correlate with the server port to make a successful connection
 * Command--GET or PUT
 * File--file to send to server
 * @return integer value
 */
int main(int argc, char** argv) 
{
   //Check the number of parameters
    if (argc < 5) 
    {
        fprintf(stderr,"ERROR, One or more of 4 parameters missing!\n");
        exit(1);
    }
    //assign the parameters to local variables
  string serverName = argv[1];
  int port = atoi(argv[2]);
  string command = argv[3];
  string filePath = argv[4];
  tcp_client c;
  cout << "First argument: " << serverName << endl;
  cout << "Second argument: " << port << endl;
  cout << "Third argument: " << command << endl;
  cout << "Fourth argument: " << filePath << endl;
  //connect to host
  c.conn(serverName,port);
  //send some data
  c.send_data(command,filePath,"HTTP/1.1\r\n\r\n");
  //receive and echo reply
  cout<<"----------------------------\n\n";
  cout<<c.receive(1024);
  cout<<"\n\n----------------------------\n\n";
  return 0;
}

