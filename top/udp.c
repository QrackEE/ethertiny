//http://www.cs.ucsb.edu/~almeroth/classes/W01.176B/hw2/examples/udp-server.c

/* Sample UDP server */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   socklen_t len;
   char mesg[1000];

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(13312);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

   for (;;)
   {
      len = sizeof(cliaddr);
      n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
      sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
      mesg[n] = 0;
//printf( "MARK\n" );
		int j;
	for(j = 0; j < n;j ++)
		{
			unsigned r = mesg[j];
			int k;
//			for( k = 1; k != 0x100; k<<=1 )
			for( k = 0x80; k; k>>=1 )
				printf( "%d", (r&k)?1:0 );
//			printf( "%02x", (unsigned char)r );
			printf( " " );
		}
		printf( "\n" );
/*
		if(mesg[0] == 'C' ) printf( "." );
		else
		      printf("%s",mesg);
			fflush( stdout );
//		if(mesg[0] == 'C' ) printf( "\n" );
*/
   }
}

