#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#define BUFF_SIZE 40960

#define LEN	64

int player_number,number_of_players;

struct connections{
	int s, p;
	struct hostent *ihp;
  	struct sockaddr_in sin;
	char hostname[64];
	int port;
};

int count = 0;

void play(struct connections master, struct connections left_player, struct connections right_player){
	fd_set fds;
	int rc,len, temp_fd, hops, i, j;
	char buf[BUFF_SIZE],c;
	char temp2[BUFF_SIZE], str_hops[5];

	
	while(1){

		FD_ZERO(&fds);
		FD_SET(right_player.s, &fds);
		FD_SET(master.s, &fds);
		FD_SET(left_player.p, &fds);
		
		rc = select(sizeof(fds)*4, &fds, NULL, NULL, NULL);
		if(rc == -1){
			perror("select ");
			return ;
		}
		if(FD_ISSET(master.s, &fds)) temp_fd = master.s;
	  	
	  	else if(FD_ISSET(right_player.s, &fds))	temp_fd = right_player.s;	

	  	else temp_fd = left_player.p;
	  	
	  	len = recv(temp_fd, buf, BUFF_SIZE, 0);
	    if ( len < 0 ) {
	  		perror("recv");
	  		exit(1);
	  	}
		buf[len] = '\0';
		// printf("%s\n", buf);

		if ( !strcmp("close", buf) ) return;

		else{

			for(i = 0; buf[i] != ' '; i++) temp2[i] = buf[i];
			temp2[i++] = '\0';
			for(j = 0; buf[i] != '\0'; i++, j++) str_hops[j] = buf[i];
			str_hops[j] = '\0';
			hops = atoi(str_hops);

			if(--hops == 0){
				printf("I am it!\n");
				if(temp2[0] == '\0')	sprintf(buf,"%d", player_number);	
				
				else sprintf(buf,"%s,%d", temp2,player_number);

				len = send(master.s, buf, strlen(buf), 0);
				if (len != strlen(buf)) {
			    	perror("send");
			    	exit(1);
				}
				//play(master, left_player, right_player);
			}
			else{
				if(rand()%2){
					temp_fd = right_player.s;
					printf("Sending potato to %d\n", (player_number+1)%number_of_players);
				}
				else {
					temp_fd = left_player.p;
					printf("Sending potato to %d\n", (player_number-1+number_of_players)%number_of_players );
				}

				if(temp2[0] == '\0')	sprintf(buf,"%d %d", player_number,hops);

				else sprintf(buf,"%s,%d %d", temp2,player_number,hops);

				len = send(temp_fd, buf, strlen(buf), 0);
				if (len != strlen(buf)) {
			    	perror("send");
			    	exit(1);
				}
		//		play(master, left_player, right_player);	
			}
		}
	}
}

struct connections parse_string(struct connections right_player, char *buf){
	// printf("%s\n",buf);
	int i=0, j, k, l;
	char port[8], hostname[64], num[10];
	for(;buf[i] != ' ';i++)	port[i] = buf[i];
	
	port[i] = '\0';
	i++;
	
	for(j=0; buf[i] != ' ';j++,i++) hostname[j] = buf[i];

	hostname[j] = '\0';
	i++;

	for(j=0; buf[i] != '\0'; j++,i++) num[j] = buf[i];

	num[j] = '\0';

	strcpy(right_player.hostname, hostname);
	right_player.port = atoi(port);
	number_of_players = atoi(num);

	return right_player;
}

struct connections create_another_socket(struct connections temp){
	int s, rc, len;
	struct hostent *hp;
	int success=0;
	struct in_addr  **pptr;
	
	gethostname(temp.hostname, sizeof temp.hostname);
    hp = gethostbyname(temp.hostname);
    if ( hp == NULL ) {
    	fprintf(stderr, " host not found (%s)\n", temp.hostname);
    	exit(1);
  	}
	temp.port = 1025 + rand()%10000;

