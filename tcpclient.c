#include <stdio.h>	/* standard C i/o facilities */
#include <stdlib.h>	/* needed for atoi() */
#include <unistd.h>	/* Unix System Calls */
#include <sys/types.h>	/* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>	/* IP address conversion stuff */
#include <string.h>

#include <netdb.h>	/* Name lookups (gethostbyname) */
/* This is the same as "client.c", except that it allows the
user to specify a hostname on the command line or an IP address
(in dotted-decimal notation).

/* tcpclient program:

   The following must passed in on the command line:

      name of the server (argv[1])
      port number of the server (argv[2])
      file name (argv[3])
*/

int main( int argc, char **argv ) {

  int sk;
  struct sockaddr_in skaddr;
  struct hostent *hp;		/* used for name lookup */
  char key[1];

  FILE *file;
  file = fopen(argv[3], "r");
  if(file == NULL){
    printf("File failed to load.\n");
    exit(1);
  }


  char buff[1000];
  char ran[1000];
  char *arrAddr = buff;
  size_t size = 0;
  char *nread = NULL;


  /* first - check to make sure there are 3 command line parameters
     (argc=4 since the program name is argv[0])
  */

  if (argc!=4) {
    printf("Usage: client <server name> <server port> <file name>\n");
    exit(0);
  }

  /* create a socket
     IP protocol family (PF_INET)
     TCP protocol (SOCK_STREAM)
  */

  if ((sk = socket( PF_INET, SOCK_STREAM, 0 )) < 0) {
    printf("Problem creating socket\n");
    exit(1);
  }

  /* fill in an address structure that will be used to specify
     the address of the server we want to connect to

     address family is IP  (AF_INET)

     server IP address is found by calling gethostbyname with the
     name of the server (entered on the command line)

     server port number is argv[2] (entered on the command line)
  */

  skaddr.sin_family = AF_INET;

  /* convert argv[1] to a network byte order binary IP address */
  /* First try to convert using gethostbyname */
  if ((hp = gethostbyname(argv[1]))!=0) {
    /* Name lookup was successful - copy the IP address */
    memcpy( &skaddr.sin_addr.s_addr, hp->h_addr, hp->h_length);
  } else {

    /* Name lookup didn't work, try converting from dotted decimal */

#ifndef SUN
    if (inet_aton(argv[1],&skaddr.sin_addr)==0) {
      printf("Invalid IP address: %s\n",argv[1]);
      exit(1);
    }
#else
    /*inet_aton is missing on Solaris - you need to use inet_addr! */
    /* inet_addr is not as nice, the return value is -1 if it fails
       (so we need to assume that is not the right address !)
    */

    skaddr.sin_addr.s_addr = inet_addr(argv[1]);
    if (skaddr.sin_addr.s_addr ==-1) {
      printf("Invalid IP address: %s\n",argv[1]);
      exit(1);
    }
#endif
  }

  skaddr.sin_port = htons(atoi(argv[2]));


  /* attempt to establish a connection with the server */
  if (connect(sk,(struct sockaddr *) &skaddr,sizeof(skaddr)) < 0 ) {
    printf("Problem connecting socket\n");
    exit(1);
  }

  write(sk, argv[3], strlen(argv[3])+1);

  read(sk, buff, 1000);

  key[0] = buff[0];

  //Checks ACK and starts reading from new file
  while(buff[0] == '>' && getline(&arrAddr, &size, file) != -1){


    write(sk, arrAddr, strlen(arrAddr));

    //The acknowledgment key from the server
    read(sk, key, 1);

    //If error in transmission and key doesn't match print error to screen & close
    if(key[0] != '>'){
      printf("Error Message: ACK key does not match. Killing connection.\n");
      break;
    }

 }

  //Close file and socket
  int fclose(FILE *file);
  close(sk);
  return(0);
}
