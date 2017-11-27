/**********************************************************************
* Alieta Train 
* CS 372 
* Oregon State Univeristy 
* November 2017 
* ftserver written in C 
* starts on HOST A validates command line parameters <SERVER_PORT>
* waits on <PORTNUM> for a client request
* establishes TCP control connection on <SERVER_PORT> 'connection P'
* waits on connection p for ftclient to send command 
* receives commands on connection P 
*************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

struct addrinfo * make_addr_info(char * portno);
struct addrinfo * make_addr_ip_info(char * ip_addr, char * port);
int make_socket(struct addrinfo * res);
void make_connection(int sockfd, struct addrinfo * res);
void make_bind(int sockfd, struct addrinfo * res);
void listen_socket(int sockfd);
void send_dir(char * ip_address, char * port, char ** files, int num_files);
void client_req(int new_fd);
void waiting(int sockfd);
int get_files(char ** files);
char ** get_file_array(int size);
void remove_file_array(char ** array, int size);
int check_exists(char ** files, int num_files, char * filename);
void send_fi(char * ip_address, char * port, char * filename);


// initiates a TCP data connection with ftclient on <DATA_PORT>. (Call this connection Q)
// If ftclient has sent the -l command, ftserver sends its directory to ftclient on connection Q, and ftclient displays the directory on-screen.
// If ftclient has sent -g <FILENAME>, 
	//ftserver validates FILENAME
	// AND EITHER
		// sends the contents of FILENAME on connection Q. ftclient saves the 
		// file in the current default directory (handling "duplicate file name" 
		// error if necessary), and displays a "transfer complete" message on-screen
	// - OR -
		// sends an appropriate error message (“File not found”, etc.) to ftclient 
		// on connection P, and ftclient displays the message on-screen.
// ftserver closes connection Q (don’t leave open sockets!)


// ftserver repeats from 2 (above) until terminated by a supervisor (SIGINT).

/*****************************************************
* MAIN METHOD 
* SET UP SERVER SOCKET + ARGS 
* WAIT FOR CLIENT CONNECTION 
*****************************************************/

int main(int argc, char *argv[]) {
	// first check arguments
	if(argc != 2){
		fprintf(stderr, "Error: wrong number of arguments\n");
		exit(1);
	}
	// if no
	struct addrinfo * results = make_addr_info(argv[1]);
	int sockfd = make_socket(results);
	make_bind(sockfd, results);
	listen_socket(sockfd);
	printf("Message: Server open on port %s\n", argv[1]);
	waiting(sockfd);
	freeaddrinfo(results);
}


/********************************************************************
* struct addrinfo * make_addr_info(char*)
* make pointer to addr info linked list with port 
* Takes 1 string: port num 
* Response: addr info linked list
* source: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#getaddrinfo 
*********************************************************************/
struct addrinfo * make_addr_info(char * portno) {
	
	struct addrinfo * results;
	struct addrinfo hints;
	int state;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	state = getaddrinfo(NULL, portno, &hints, &results);

	// if you've received a non zero return, print error message
	if(state != 0) {
		fprintf(stderr, "ERROR: check port number", gai_strerror(state));
		exit(1);
	}
	return results;
}

/********************************************************************
* struct addrinfo * make_addr_info(char*)
* make pointer to addr info linked list with port 
* Takes 2 strings: addr + port num 
* Response: addr info linked list 
*********************************************************************/
struct addrinfo * make_addr_ip_info(char * ip_addr, char * portno) {
	struct addrinfo hints;
	struct addrinfo * results; // service info list
	int state;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	state = getaddrinfo(ip_addr, portno, &hints, &results);
	
	if( state != 0) {
		fprintf(stderr, "Error: %s \nDid you enter the right IP address and port combination?\n", gai_strerror(state));
		exit(1);
	}
	return results;
}

/********************************************************************
* int create_socket(struct addrinfo * )
* make the socket  from addr info linked list
* Takes linked list 
* Response: socket file description
*********************************************************************/
int make_socket(struct addrinfo * results) {
	int sockfd;
	sockfd = socket((struct addrinfo * )(results)->ai_family, results->ai_socktype, results->ai_protocol);
	if(sockfd == -1) {
		fprintf(stderr, "Error: unable to create socket\n");
		exit(1);
	}
	return sockfd;
}

