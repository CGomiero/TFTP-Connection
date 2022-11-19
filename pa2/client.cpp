//Author: Clement Gomiero
//Client.cpp
#include <sys/types.h>	// socket, bind
	#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
	#include <netinet/in.h>   // htonl, htons, inet_ntoa
	#include <arpa/inet.h>	// inet_ntoa
	#include <netdb.h>    	// gethostbyname
	#include <unistd.h>   	// read, write, close
	#include <strings.h>  	// bzero
	#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>  	// writev

#include <iostream>
#include <cstring> //memset
#include <sys/time.h> //gettimeofday()

using namespace std;
const int BUFF_CONST = 1500;

//main: the main method will connect with a server, start a timer to
//determine how long it takes to send it data and for the data to come back
//Preconditions: the main method must take 7 arguments for it to work,
//the 7 arguments are: serverPort, serverName, iterations(same with server),  
//nbufs, bufsize, and the type used to write the data
int main(int argc, char** argv){

  //check amount of arguments
  if(argc != 7){
    cout << "The client did not have the right amount of input arguments." << endl;
    cout << argc << endl;
    return -1;
  }
  
  //storing arguments input
  int serverPort = atoi(argv[1]);
  char* serverName = argv[2];
  int iterations = atoi(argv[3]);
  int nbufs = atoi(argv[4]);
  int bufsize = atoi(argv[5]);
  int type = atoi(argv[6]);

  char databuf[nbufs][bufsize]; // where nbufs * bufsize = 1500
  int clientSd;
  
  //checking ports
  if(serverPort < 0 || serverPort >65535){
    cout << "The port number was not in the range of 0 - 65535." << endl;
    return -1;
  }
  
  //checking iterations
  if(iterations < 0){
    cout << "The number of iterations can not be below 0." << endl;
    return -1;
  }
  
  //nbufs * bufsize must equal 1500
  if((nbufs * bufsize) != BUFF_CONST){
    cout << "nbufs and bufsize are not equal to 1500." << endl;
    return -1;
  }
  
  //check the type is between 1 and 3
  if(type < 1 || type > 3){
    cout << "The type value is incorrect." << endl;
    return -1;
  }
  
  //initialize address info
  struct addrinfo hints;
  struct addrinfo *servInfo;
  memset(&hints, 0, sizeof(hints) );
  hints.ai_family  	= AF_UNSPEC; // Address Family Internet
  hints.ai_socktype	= SOCK_STREAM; // TCP
    
  int rc = getaddrinfo(serverName, argv[1], &hints, &servInfo );
  
  if(rc != 0){
    cout << "ERROR with getaddrinfo()." << endl;
    return -1;
  }
  
  //create socket
  clientSd = socket( servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol );
  rc = connect( clientSd, servInfo->ai_addr, servInfo->ai_addrlen);
  
  if(rc < 0){
    cout << "Connection to the server failed." << endl;
    return -1;
  }
  
  //using timeval to calculate the transfer
  struct timeval start, lap, stop; 
  
  //start the time before executing any for loop
  gettimeofday(&start, NULL);
  
  //type 1: multiple writes
  if(type == 1){
      for ( int i = 0; i < iterations; i++){
        for ( int j = 0; j < nbufs; j++ ){
          write( clientSd, databuf[j], bufsize );	// sd: socket descriptor
      }
    }
  }
  
  //type 2: writev
  if(type == 2){
    for ( int i = 0; i < iterations; i++){
      struct iovec vector[nbufs];
      for ( int j = 0; j < nbufs; j++ ) {
        vector[j].iov_base = databuf[j];
        vector[j].iov_len = bufsize;
      }
    writev( clientSd, vector, nbufs );       	// sd: socket descriptor
    }
  }
  
  //type 3: write
  if(type == 3){
    for ( int i = 0; i < iterations; i++){
      write( clientSd, databuf, nbufs * bufsize ); // sd: socket descriptor
    }
  }  
  //after it is done writing, it is the first lap
  gettimeofday(&lap, NULL);
  
  //read answer from server
  int serverCount = 0;
  read(clientSd, &serverCount, sizeof(serverCount));
  
  gettimeofday(&stop, NULL);
  
  //calculate the amount of  time it took to transmit and receive the data from the server
  long dataTransmissionTime = ((lap.tv_sec - start.tv_sec) * 1000000) + (lap.tv_usec - start.tv_usec);
  long roundTripTime = ((stop.tv_sec - start.tv_sec) * 1000000) + (stop.tv_usec - start.tv_usec);
  
  cout << "repetition = " << iterations << " nbufs = " << nbufs << " bufsize = " << bufsize << " type = " << type << endl;
  cout << "Test " << type << ": data-sending time = " << dataTransmissionTime << " usec, round-trip time = " << roundTripTime << " usec, " << serverCount << " times." << endl;
  close(clientSd);
}