	temp.s = socket(AF_INET, SOCK_STREAM, 0);
  	if ( temp.s < 0 ) {
    	perror("socket:");
    	exit(s);
  	}

/*  	int optval = 1;
	rc = setsockopt(temp.s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if ( rc < 0 ) {
    	perror("sockopt:");
    	exit(rc);
    }
*/
  	temp.sin.sin_family = AF_INET;
  	while(!success){
  		temp.sin.sin_port = htons(temp.port);
  		memcpy(&(temp.sin.sin_addr), hp->h_addr_list[0], hp->h_length);
  
  		/* bind socket s to address sin */
  		rc = bind(temp.s, (struct sockaddr *)&(temp.sin), sizeof(temp.sin));
  		if ( rc < 0 ) {
  			temp.port++;
  		}
  		else success = 1;	
  	}
  	
  	rc = listen(temp.s, 5);
  	if ( rc < 0 ) {
    	perror("listen:");
    	exit(rc);
  	}
  	return temp;
}


struct connections connect_to(char * argv[], struct connections temp){

	int rc;
	struct hostent *hp;

	hp = gethostbyname(temp.hostname); 
    if ( hp == NULL ) {
    	fprintf(stderr, "%s: host not found (%s)\n", argv[0], temp.hostname);
    	exit(1);
  	}

  	temp.s = socket(AF_INET, SOCK_STREAM, 0);
  	if ( temp.s < 0 ) {
    	perror("socket:");
    	exit(temp.s);
  	}

  	temp.sin.sin_family = AF_INET;
  	temp.sin.sin_port = htons(temp.port);
  	memcpy(&(temp.sin.sin_addr), hp->h_addr_list[0], hp->h_length);
  


  	rc = connect(temp.s, (struct sockaddr *)&(temp.sin), sizeof(temp.sin));
  	if ( rc < 0 ) {
    	perror("connect:");
    	exit(rc);
    }

    return temp;

}


int main(int argc, char *argv[]){

  	struct connections master, left_player, right_player;
  	
  	char buf[BUFF_SIZE], buf2[BUFF_SIZE];
  	int success, len, rc;
  	struct hostent *ihp;
  	struct sockaddr_in incoming;

  	srand(time(NULL));

  	if ( argc != 3 ) {
  		fprintf(stderr, "Usage: %s <master-machine-name> <port-number>\n", argv[0]);
    	exit(1);
    }

    strcpy(master.hostname, argv[1]);
  	master.port = atoi(argv[2]);
  	master = connect_to(argv, master);
  

  	len = recv(master.s, buf, 32, 0);
    if ( len < 0 ) {
  		perror("recv");
  		exit(1);
  	}
  	buf[len] = '\0';
  	player_number = atoi(buf);
  	printf("Connected as player %d\n",player_number);
    
    /*Create one more socket on all players*/
	left_player = create_another_socket(left_player);
  	sprintf(buf,"%d %s", left_player.port,left_player.hostname );
  	len = send(master.s, buf, strlen(buf), 0);

  	len = recv(master.s, buf2, 32, 0);
    if ( len < 0 ) {
  		perror("recv");
  		exit(1);
  	}
  	buf2[len] = '\0';
  	
  	right_player = parse_string(right_player, buf2);
  	//printf("%s %d\n", right_player.hostname, right_player.port);
  	right_player = connect_to(argv, right_player);


	sprintf(buf,"done");
  	len = send(master.s, buf, strlen(buf), 0);

	if(player_number == 0){
  		rc = connect(right_player.s, (struct sockaddr *)&(right_player.sin), sizeof(right_player.sin));
  	
  		left_player.p = accept(left_player.s, (struct sockaddr *)&(left_player.sin), &len);
  	}
  	
  	else{
  		left_player.p = accept(left_player.s, (struct sockaddr *)&(left_player.sin), &len);

  		rc = connect(right_player.s, (struct sockaddr *)&(right_player.sin), sizeof(right_player.sin));
  	}

  	play(master,left_player,right_player);
  	
  	//sleep(10);
  	close(master.s);
  	close(left_player.s);
    exit(0);
}