/********************************************************************
* void connect_socket(int, struct addrinfo * )
* 
* connects socket to linked list address info 
* takes a socket file descrptor and linked list of address info
*********************************************************************/
void make_connection(int sockfd, struct addrinfo * results){
	int state;
	state = connect(sockfd, results -> ai_addr, results->ai_addrlen); 
	if(state == -1) {
		fprintf(stderr, "Error: unable to connect socket\n");
		exit(1);
	}
}

/********************************************************************
* void bind_socket(int, struct addrinfo *)
* binds port with socket
* takes socket file descriptor and linked list of address info
*********************************************************************/
void make_bind(int sockfd, struct addrinfo * results){
	if(bind(sockfd, results->ai_addr, results->ai_addrlen)== -1) {
		close(sockfd);
		fprintf(stderr, "Error: unable to bind socket\n");
		exit(1);
	}

}

/********************************************************************
* void listen_socket(int)
* listen on the port
* takes socket file descriptor (sockfd)
* https://www.systutorials.com/docs/linux/man/2-listen/
*********************************************************************/
void listen_socket(int sockfd){
	if(listen(sockfd, 5) == -1) {
		close(sockfd);
		fprintf(stderr, "Error: unable to listen on socket\n");
		exit(1);
	}
}


/********************************************************************
* void send_dir(char *, char *, char **, int)
* send list of files in directory  to the client 
* takes ip, port, list of files, num of files, 
*********************************************************************/
void send_dir(char * ip_address, char * portno, char ** file_list, int file_count){
	
	sleep(2);
	char * done_message = "done";
	int i = 0;
	struct addrinfo * results = make_addr_ip_info(ip_address, portno);
	int data_socket = make_socket(results);
	make_connection(data_socket, results);
	
	for(i; i<file_count; i++) {
		send(data_socket, file_list[i], 100, 0);
	}

	send(data_socket, done_message, strlen(done_message), 0);
	close(data_socket);
	freeaddrinfo(results);
}

/********************************************************************
* void client_req(int)
* proceses requests from client 
* takes the socket we created earlier
* http://beej.us/guide/bgnet/output/html/multipage/clientserver.html 
*********************************************************************/
void client_req(int new_fd){
 
	 // prep some messages 
	char * success_msg = "ok";
	char * failure_msg = "nope";
	char * unfound_msg = "File not found";
	char * found_msg = "File found";

	char portno[100];
	char command[100];
	char ip_address[100];

	memset(portno, 0, sizeof(portno));
	recv(new_fd, portno, sizeof(portno)-1, 0);
	send(new_fd, success_msg, strlen(success_msg),0);

	memset(command, 0, sizeof(command));
	recv(new_fd, command, sizeof(command)-1, 0);
	send(new_fd, success_msg, strlen(success_msg), 0);

	memset(ip_address,0,sizeof(ip_address));
	recv(new_fd, ip_address, sizeof(ip_address) -1, 0);
	printf("Connection coming in from %s\n", ip_address);	
	
	/* case for file transfer */
	if (strcmp(command, "g") == 0) {
		send(new_fd, success_msg, strlen(success_msg), 0);
		char filename[100];
	
		memset(filename, 0, sizeof(filename));
		recv(new_fd, filename, sizeof(filename) -1, 0);
		printf("Info: %s requested on port number %s\n", filename, portno);
	
		char ** file_list = get_file_array(100);
		int file_count  = get_files(file_list);
		int contains_file = check_exists(file_list, file_count, filename);
	
	/* if exists, proceed */
		if (contains_file) { 
			printf("Info: Sending %s to client on port %s \n", filename, portno);
			send(new_fd, found_msg, strlen(found_msg), 0);
			char new_filename[100];
			memset(new_filename, 0, sizeof(new_filename));
			strcpy(new_filename, "./");
			char * end = new_filename + strlen(new_filename);
			end += sprintf(end, "%s", filename);
			send_fi(ip_address, portno, new_filename);
		}
		/* if doesn't pass check for contains */ 
		else {
			printf("Info: file %s not found, sending error message to client\n", filename);
			send(new_fd, unfound_msg, 100, 0);
		}
		remove_file_array(file_list, 100);
	}

	/* handle request for file list */
  else if (strcmp(command, "l") == 0) {
		send(new_fd, success_msg, strlen(success_msg), 0);
		printf("Info: List directory requested on port %s\n", portno);
		printf("Info: Sending directory contents to %s on port number %s\n", ip_address, portno);
		char ** file_list = get_file_array(100);
		int num_files = get_files(file_list);
		send_dir(ip_address, portno, file_list, num_files);
		remove_file_array(file_list, 100);
	}

	/* I wanted to put this at the top of the loop to make it return error before runing other checks 
	 * but I found it diffiult to get my circuit working in the correct order, this simple dead end route 
	 * was easiest to keep it running to receive connections.  */
	else {
		send(new_fd, failure_msg, strlen(failure_msg), 0);
		printf("Usage: Please see readme for usage. (invalid command)d\n");
	}
	printf("Info: Still waiting for incoming connections \n");
}

