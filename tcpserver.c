#include <stdio.h>	/* standard C i/o facilities */
#include <unistd.h>	/* Unix System Calls */
#include <sys/types.h>	/* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>	/* IP address conversion stuff */
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
/* Server main routine - this is an iterative server

   1. create a socket
   2. bind the socket and print out the port number assigned
   3. put the socket into passive mode (listen)
   4. do forever
        get next connection
        handle the connection
      enddo
*/

void *child_thread(void *);


int main(int argc, char **argv) {

  int ld,sd;
  struct sockaddr_in skaddr;
  struct sockaddr_in from;
  int addrlen,length;
  pthread_t thread;




/* create a socket
       IP protocol family (PF_INET)
       TCP protocol (SOCK_STREAM)
*/

  if ((ld = socket( PF_INET, SOCK_STREAM, 0 )) < 0) {
    perror("Problem creating socket\n");
    exit(1);
  }

/* establish our address
   address family is AF_INET
   our IP address is INADDR_ANY (any of our IP addresses)
   the port number is assigned by the kernel
*/

  skaddr.sin_family = AF_INET;
  skaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  skaddr.sin_port = htons(atoi(argv[1]));

  if (bind(ld, (struct sockaddr *) &skaddr, sizeof(skaddr))<0) {
    perror("Problem binding\n");
    exit(0);
  }

/* find out what port we were assigned and print it out */

  length = sizeof( skaddr );
  if (getsockname(ld, (struct sockaddr *) &skaddr, &length)<0) {
    perror("Error getsockname\n");
    exit(1);
  }
  printf("The Server passive socket port number is %d\n",ntohs(skaddr.sin_port));

/* put the socket into passive mode (waiting for connections) */

  if (listen(ld,5) < 0 ) {
    perror("Error calling listen\n");
    exit(1);
  }

  //Keeps the connection open for incoming clients
  while(1){

    printf("Ready for a connection...\n");
    addrlen=sizeof(skaddr);
    if ( (sd = accept( ld, (struct sockaddr*) &from, &addrlen)) < 0) {
      perror("Problem with accept call\n");
      exit(1);
    }

    printf("Got a connection - processing...\n");

    /* Determine and print out the address of the new
       server socket */

    length = sizeof( skaddr );
    if (getsockname(sd, (struct sockaddr *) &skaddr, &length)<0) {
      perror("Error getsockname\n");
      exit(1);
    }
    printf("The active server port number is %d\n",ntohs(skaddr.sin_port));
    printf("The active server IP ADDRESS is %s\n",inet_ntoa(skaddr.sin_addr));

    /* print out the address of the client  */

    printf("The client port number is %d\n",ntohs(from.sin_port));
    printf("The client IP ADDRESS is %s\n",inet_ntoa(from.sin_addr));

      //This creates a new thread based on function child_thread
      if((pthread_create(&thread, NULL, child_thread, (void*) &sd)) != 0){

        //If thread fails to create
        printf("Error with creating child thread\n");
        return 0;
      }

    }
}

//A function that will be multi threaded
void *child_thread(void *tempsock){

  //Thread detachs itself from parent thread
  pthread_detach(pthread_self());
  int newsock = *(int*) tempsock;
  char buff[100];
  char convert[100];
  char key = '>';
  size_t size = 0;
  int n;
  struct timeval now;
  gettimeofday(&now, NULL);
  int i = 0;
  int index = 0;

  //Prints the
  sprintf(convert, "%d", (int)now.tv_usec);

  read(newsock,buff,1000);

  int temp = strlen(buff);
  char tempArr[1000];

  //Places the timestamp on the file name
  int tempval = strlen(buff);
  for(i = strlen(buff); i < strlen(buff) + strlen(convert); i++){
    buff[i] = convert[index];
    index++;
  }

  //Opens the new file for writing
  FILE *ptr;
  ptr = fopen(buff, "w");

  //Places key at front of file name.
  tempArr[0] = key;
  for(i = 1; i <strlen(buff) + strlen(convert); i++){
    tempArr[i] = buff[i-1];
  }

  write(newsock, tempArr, 1000);

  //Clears both buff and temp
  for(i = 0; i < 1000; i++){
    buff[i] = '\0';
    tempArr[i] = '\0';
  }

  //Reads line by line into buff and reverses buff
  while ((n=read(newsock,buff,1000))>0) {

    if(buff[n-1] == '\n'){
      int y = n - 1;
      int x = 0;
      char temp;
      while(x < y) {
        temp = buff[x];
        buff[x] = buff[y];
        buff[y] = temp;
        x++;
        y--;

      }

      //Prints out to the file from buff
      write(fileno(ptr),buff,n);
      
    }

  //Writing the acknowledgement of the TCP Handshake
    write(newsock,">",1);

  }

  //Closes the file
  fclose(ptr);

  printf("\nDone with connection - waiting\n\n\n");

  //Closes socket
  close(newsock);

  for(i = 0; i < 1000; i++){
    buff[i] = '\0';
    }
  }
