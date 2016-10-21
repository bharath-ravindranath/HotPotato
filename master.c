#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define BUFF_SIZE 40960

struct client_info{
	int p,s, player_socket;
	struct hostent *ihp;
	char player_hostname[64];
	int player_listen_port;
};

typedef struct client_info CLIENT;



void play(CLIENT *players, int number_of_players, int hops){
	int i=0, rc,len,sending_to;
	char buff[BUFF_SIZE];
	fd_set fds;
	time_t t;
	
	FD_ZERO(&fds);

	if(hops == 0){
		printf("All players present, hop count is 0. Ending game\n");
		for(i = 0; i < number_of_players; i++){
			len = send(players[i].p, "close", 5, 0);
			if (len != 5) {
			    perror("send");
			    exit(1);
			}
		}
		return;
	}

	sending_to = rand()%number_of_players;
	printf("All players present, sending potato to player %d\n",sending_to);

	sprintf(buff," %d", hops);
	len = send(players[sending_to].p, buff, strlen(buff), 0);
	if (len != strlen(buff)) {
	    perror("send");
	    exit(1);
	}

	for(i = 0; i < number_of_players; i++){
		FD_SET(players[i].p, &fds);
	}
	rc = select(sizeof(fds)*4, &fds, NULL, NULL, NULL);
	if(rc == -1){
		perror("select ");
		return ;
	}

	for(i = 0; i < number_of_players; i++){
		if(FD_ISSET(players[i].p, &fds)) break;
	}

	len = recv(players[i].p, buff, BUFF_SIZE, 0);
    if ( len < 0 ) {
  		perror("recv");
  		exit(1);
  	}
	buff[len] = '\0';
	printf("Trace of Potato:\n%s\n", buff);

	for(i = 0; i < number_of_players; i++){
		len = send(players[i].p, "close", 5, 0);
		if (len != 5) {
		    perror("send");
		    exit(1);
		}
	}
}




CLIENT accept_connections(CLIENT player, int i, int s, int len){

	struct sockaddr_in incoming;
    player.p = accept(s, (struct sockaddr *)&(incoming), &len);
    player.ihp = gethostbyaddr((char *)&(incoming).sin_addr, 
      sizeof(struct in_addr), AF_INET);

    printf("Player %d is on %s\n", i, (player.ihp)->h_name);
    return player;
}



void send_neighbor_info(CLIENT player, CLIENT neighbor){
	int len;
	char buff[BUFF_SIZE];


}

int main(int argc, char *argv[]){
	CLIENT *clients ;
	char buff[BUFF_SIZE], host[64], buff2[BUFF_SIZE];
	char temp[8], host_buff[64];
	int s, p, fp, rc, len, port, i, j, k;
	struct hostent *hp, *ihp;
	struct sockaddr_in sin, incoming;
	int number_of_players, hops;

	srand(time(NULL));

	if ( argc < 4 ){
    	fprintf(stderr, "Usage: %s <port-number> <number-of-players> <hops>\n", argv[0]);

    	exit(1);
	}
	port = atoi(argv[1]);
	number_of_players = atoi(argv[2]);
	if(number_of_players <= 1){
		fprintf(stderr, "Number of players should be atleast 2 \n" );
		exit(1);
	}
	hops = atoi(argv[3]);
	if(hops < 0){
		fprintf(stderr, "Number of hops should be atleast 0\n" );
	}

	gethostname(host, sizeof host);
  	hp = gethostbyname(host);
  	if ( hp == NULL ) {
    	fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
    	exit(1);
    }
    
    printf("Potato Master on %s\n", host);
    printf("Players = %d\n", number_of_players);
    printf("Hops = %d\n", hops);
    s = socket(AF_INET, SOCK_STREAM, 0);
  	if ( s < 0 ) {
    	perror("socket:");
    	exit(s);
    }

    int optval = 1;
    rc = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if ( rc < 0 ) {
    	perror("sockopt:");
    	exit(rc);
    }
    /* set up the address and port */
  	sin.sin_family = AF_INET;
  	sin.sin_port = htons(port);
  	memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  
  	/* bind socket s to address sin */
  	rc = bind(s, (struct sockaddr *)&sin, sizeof(sin));
  	if ( rc < 0 ) {
    	perror("bind:");
    	exit(rc);
    }


    rc = listen(s, 5);
  	if ( rc < 0 ) {
    	perror("listen:");
    	exit(rc);
  	}


  	clients = malloc(number_of_players*sizeof( struct client_info));

  	for(i = 0; i < number_of_players; i++){
  		clients[i].s = s;
  		clients[i] = accept_connections(clients[i], i, s, sizeof(sin));
  		sprintf(buff,"%d",i);
		len = send(clients[i].p, buff, strlen(buff), 0);
		if (len != strlen(buff)) {
		    perror("send");
		    exit(1);
		}

  	}

  	
  	// For each player send its player number
  	/*for(i = 0; i < number_of_players; i++){
  		sprintf(buff,"%d",i);
		len = send(clients[i].p, buff, strlen(buff), 0);
		if (len != strlen(buff)) {
		    perror("send");
		    exit(1);
		}
  	}*/

  	for(i = 0; i < number_of_players; i++){
  		// printf("%d\n",i);
  		len = recv(clients[i].p, buff, 32, 0);
      	if ( len < 0 ) {
			perror("recv");
			exit(1);
		}
		buff[len] = '\0';
		// printf("Received string %d: %s\n",i,buff);
		for(j = 0; j < len; j++){
			if(buff[j] == ' ') break;
			temp[j] = buff[j];
		}
		//printf("I am here\n");
		temp[j] = '\0';
		j++;
		clients[i].player_listen_port = atoi(temp);
		for(k = 0; j < len; k++,j++){
			host_buff[k] = buff[j];
		}
		host_buff[k] = '\0';
		strcpy(clients[i].player_hostname, host_buff);
		
  	}

  	for(i = 0; i < number_of_players; i++){
  		sprintf(buff,"%d %s %d", clients[(i+1)%number_of_players].player_listen_port, 
  			clients[(i+1)%number_of_players].player_hostname, number_of_players);
		len = send(clients[i].p, buff, strlen(buff), 0);
		if (len != strlen(buff)) {
		    perror("send");
		    exit(1);
		}
  	}

  	for(i = 0 ; i < number_of_players; i++){
  		len = recv(clients[i].p, buff, 32, 0);
      	if ( len < 0 ) {
			perror("recv");
			exit(1);
		}
		buff[len] = '\0';
		// printf("Received string %d: %s\n",i,buff);
  	}

  	play(clients, number_of_players, hops);
  	//sleep(20);
  	close(s);
  	return ;

}