/********************************************************************
* waiting(int) 
* accepts a file socket descriptor 
* waits for incoming connections 
* calls to client request when new requests are received
* http://beej.us/net2/html/clientserver.html
*********************************************************************/
void waiting(int sockfd){	
	socklen_t addr_size;
	int new_fd;
	struct sockaddr_storage their_addr;
	
	while(1) {
		addr_size = sizeof(their_addr);
		new_fd = accept(sockfd, (struct addrinfo *) &their_addr, &addr_size);
		if(new_fd == -1) {
			continue;
		}
		client_req(new_fd);
		close(new_fd);
	}
}



/********************************************************************
* int get_files(char **)
* count files in dir and put in string
* takes string array 
* http://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
*********************************************************************/
int get_files(char ** files){

	struct dirent * dir;
	DIR * fileDir = opendir(".");
	int i = 0;

	if(fileDir) { 
		while((dir = readdir(fileDir)) != NULL) {
			if (dir -> d_type == DT_REG) {
				strcpy(files[i], dir->d_name);
				i++;
			}
		}
		closedir(fileDir);
	}
	return i;
}

/********************************************************************
* char ** get_file_array(int)
*
* make string aray on the heap for files in dir
* takes integer number (of files) 
*********************************************************************/
char ** get_file_array(int size){
	
	char ** arr = malloc(size*sizeof(char *));
	int i;

	for(i = 0; i< size; i ++) {
		arr[i] = malloc(100*sizeof(char));
		memset(arr[i], 0, sizeof(arr[i]));
	}
	return arr;
}

/********************************************************************
* void remove_file_array(char ** , int)
* basic helper function to clear heap array
* removes the string array on the heap 
* takes string array and how many files it contained
*********************************************************************/
void remove_file_array(char ** arr, int size){
	int i; 
	for(i = 0; i < size; i++){
		free(arr[i]);
	}
	free(arr);
}

/********************************************************************
* int check_exists(char **, int, char *)
* 
* check to see if the file exists in the array 
* takes files array, filename, and num of files
*********************************************************************/
int check_exists(char ** files, int num_files, char * filename){
	int i;
	int exists = 0;
	for(i=0; i<num_files; i++) {
		if(strcmp(files[i], filename) == 0 ) {
			exists = 1;
		}
	}
	return exists;
}

/********************************************************************
* void send_fi(char *, char *, char *)
* 
* send file on socket to set ip + port no 
* takes ip, port and filename  
* https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
*********************************************************************/
void send_fi(char * ip_address, char * portno, char * filename){

	sleep(2);
	
	struct addrinfo * results = make_addr_ip_info(ip_address, portno);
	int data_socket = make_socket(results);
	make_connection(data_socket, results);
	char buffer[1000];
	memset(buffer, 0, sizeof(buffer));
	int fd = open(filename, O_RDONLY);

	while (1) {
		int bytes_read = read(fd, buffer, sizeof(buffer) -1);
		if (bytes_read == 0) {
			break;
		}
		if(bytes_read < 0) {
			fprintf(stderr, "Error: unable to read file\n");
			return;
		}
		void *p = buffer; 
		while(bytes_read > 0) {
			int bytes_written = send(data_socket, p, sizeof(buffer), 0);
			if(bytes_written < 0){
				fprintf(stderr, "Error: unable to write socket\n");
				return;
			}
			bytes_read -= bytes_written;
			p+= bytes_written;
		}
		memset(buffer, 0, sizeof(buffer));
	}
	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, "__done__");
	send(data_socket, buffer, sizeof(buffer),0);
	close(data_socket);
	freeaddrinfo(results);
}
