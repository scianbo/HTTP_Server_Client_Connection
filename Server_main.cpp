
/*Header Files*/
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include<iostream>
#include<sstream>
#include<fstream>
#include<csignal>
using namespace std;
int sockfd, newsockfd;

/*Function for displaying corresponding error message*/
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/**
 * Function responsible for handling key-press Interrupts (Ctrl+C)
 * Closes all the active sockets and exits control
 */
void signalHandler( int signum) {
   cout << "Interrupt signal (" << signum << ") received.\n";
   cout<<"Closing Server and Client socket connections\n";
   close(newsockfd);
   close(sockfd);
   exit(signum);  
}

/**
 * @param argc-- takes integer value; gives the number of arguments
 * @param argv-- takes char pointer; server takes one argument: PORT Number
 * @return integer
 */
int main(int argc, char *argv[])
{
	/*Variable Declaration*/
    int portNumber;
    socklen_t clientLength;
    char buffer[256];
    struct sockaddr_in serverAddr, clientAddr;
    int n;
    signal(SIGINT, signalHandler);
	
    /*checks for the PORT number argument*/
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
	
    /*
     * Create Internet domain socket
     *  and checks for error in creating the socket
     */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
       error("ERROR opening socket");
   
   //Fills the entire socket structure with Zeroes
    bzero((char *) &serverAddr, sizeof(serverAddr));
    portNumber = atoi(argv[1]);
	
    /*
     * sets the values of socket structure members
     * and binds them with a defined socket
     */
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(portNumber);
    if (bind(sockfd, (struct sockaddr *) &serverAddr,
             sizeof(serverAddr)) < 0) 
             error("ERROR in binding");
    cout<<"\nListening on port "<<portNumber<<" \n";
	
    /*The server listens for incoming connections with maximum limit upto 5 clients*/
    listen(sockfd,5);
    while(1)
    {
        clientLength = sizeof(clientAddr);
        newsockfd = accept(sockfd, 
                    (struct sockaddr *) &clientAddr, 
                    &clientLength);
        if (newsockfd < 0) 
            error("ERROR on accept");
        bzero(buffer,256);
		
        /*reads the first information on the socket,
         which contains client-side details*/
        n = read(newsockfd,buffer,255);
        buffer[n]=0;
        string request(buffer);
        if (n < 0) 
           error("ERROR reading from socket");
	   
       /*Declare a istringstream object for 
        *dealing with string variables
        *and assign them to separate local string 
        *variables
	*/
        istringstream iss(request);
        int tmpParam=0;
        string command, file, protocol;
        string dataStream="";
        while (iss) 
        {
            string word;
            iss >> word;
            cout << word << endl;
            if  (tmpParam==0)
            {
                command=word;    
            }
            else if (tmpParam==1)
            {
                file=word;
            }
            else if (tmpParam==2)
            {
                protocol=word;
            }
            else if (tmpParam>2)
            {
				dataStream=dataStream+" "+word;
            }
            tmpParam++;   
        }
		
	//When GET is selected from Client
        if(!command.compare("GET"))
        {
            string getcontent = "";
            string writeFileContents = "";
            ifstream openfile(file.c_str());
            if(openfile.is_open())
            {
                cout<<"\nYes, the file:"<<file<<" exists in the Server\n";
                writeFileContents=protocol+" 200 OK\n ";
                writeFileContents="File:"+file+" exists in the Server\n\n";
				
                //Passes server response till the end of file is met
                while(!openfile.eof())
                {
                    getline(openfile, getcontent);
                    cout<<getcontent<<"\n";
                    writeFileContents=writeFileContents+getcontent;
                }
                write(newsockfd,writeFileContents.c_str(),2048);
            } 
            else
            {
                cout<<"\nFile not found on Server";
                write(newsockfd,"404 NOT FOUND",256);
            }
        }
		
	//When PUT is selected from Client
        else if(!command.compare("PUT"))
        {
            /*Declare a output file stream to write the socket data to a file*/
            ofstream myfile;
            myfile.open (file);
            cout << "Writing content to file.\n";
            myfile << dataStream;
			
            //Closes the file and sends response to the client about file creation
            myfile.close();
            write(newsockfd,"200 OK FILE CREATED",256);
        }
        else
        {
            cout<<"\nInvalid Command";
        }
    }
	
    //closes all the open sockets
    cout<<"\nConnection Ended";
    close(newsockfd);
    close(sockfd);
    return 0; 
}
