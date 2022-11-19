//Author: Clement Gomiero
//Server.cpp
#include <sys/types.h>	// socket, bind
	#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
	#include <netinet/in.h>   // htonl, htons, inet_ntoa
	#include <arpa/inet.h>	// inet_ntoa
	#include <netdb.h>    	// gethostbyname
	#include <unistd.h>   	// read, write, close
	#include <strings.h>  	// bzero
	#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>  	// writev

#include <pthread.h>
#include <cstdlib>
#include <iostream>
#include <cstring> //memset
#include <sys/time.h> //gettimeofday()

using namespace std;

//the array has to be of size 1500 and the 
//amount of connections/threads is set to 10
const int BUFF_CONST = 1500;
const int MAX_CONNECTION = 10;

//number of thread
int t_num = 0;

//variables about the server  
int  iterations;
int port; 
int serverSd;
int newSd;

//serverConnection: the method create the array, read the data from the client
//and get the amount of time it took to read the data. Write the amount of time back
//to the client
//Preconditions: The method is to be used with a new thread, as the server is multithreaded.
//The method takes in a void pointer which is the address of the newSd
//Postconditions: close the current and terminate the thread
void* serverConnection(void* t){
cout << "Connection number: " << t_num << "        ";
  char databuf[BUFF_CONST];
  int currentSd = *(int*) t;
  //get the time variables and start the timer
  struct timeval start, stop;
  gettimeofday(&start, NULL);
  
  //read the data send from the client
  int count = 0;
  for ( int i = 0 ; i < iterations; i++ ) {
    for ( int nRead = 0;
       ( nRead += read(currentSd, databuf, BUFF_CONST - nRead ) ) < BUFF_CONST;    
 	  ++count );
  }
  
  //stop the timer
  gettimeofday(&stop, NULL);
  
  //calculate the amount of time it took to read the data
  long dataReceivingTime = ((stop.tv_sec - start.tv_sec) * 1000000) + (stop.tv_usec - start.tv_usec);
  cout << "Data Receiving Time = " << dataReceivingTime << " usec" << endl;
  
  //send the number of read that is showed with the count variable
  write( currentSd, &count, sizeof(count));
  
  //close Sd and terminate thread
  close(currentSd);
  pthread_exit(NULL);
}

//main: The main will create a server to make TCP connections with clients
//Preconditions: The server must receive a port number and the number 
//of iterations that is the amount of time it will read what the client sent.
//(The number of iterations must be the same as the client)
//PostConditions: The server will keep running until shut down, 
//while the server run, it will look for any client connections
//and run the serverConnection method in a new thread when it finds one.
int main(int argc, char** argv){
  pthread_t threads[MAX_CONNECTION];

  //checking arguments amount
  if(argc != 3){
    cout << "The number of input is incorrect." << endl;
    return -1;
  }
  
  //storing command line input
  port = atoi(argv[1]);
  iterations = atoi(argv[2]);
  
  //checking port range
  if(port < 0 || port >65535){
    cout << "The port number was not in the range of 0 - 65535." << endl;
    return -1;
  }
  
  //iterations must be above zero
  if(iterations < 0){
    cout << "The number of iterations can not be below 0." << endl;
    return -1;
  }
  
  //initialize address info
 struct addrinfo hints, *res;
  memset( &hints, 0, sizeof (hints) );
  hints.ai_family  	= AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  getaddrinfo(NULL, argv[1], &hints, &res);

  //while loop that will run forever
  while(1){
  //create new socket
  serverSd = socket(res->ai_family, res->ai_socktype, res->ai_protocol );
  const int yes = 1;
  setsockopt( serverSd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof( yes ) );

  bind( serverSd, res->ai_addr, res->ai_addrlen );

  //allow the server to listen to 10 different connection request
  listen( serverSd, MAX_CONNECTION );

  //receive a request from a client and accept it
  struct sockaddr_storage newSockAddr;
  socklen_t newSockAddrSize = sizeof( newSockAddr );
  newSd = accept( serverSd, (struct sockaddr *)&newSockAddr, &newSockAddrSize );
  
  //make sure the connection worked otherwise next iteration
  if(newSd < 0){  
    cout << "Client connection failed." << endl;
    continue;
  }
  
  //create a new thread to execute the method
  //The thread method will take the reference of the newSd, in case other
  //clients make a connection at the same time
  pthread_create (&threads[t_num++], NULL, &serverConnection, &newSd);
  if(t_num == MAX_CONNECTION){
    t_num = 0;
  }
  
  //close the serverSd so it can acccept a new one
  close(serverSd);
  }
  
  return -1;
}