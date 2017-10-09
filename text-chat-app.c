/**
 * @d25_assignment1
 * @author  Deepika Chaudhary <d25@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 REFERENCES : https://stackoverflow.com/questions/4139405/how-can-i-get-to-know-the-ip-address-for-interfaces-in-c (for external IP address)
 http://www.geeksforgeeks.org/strtok-strtok_r-functions-c-examples/ (for breking string into tokens)

 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/global.h"
#include "../include/logger.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h> // open function
#include <unistd.h> // close function
#include <regex.h>
#include <ifaddrs.h>
#include <ctype.h>

#define BUFFSIZE 1000
#define your_ubit_name "d25"
#define MAXCLIENTS 4
#define MYPORT 4322
#define STDIN 0

//client
struct clientDetailsStore{
			//int list_id;
	char hostname[100];
	char ipstr[INET_ADDRSTRLEN];
	int port_num;
	char *blockedClients[MAXCLIENTS - 1];
};

struct clientDetailsStore connectedLoggedInClients[MAXCLIENTS];
int checkForUpperCase(const char *inputStr){
	int len = strlen(inputStr);
	for(int i=0;i<len;i++){
		if(inputStr[i] >= 'a' && inputStr[i] <= 'z'){
			return 0;
		}
	}
	return 1;
}

int sendall(int s, char *buf, int *len)
{
    
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;
    char *str = buf;
    
    while(total < *len) {
        n = send(s, str+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        //printf("%d",n);
        bytesleft -= n;
    }
    
    *len = total; // return number actually sent here
    return n==-1?-1:0; // return -1 on failure, 0 on success
}
char receivedStr[BUFFSIZE];
int receivebuffer(int s,int len,char *outbuf){
	
	
	char *recvLen_buf = (char*)calloc(len, sizeof(char));
	// char *data_buf;
	
	int bytesleft;
	int n;
	
	int bytes_rcvd = recv(s,recvLen_buf,len,0);
	
	bytesleft = atoi(recvLen_buf);
	
	int total = 0;
	int totalLen = bytesleft;
	
	// memset(recvLen_buf,'\0',strlen(recvLen_buf));
	
	memset(recvLen_buf,'\0',sizeof(recvLen_buf));
	char *data_buf = (char*)calloc((totalLen + 1), sizeof(char));
	
	
    
	while(total < totalLen && bytesleft > 0){
	    n = recv(s, data_buf+total, bytesleft, 0);
		if(n == -1){break;}

		total += n;
		bytesleft -= n;
		
	}
	
	
	data_buf[totalLen] = '\0';
	strcpy(outbuf,data_buf);
	
	int lenBuff = strlen(outbuf);
	
	if(outbuf[lenBuff] == '\n' || outbuf[lenBuff] != '\0'){
		outbuf[lenBuff] = '\0';
		
	}
	
	
	memset(data_buf,'\0',sizeof(data_buf));
 	
    return n==-1?-1:(total-5); 
}

//change this
char *firstWord=NULL, *secondWord=NULL, *remain=NULL;

int breakIntoThreeParts(char *sendBuff){
	// printf("\nBreaking the words\n");
	firstWord = "";
	secondWord = "";
	remain = "";
	
	if(sendBuff == NULL) return 0;
	char input[BUFFSIZE];
	strcpy(input,sendBuff);

	char delimiter[] = " ";
	char *context;
    
	int inputLength = strlen(input);
	
	
	char *inputCopy = (char*) calloc(inputLength + 1, sizeof(char));
	

	strncpy(inputCopy, input, inputLength);

	if(inputCopy == NULL){

	}else{
		firstWord = strtok_r (inputCopy, delimiter, &context);
		
		int contextLen = strlen(context);
		if(contextLen == 0){
		//do nothing
			secondWord = 0;
			remain = 0;
		}else{
			secondWord = strtok_r (NULL, delimiter, &context);
			contextLen = strlen(context);
			if(contextLen == 0){
				//do nothing
				remain = 0;
			}else{
				remain = context;
				
			}
		
		
		}
	}
	if(firstWord != NULL){
		int lenOfWord = strlen(firstWord) -1;
		if(firstWord[lenOfWord] == '\n'){
			firstWord[lenOfWord] = '\0';
		}
	}
	if(secondWord != NULL){
		int lenOfWord = strlen(secondWord) -1;
		if(secondWord[lenOfWord] == '\n'){
			
			secondWord[lenOfWord] = '\0';
		}
	}
	if(remain !=NULL){
		int lenOfWord = strlen(remain) -1;
		if(remain[lenOfWord] == '\n'){
			remain[lenOfWord] = '\0';
		}
	}
	
	
	
	
	
    
	
	//memset(inputCopy,0,strlen(inputCopy));
	inputCopy=NULL;
	
	return 0;
}

void createSendingString(int s,char *stdinBuffer){
	// printf("\nCreating sending string\n");
	char *dest = NULL;
	int len = strlen(stdinBuffer);
	
	dest = malloc(len + 5);
	sprintf(dest,"%5d%s",len,stdinBuffer);
	// printf("\nsending string : %s\n",stdinBuffer);
	int totLen = strlen(dest);
	sendall(s,dest,&totLen);
	
}
void EventMessageRelayed(char *from_client_ip,char *to_client_ip,char* msg){
	char *command_str="RELAYED";
    cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
    cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", from_client_ip, to_client_ip, msg);
    cse4589_print_and_log("[%s:END]\n", command_str);
	
}

void EventMessageReceived(char *from_client_ip,char* msg){
	char *command_str="RECEIVED";
    cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
    cse4589_print_and_log("msg from:%s\n[msg]:%s\n", from_client_ip, msg);
    cse4589_print_and_log("[%s:END]\n", command_str);
}
//client
int breakTheResponse(char *server_response){
	
	char serverCopy[BUFFSIZE];
	strcpy(serverCopy, server_response);
	char *indicator;
	char *  resp= strtok (server_response,"-");
	
	indicator = resp;
    char **	breakresp = NULL;	    
	int n_clients = 0;				    
    
	
	if(indicator != NULL && strcmp(indicator,"N")==0){
			
			char *secondPart;
			char *context;
			secondPart = strtok_r (serverCopy, "-", &context);
			
			
			
			breakIntoThreeParts(context);
			
		    char *from_client_ip = firstWord;
		    char msg[BUFFSIZE+12];
		    strcpy(msg,secondWord);
		    if(remain != NULL){
		    	strcat(msg," ");
		    	strcat(msg,remain);
		    }
		    EventMessageReceived(from_client_ip,msg);
		    
		    
			
			
		
		
		return 0;
	}
	else{
		while (resp != NULL) {
	    	breakresp = realloc (breakresp, sizeof (char*) * ++n_clients);
	    	breakresp[n_clients-1] = resp;
	    	resp = strtok (NULL, "-");
		}
		for(int i=0;i<n_clients;i++){
			
			char *  client= strtok (breakresp[i],",");
			char **breakClient = NULL;
		    int n_comma=0;
		    while(client != NULL){

		    	breakClient = realloc (breakClient, sizeof (char*) * ++n_comma);
		    	breakClient[n_comma-1]=client;
		    	client = strtok(NULL,",");
		    }
		    
			strcpy(connectedLoggedInClients[i].hostname,breakClient[1]);
			strcpy(connectedLoggedInClients[i].ipstr,breakClient[2]);
			connectedLoggedInClients[i].port_num = atoi(breakClient[3]);
			
			breakClient=NULL;
		}
		
		breakresp = NULL;
		
		return n_clients;
	}
	
}



int validatePORT(char *portAddr){
	while (*portAddr) {
        if (isdigit(*portAddr++) == 0) return 0;
    }
    return 1;
	
}
void sortAndDisplayList(int num_clients,int isDisplay) {
	//sort
	for(int k=0;k<num_clients-1;k++){
		for(int j=k+1;j<num_clients;j++){
			if(connectedLoggedInClients[k].port_num > connectedLoggedInClients[j].port_num){
				struct clientDetailsStore temp = connectedLoggedInClients[k];
				connectedLoggedInClients[k] = connectedLoggedInClients[j];
				connectedLoggedInClients[j] = temp;
			}
		}
	}
	//display
	if(isDisplay){
		for(int i=0;i<num_clients;i++){
	    	int list_id = i+1;
	    	char *hostname = connectedLoggedInClients[i].hostname;
	    	char *ipstr = connectedLoggedInClients[i].ipstr;
	    	int port_num = connectedLoggedInClients[i].port_num;
			cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id, hostname, connectedLoggedInClients[i].ipstr, port_num);

	    }
	}
	
}
//server
struct clientDetailsStorage{
			char hostname[100];
			char ipstr[INET_ADDRSTRLEN];
			int port_num;
			int onSocket;
			char loggedInState[15];
			char *blockedClients[MAXCLIENTS - 1];
			int num_msg_sent;
			int num_msg_rcv;
		};
		struct msgStore{
			char clientName[INET_ADDRSTRLEN];
			char messages[BUFFSIZE];
		};
		struct BuffDatastorage{
			char ipname[INET_ADDRSTRLEN];
			struct msgStore msg[100];
			int hasMsg;

		};
struct BuffDatastorage storedBuff[MAXCLIENTS];
		int numOfListeners = 0;
		struct clientDetailsStorage clientNo[MAXCLIENTS];


void sendToDest(int sockNo,char *msg,char *destinationIP,char* sendType){
	char *sendBuffer=NULL;
	char *fromClientNumWithip = NULL;
	int fromClientNo;
	for(int i =0 ;i<MAXCLIENTS;i++){
		if(clientNo[i].onSocket == sockNo){

			fromClientNumWithip = clientNo[i].ipstr;
			fromClientNo = i;
			break;
		}
	}
	

	if(fromClientNumWithip){
		int ipLen = strlen(fromClientNumWithip);
		

		sendBuffer = (char*)calloc((strlen(msg)+ipLen+1) ,sizeof(char));
		
		clientNo[fromClientNo].num_msg_sent += 1;
		
		
	}

	int foundClient = 0;
	int sockNumIdentified= 0;
	int clientNum;
	int foundInBlocked = 0;
	//identify where msg should be sent
	for(int i=0;i<MAXCLIENTS;i++){
		

		if(strcmp(clientNo[i].ipstr,destinationIP)==0){
			
			//blockedClient can be array
			
			for(int s=0;s<MAXCLIENTS-1;s++){
				if(clientNo[i].blockedClients[s]){
					if(strcmp(clientNo[i].blockedClients[s],fromClientNumWithip) == 0){foundInBlocked=1;}
				}
			}
			if(foundInBlocked){
				
				foundClient = 1;
				
			}else{
				
				strcpy(sendBuffer,"\0");
				strcat(sendBuffer,fromClientNumWithip);
				strcat(sendBuffer," ");
				strcat(sendBuffer,remain);
				
				sockNumIdentified = clientNo[i].onSocket;
				clientNum = i;
				
				foundClient = 1;
				
			}
			break;
			
		}
	}
	int msglength = strlen(sendBuffer);
	
	if(!foundClient){
	// printf("\nCOULDN'T SEND : This client doesn't exists anymore\n");
	}

	if(strcmp(clientNo[clientNum].loggedInState,"logged-in") == 0 && !foundInBlocked ){
		
		if(strcmp(sendType,"NORMAL") == 0){EventMessageRelayed(fromClientNumWithip,destinationIP,msg);}
		clientNo[clientNum].num_msg_rcv += 1;
		char normalSend[BUFFSIZE];
		strcpy(normalSend,"N-");
		strcat(normalSend,sendBuffer);
		if(sockNumIdentified){
			createSendingString(sockNumIdentified,normalSend);
			
		}
	}
	if(strcmp(clientNo[clientNum].loggedInState,"logged-out") == 0 && !foundInBlocked){
		
		
		clientNo[clientNum].num_msg_rcv += 1;
		
		int notAvailableIn = 0;
		for(int k=0;k<MAXCLIENTS;k++){
			if(strcmp(storedBuff[k].ipname,destinationIP)==0){
				storedBuff[k].hasMsg = 1;
				for(int i=0 ;i<100;i++){
					if(strcmp(storedBuff[k].msg[i].messages,"") == 0){
						// strcpy(storedBuff[k].msg[i].messages,"N-");
						strcpy(storedBuff[k].msg[i].clientName,fromClientNumWithip);
						strcpy(storedBuff[k].msg[i].messages,msg);
						break;
					}
				}
				
				
				break;
			}
			
		}
	}
	if(foundInBlocked){ foundInBlocked = 0;}
	memset(sendBuffer,'\0',sizeof(sendBuffer));
}
/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	struct linger linger;
	linger.l_onoff = 1;
	linger.l_linger = 0;
	struct sockaddr_in sa;
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
if(*argv[1] == 's'){
		char* list[] = {"AUTHOR","IP","PORT","LIST","STATISTICS","BLOCKED","LOGIN","REFRESH","SEND","BROADCAST","BLOCK","UNBLOCK","LOGOUT","EXIT"};
		
		

		char ipstr[INET_ADDRSTRLEN];
		int server_socket;
		int bytes_sent,bytes_rcvd;
		char server_buffer[BUFFSIZE];
		char recv_request[BUFFSIZE];
		struct sockaddr_in server_address;
		
		int yes=1;
		struct sockaddr_in their_addr; // capture the details of incoming connection in this
		socklen_t their_addr_len;
		int fdmax; // maximum filedescriptor
		int childSocketDescriptors[MAXCLIENTS];
		
		memset(&server_address, 0, sizeof(struct sockaddr_in));
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(atoi(argv[2])); //MYPORT


		//for getting some address
		
		server_address.sin_addr.s_addr = INADDR_ANY;//inet_addr("192.168.0.7"); //
		
		memset(&(server_address.sin_zero),'\0',8);
		//get EXTERNAL ipaddress
		struct ifaddrs *ifap, *ifa;
	    struct sockaddr_in *sa;
	    char *addr;

	    getifaddrs (&ifap);
	    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
	        if (ifa->ifa_addr->sa_family==AF_INET) {
	            sa = (struct sockaddr_in *)ifa->ifa_addr;
	            if(strcmp(ifa->ifa_name,"eth0") == 0){
	            	addr = inet_ntoa(sa->sin_addr);
	            }
	            
	        }
	    }
	    freeifaddrs(ifap);
		//open a socket on server
		server_socket = socket(AF_INET,SOCK_STREAM,0);	//socket descriptor which is open all the times.
		if (setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
		    perror("setsockopt");
		    
		}
		if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, (const char*)&yes, sizeof(yes)) < 0) {
        	perror("setsockopt(SO_REUSEPORT) failed");
		}

		if (setsockopt(server_socket, SOL_SOCKET, SO_LINGER, &linger, sizeof(struct linger)) < 0) {
        	perror("setsockopt(SO_LINGER) failed");
		}
		//bind the socket to specified IP and PORT of server-its own
		bind(server_socket,(struct sockaddr*) &server_address,sizeof(server_address));
		
		
		listen(server_socket, MAXCLIENTS);
		//for multiple connections on server
		fd_set master;    // master file descriptor list
        fd_set read_fds;  // temp file descriptor list for select()
		fdmax = server_socket;
		for(int i=0;i<MAXCLIENTS;i++){
			clientNo[i].port_num = 0;
			clientNo[i].num_msg_rcv = 0;
			clientNo[i].num_msg_sent = 0;
			clientNo[i].onSocket = -1;
			storedBuff[i].hasMsg = 0;
			for(int j=0;j<100;j++){
				strcpy(storedBuff[i].msg[j].messages,"");
			}
		}
		FD_ZERO(&master);    // clear the master and temp sets
		FD_ZERO(&read_fds);
		FD_SET(server_socket,&master);
		FD_SET(STDIN,&master);
		while(1){
			read_fds = master;
			
			//for multiple connections on listener
			if (select(fdmax+1, &read_fds, NULL, NULL, NULL) < 0) {
				perror("error");
		      
		    }

		    

		    //for new connections
		    

			int maxSockets = fdmax + 1;   //for client connections
		    for(int j=0 ; j<maxSockets;j++){
		    	if(FD_ISSET(STDIN,&read_fds)){
			    	char stdinBuffer[BUFFSIZE];
			    	char command_str[BUFFSIZE];
			    	int lenOfBuff;
			    	int isUpperCase;
			    	
			    	//get data from standard input
			    	fgets(stdinBuffer, BUFFSIZE, stdin);
			    	breakIntoThreeParts(stdinBuffer);
			    	strcpy(command_str,firstWord);
			    	
			    	lenOfBuff = strlen(command_str) - 1;
			    	if(command_str[lenOfBuff] == '\n'){
		    			command_str[lenOfBuff] = '\0';
		    		}
		    		isUpperCase = checkForUpperCase(command_str);
		    		if(isUpperCase && lenOfBuff){
		    			
		    			if(strcmp(list[0],command_str)==0){ //AUTHOR
		    				cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
							cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", your_ubit_name);
				    	}else if(strcmp(list[1],command_str)==0){ //IP
				    		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
				    		cse4589_print_and_log("IP:%s\n", addr);
				    	}else if(strcmp(list[2],command_str)==0){ //PORT
				    		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
				    		cse4589_print_and_log("PORT:%s\n", argv[2]);
				    	}else if(strcmp("RESET",command_str)==0){
				    		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
				    		cse4589_print_and_log("RESETTING...");
				    		//free(receivedStr);
				    		
				    		//receivedStr = NULL;
				    		numOfListeners = 0;
				    		for(int j=1 ; j<fdmax+1;j++){
				    			if(j!= server_socket){
				    				close(j);
				    				FD_CLR(j,&master);
				    			}
				    		}
				    		fdmax = server_socket;
				    		FD_ZERO(&master);    // clear the master and temp sets
							FD_ZERO(&read_fds);
							FD_SET(server_socket,&master);
							FD_SET(STDIN,&master);
							break;
				    	}else if(strcmp(list[3],command_str)==0){ //get list
				    		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
				    		
				    		if(numOfListeners){
				    			for(int k=0;k<MAXCLIENTS;k++){
			    					for(int j=k+1;j<MAXCLIENTS;j++){
			    						if(clientNo[k].port_num > clientNo[j].port_num){
			    							struct clientDetailsStorage temp = clientNo[k];
			    							clientNo[k] = clientNo[j];
			    							clientNo[j] = temp;
			    						}
			    					}
			    				}
			    				int count = 1;
			    				for(int i=0; i<MAXCLIENTS;i++){
			    					int list_id;
			    					char hostname[20];
			    					strcpy(hostname ,clientNo[i].hostname);
			    					int port_num = clientNo[i].port_num;
			    					// if(clientNo[i].port_num && strcmp(clientNo[i].loggedInState,"logged-in") == 0){
			    					if(clientNo[i].port_num && strcmp(clientNo[i].loggedInState,"logged-in") == 0){
			    						list_id = count++;
			    						cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id, hostname, clientNo[i].ipstr, port_num);
			    					}
								}
				    		}
		    				
				    		
				    	}else if(strcmp(list[4],command_str)==0){//STATISTICS
				    		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
			    			if(numOfListeners){
			    				for(int i=0; i<MAXCLIENTS;i++){
			    					if(clientNo[i].onSocket && clientNo[i].port_num){
			    						int list_id = i+1;
				    					char hostname[20];
				    					strcpy(hostname ,clientNo[i].hostname);
				    					int num_msg_sent = clientNo[i].num_msg_sent;
				    					int num_msg_rcv = clientNo[i].num_msg_rcv;
				    					char status[10];
				    					strcpy(status,clientNo[i].loggedInState);
				    					cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", list_id, hostname, num_msg_sent, num_msg_rcv, status);
			    					}
			    					
			    				}
			    				
			    			}

			    		}else if(strcmp(list[5],command_str)==0){//BLOCKED
			    			
			    			
		    				
		    				
		    				if(secondWord == NULL || (secondWord != NULL && inet_pton(AF_INET,secondWord,&(sa->sin_addr)) != 1)){
		    					cse4589_print_and_log("[%s:ERROR]\n", command_str);

		    				}else{
		    					int clientNum=0;
		    					int len;
		    					for(int i=0;i<MAXCLIENTS;i++){
		    						len = strlen(clientNo[i].ipstr);
		    						
		    						if(strcmp(clientNo[i].ipstr,secondWord) == 0){
		    							clientNum = i+1;
		    						}
		    					}
		    					
		    					if(!clientNum){
		    						cse4589_print_and_log("[%s:ERROR]\n", command_str);
		    						cse4589_print_and_log("[%s:END]\n", command_str);
		    						break;
		    					}else{
		    						struct clientDetailsStorage blockedList[MAXCLIENTS-1];
				    				for(int i=0;i<MAXCLIENTS-1;i++){
				    					blockedList[i].port_num = 0;
				    				}
			    					cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
			    					int num = clientNum-1;
			    					for(int s=0;s<MAXCLIENTS-1;s++){
		    							if(clientNo[num].blockedClients[s] != NULL){
		    								
		    								for(int k=0;k<MAXCLIENTS;k++){
		    									if(strcmp(clientNo[num].blockedClients[s],clientNo[k].ipstr)==0){
		    										for(int d=0;d<MAXCLIENTS-1;d++){
		    											if(blockedList[d].port_num == 0){
			    											strcpy(blockedList[d].ipstr,clientNo[k].ipstr);
					    				
										    				blockedList[d].port_num = clientNo[k].port_num;
										    				
										    				strcpy(blockedList[d].hostname,clientNo[k].hostname);
										    				
										    				
										    				strcpy(blockedList[d].loggedInState,clientNo[k].loggedInState);

										    				break;
			    										}

		    										}
		    										

		    									}
		    								}
		    							}
		    						}
		    						//sort
		    						for(int k=0;k<MAXCLIENTS-1;k++){
				    					for(int j=k+1;j<MAXCLIENTS-1;j++){
				    						if(blockedList[k].port_num > blockedList[j].port_num){
				    							struct clientDetailsStorage temp = blockedList[k];
				    							blockedList[k] = blockedList[j];
				    							blockedList[j] = temp;
				    						}
				    					}
				    				}
				    				//display
				    				int count =1;
				    				for(int k=0;k<MAXCLIENTS-1;k++){
				    					if(blockedList[k].port_num !=0 )
		    								cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", (count++), blockedList[k].hostname, blockedList[k].ipstr,blockedList[k].port_num);

				    				}
		    					}
		    					
		    				}
			    			
			    			
			    		}else if(strcmp(command_str,"EXIT")==0){
					    	//not necessary
					    	cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
					    	cse4589_print_and_log("[%s:END]\n", command_str);
					    	numOfListeners = 0;
					    	for(int i=0;i<fdmax+1;i++){
					    		
					    			close(i);
									FD_CLR(i,&master);
									FD_CLR(i,&read_fds);
					    			
								
									
								
							}
							FD_ZERO(&master);
							FD_ZERO(&read_fds);
							exit(0);
					    }else{
			    			cse4589_print_and_log("[%s:ERROR]\n", command_str);
				    	}

		    		}else{
		    			cse4589_print_and_log("[%s:ERROR]\n", command_str);
		    		}
		    		cse4589_print_and_log("[%s:END]\n", command_str);
		    		// fflush(stdout);
		    		break;
			    }else if(FD_ISSET(server_socket,&read_fds)){ // for server sockets - new connection
		    	//handle new connections
		    		// printf("\n new connection\n");
		    		if(numOfListeners <= MAXCLIENTS){
		    			int new_sock;
						
						
					    new_sock = accept(server_socket, (struct sockaddr *)&their_addr, &their_addr_len); //new sock descriptor for accepted connection to send and receive data.
					    if(new_sock == -1){
					    	printf("error in accept");
					    }else{
					    	//assign socket descriptor to new client
					    	
					    	
					    	char getIpAddr[INET_ADDRSTRLEN];
					    	int port_num;
					    	struct hostent *he;
							struct in_addr ipv4addr;
							
							

					    	getpeername(new_sock,(struct sockaddr*) &their_addr,&their_addr_len);
					    	inet_ntop(AF_INET, &their_addr.sin_addr, getIpAddr, INET_ADDRSTRLEN);
					    	port_num = ntohs(their_addr.sin_port);
					    	inet_pton(AF_INET, getIpAddr, &ipv4addr);
							he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
							
					    	int oldLoggedOutClient = 0;
					    	//old connection send buffered messages to client
					    	for(int i=0;i<MAXCLIENTS;i++){
					    		
					    		if(strcmp(clientNo[i].ipstr,getIpAddr) == 0 &&  clientNo[i].port_num==port_num && strcmp(clientNo[i].loggedInState,"logged-out") == 0){
					    			
					    			strcpy(clientNo[i].loggedInState,"logged-in");
					    			clientNo[i].onSocket = new_sock;
					    			oldLoggedOutClient = 1;
					    			for(int x=0;x<MAXCLIENTS;x++){
					    				if(storedBuff[x].hasMsg && strcmp(storedBuff[i].ipname,getIpAddr) == 0){
						    				
						    				for(int y=0;y<100;y++){
						    					if(strcmp(storedBuff[x].msg[y].messages,"") != 0){
						    						EventMessageRelayed(storedBuff[x].msg[y].clientName,clientNo[i].ipstr,storedBuff[x].msg[y].messages);
						    						char displayBuff[BUFFSIZE];
						    						strcpy(displayBuff,"N-");
						    						strcat(displayBuff,storedBuff[x].msg[y].clientName);
						    						strcat(displayBuff," ");
						    						strcat(displayBuff,storedBuff[x].msg[y].messages);
						    						createSendingString(new_sock,displayBuff);
						    						strcpy(storedBuff[x].msg[y].messages,"");
						    					}
						    				}
						    				
						    				storedBuff[i].hasMsg = 0;
						    				
						    				break;
						    			}
					    			}

					    			
					    			
					    			
					    		}
					    	}
					    	
					    	//new connection add to the client list
					    	if(!oldLoggedOutClient){
					    		// printf("\nconnection came\n");
					    		numOfListeners++;
					    		for(int i=0;i<MAXCLIENTS;i++){
					    			if(clientNo[i].port_num == 0){
					    				strcpy(clientNo[i].ipstr,getIpAddr);
					    				
					    				clientNo[i].port_num = port_num;
					    				clientNo[i].onSocket = new_sock;
					    				strcpy(clientNo[i].hostname,he->h_name);
					    				
					    				strcpy(storedBuff[i].ipname,getIpAddr);
					    				strcpy(clientNo[i].loggedInState,"logged-in");
					    				break;
					    			}
					    		}
					    		
				    		}else{
				    			oldLoggedOutClient = 0;
				    		}

				    			char tempBuf[BUFFSIZE];
				    			//prepare the list to be sent
				    			
				    			memset(server_buffer,'\0',sizeof(server_buffer));
				    			
				    			for(int i=0;i<MAXCLIENTS;i++){
				    				if(strcmp(clientNo[i].loggedInState,"logged-in") == 0){
				    					sprintf(tempBuf, "%d,%s,%s,%d-",(i+1),clientNo[i].hostname,clientNo[i].ipstr,clientNo[i].port_num);
				    					strcat(server_buffer,tempBuf);
				    				}
				    				
				    			}
				    			
				    			
				    			createSendingString(new_sock,server_buffer);
				    			memset(tempBuf,'\0',sizeof(tempBuf));
				    		//set the socket for listening
				    		FD_SET(new_sock,&master);

			    			if(fdmax < new_sock){
			    				fdmax = new_sock;
			    			}

			    		}
			    		
				    }else{
				    	// printf("MAX client Connections Reached...NO more connections allowed\n");
				    }
				    break;
				}else if(FD_ISSET(j,&read_fds) && j != server_socket && j != STDIN){
			    	//handle data from client:will write it later
			    	// printf("HANDLE CLIENT with socket %d\n",j);
			    	bytes_rcvd = receivebuffer(j,5,receivedStr); //first receive length
			    	
			    	if(bytes_rcvd <= 0){
			    		
					    	numOfListeners--;
					    	for(int i=0;i<MAXCLIENTS;i++){
								
								if(clientNo[i].onSocket == j){
									clientNo[i].onSocket = -1;
									memset(&clientNo[i],0,sizeof(struct clientDetailsStorage));
									
									if(FD_ISSET(j,&master)){
										close(j);

										FD_CLR(j,&master);
										FD_CLR(j,&read_fds);
									}
									
									break;
								}
							}
			    		
			    	}else{
			    		//data received from clients
			    		
			    		
			    		breakIntoThreeParts(receivedStr);
			    		
			    		
					    if(strcmp(firstWord,"LOGIN")==0){
					    	for(int i=0;i<MAXCLIENTS;i++){
								
								if(clientNo[i].onSocket == j){
									if(strcmp(clientNo[i].loggedInState,"logged-out")){
										strcpy(clientNo[i].loggedInState,"logged-in");
										
										break;
									}
									
									
								}
							}

					    }else if(strcmp(firstWord,"REFRESH")==0){
					    	char tempBuf[BUFFSIZE];
					    	memset(server_buffer,'\0',sizeof(server_buffer));
				    			//prepare the list to be sent
					    	
				    			for(int i=0;i<MAXCLIENTS;i++){
				    				if(strcmp(clientNo[i].loggedInState,"logged-in") == 0){
				    					sprintf(tempBuf, "%d,%s,%s,%d-", (i+1),clientNo[i].hostname,clientNo[i].ipstr,clientNo[i].port_num);
				    					strcat(server_buffer,tempBuf);
				    				}
				    				
				    			}
				    			
				    			
					    	createSendingString(j,server_buffer);

					    }else if(strcmp(firstWord,"SEND")==0){ /// check this on server
					    	

					    	sendToDest(j,remain,secondWord,"NORMAL");
					    	
					    
					    }else if(strcmp(firstWord,"BROADCAST")==0){
					    	// printf("Broadcasting....");
					    	
					    	
					  		for(int i=0;i<MAXCLIENTS;i++){
					  			if(clientNo[i].onSocket != j && clientNo[i].port_num && (clientNo[i].onSocket != -1 || strcmp(clientNo[i].loggedInState,"logged-out") == 0)){
					  				sendToDest(j,remain,clientNo[i].ipstr,"BROADCAST");
					  			}else if(clientNo[i].onSocket == j){
					    			EventMessageRelayed(clientNo[i].ipstr,"255.255.255.255",remain);

					  			}
					  			
					  		}
					    }else if(strcmp(firstWord,"BLOCK")==0){
					    	//validation on secondword.
					    	int BlockingValidClient = 0;
					    	for(int i=0;i<MAXCLIENTS;i++){
					    		if(strcmp(clientNo[i].ipstr,secondWord)==0){
					    			
					    			BlockingValidClient = 1;
					    		}
					    	}
					    	if(BlockingValidClient){
					    		for(int i=0;i<MAXCLIENTS;i++){
						    		if(clientNo[i].onSocket == j){
						    			
						    			
						    			for(int k=0;k<MAXCLIENTS-1;k++){
						    				if(clientNo[i].blockedClients[k] == NULL){
						    					clientNo[i].blockedClients[k] = malloc(INET_ADDRSTRLEN);
						    					strcpy(clientNo[i].blockedClients[k],secondWord);
						    					break;
						    				}else if(strcmp(clientNo[i].blockedClients[k],secondWord)==0){
												// printf("Already blocked");
						    					break;
						    					
						    				}
						    			}
						    			
						    		}
						    	}
					    	}else{
					    		// printf("\nPlease update your list client is not present\n");
					    	}
						    	

					    }else if(strcmp(firstWord,"UNBLOCK")==0){ //unblock
					    	for(int i=0;i<MAXCLIENTS;i++){
					    		if(clientNo[i].onSocket == j){
					    			for(int k=0;k<MAXCLIENTS-1;k++){
						    				if(clientNo[i].blockedClients[k] != NULL && strcmp(clientNo[i].blockedClients[k],secondWord)==0){
						    					memset(clientNo[i].blockedClients[k],'\0',sizeof(char) * 12);
						    					clientNo[i].blockedClients[k] = NULL;
						    					break;
						    				}else{
												// printf("\nNOT FOUND\n");
						    					
						    					
						    				}
						    			}
					    			
					    		}
					    	}

					    }else if(strcmp(firstWord,"LOGOUT")==0){
					    	// printf("\nLogging out...\n");
					    	for(int i=0;i<MAXCLIENTS;i++){
								
								if(clientNo[i].onSocket == j){
									
									
									strcpy(clientNo[i].loggedInState,"logged-out");
									
									clientNo[i].onSocket = -1;
									close(j);
									FD_CLR(j,&master);
									break;
								}
							}
					    }
					    memset(receivedStr,'\0',sizeof(receivedStr));
			    	}
			    	break;
			    }
		    }
		    
			
		}

	}else if(*argv[1] == 'c'){
		char* list[] = {"AUTHOR","IP","PORT","LIST","LOGIN","REFRESH","SEND","BROADCAST","BLOCK","UNBLOCK","LOGOUT","EXIT"};

		char ipstr[INET_ADDRSTRLEN];
		int bytes_read; //no of bytes read into buffer
		
		struct sockaddr_in dest_server_address;
		struct sockaddr_in client_address;
		
		char sendBuff[BUFFSIZE];
		char server_response[BUFFSIZE]; //response will be received in this.
		char request[BUFFSIZE];
		int yes=1;
		int logged_in = 0;
		int newConnectionPassList = 0;
		int num_clients = 0;
		int acceptBuffer=0;
		int receiveData = 0;
		int listFlag = 1;
		memset(&client_address, 0, sizeof(struct sockaddr_in));
		client_address.sin_family = AF_INET;
		client_address.sin_port = htons(atoi(argv[2])); //MYPORT

		
		
		//for getting some address
		//inet_ntop(AF_INET, &(client_address.sin_addr), ipstr, INET_ADDRSTRLEN);
		
		client_address.sin_addr.s_addr = INADDR_ANY;
		struct ifaddrs *ifap, *ifa;
		    struct sockaddr_in *sa;
		    char *addr;

		    getifaddrs (&ifap);
		    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		        if (ifa->ifa_addr->sa_family==AF_INET) {
		            sa = (struct sockaddr_in *) ifa->ifa_addr;
		            if(strcmp(ifa->ifa_name,"eth0") == 0){
		            	addr = inet_ntoa(sa->sin_addr);
		            }
		        }
		    }
	    	freeifaddrs(ifap);
		memset(&(client_address.sin_zero),'\0',8);
		memset(&(dest_server_address.sin_zero),'\0',8);
		//open a socket for server which is binded to client port but connecting to server port
		int client_socket;
		

		
		fd_set master;    // master file descriptor list
        fd_set read_fds;
        FD_ZERO(&read_fds); 
        FD_ZERO(&master); 
		int sockmax=0;
		
		//FD_SET(client_socket,&master);
		FD_SET(STDIN,&master);
		
		
		int val = sockmax;
		while(1){
			read_fds = master;
			val = sockmax;
			
			if(select(sockmax+1, &read_fds, NULL, NULL, NULL) < 0) {
				perror("error");
		      
		    }
			
			//Destination port struct
			
			dest_server_address.sin_family = AF_INET;
			
			for(int i=0;i<=val;i++){
				
				if(FD_ISSET(STDIN,&read_fds)){
					char stdinBuffer[BUFFSIZE];
					
			    	char command_str[BUFFSIZE];
			    	int lenOfBuff;
			    	int isUpperCase;
			    	
			    	
			    	//get data from standard input
			    	fgets(stdinBuffer, BUFFSIZE, stdin);
			    	
			    	lenOfBuff = strlen(stdinBuffer) - 1;
			    	
			    	if(stdinBuffer[lenOfBuff] == '\n'){
		    			stdinBuffer[lenOfBuff] = '\0';
		    		}
		    		if(!lenOfBuff){
		    			cse4589_print_and_log("[%s:ERROR]\n", stdinBuffer);
		    			cse4589_print_and_log("[%s:END]\n", stdinBuffer);
		    		}else{
		    			breakIntoThreeParts(stdinBuffer);

			    		strcpy(command_str,firstWord);
			    		
			    		isUpperCase = checkForUpperCase(command_str);
			    		
			    		if(isUpperCase && lenOfBuff){
			    			
			    			if(strcmp(list[0],command_str)==0){ //AUTHOR
			    				cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
								cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", your_ubit_name);
					    	}else if(strcmp(list[1],command_str)==0){ //IP
					    		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
					    		cse4589_print_and_log("IP:%s\n", addr);
					    	}else if(strcmp(list[2],command_str)==0){ //PORT
					    		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
					    		cse4589_print_and_log("PORT:%s\n", argv[2]);
					    	}else if(strcmp(list[3],command_str)==0 && listFlag){ //list
					    		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
					    		//displays the list
					    		sortAndDisplayList(num_clients,1);
					    	}else if(strcmp(list[4],command_str)==0 && !logged_in){ //login
					    		
					    		if(secondWord != NULL && remain!=NULL && validatePORT(remain) == 1 && inet_pton(AF_INET, secondWord, &(dest_server_address.sin_addr)) != 0){
					    			
					    			listFlag = 1;
						    		client_socket = socket(AF_INET,SOCK_STREAM,0);

									if (setsockopt(client_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
									    perror("setsockopt");
									    exit(1);
									}
									if (setsockopt(client_socket, SOL_SOCKET, SO_REUSEPORT, (const char*)&yes, sizeof(yes)) < 0) {
							        	perror("setsockopt(SO_REUSEPORT) failed");
									}
									if (setsockopt(client_socket, SOL_SOCKET, SO_LINGER, &linger, sizeof(struct linger)) < 0) {
							        	perror("setsockopt(SO_LINGER) failed");
									}
									bind(client_socket,(struct sockaddr*) &client_address,sizeof(client_address));
									
									

						    		dest_server_address.sin_port = htons(atoi(remain)); //DestinationPORT-specified from login
									//strcpy(dest_server_address.sin_addr.s_addr,inet_addr("128.205.36.46"));//Destination address-specified from login inet_addr("128.205.36.45")
									// printf("\nserver port is %d\n",dest_server_address.sin_port);
									inet_pton(AF_INET, secondWord, &(dest_server_address.sin_addr));
									
									//client is connecting to server -dest ip and port
									// printf("\nConnecting.....\n");
									int connection_status = connect(client_socket, (struct sockaddr*) &dest_server_address, sizeof(dest_server_address));
									//printf("\nMY IP %s and MY PORT %d\n",inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));
									//check for error with the connection
								    if(connection_status == -1){
								    	
								      printf("\nConnection error\n");
								      cse4589_print_and_log("[%s:ERROR]\n", command_str);
								      cse4589_print_and_log("[%s:END]\n", command_str);
								      break;
								      
								    }else{
								    	printf("\nserver-client now connected\n");

								    }
								    //set the socket descriptor for listening
								    
								    logged_in = 1;
						    		newConnectionPassList = 1;
						    		FD_SET(client_socket,&master);
						    		FD_SET(client_socket,&read_fds);
								    
								    	
						    		FD_SET(STDIN,&master);
									if(sockmax < client_socket){
					    				sockmax = client_socket;
					    			}
						    		if(acceptBuffer){
						    			acceptBuffer = 0;
						    			int bytes_rcvd = receivebuffer(client_socket,5,receivedStr); //first receive length
							    		//printf("\nprinting received %s\n",receivedStr);
							    		num_clients = breakTheResponse(receivedStr);
						    		}
						    		if(connection_status != -1)
						    			cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
					    		}else{
					    			cse4589_print_and_log("[%s:ERROR]\n", command_str);
					    		}
					    		
					    		// free(res);
					    		

					    	}else if(strcmp(list[5],command_str)==0 && logged_in){ //REFRESH

					    		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
					    		char *refreshBuf;
					    		strcpy(request,command_str);
					    		createSendingString(client_socket,stdinBuffer);
							    newConnectionPassList=1;
							    
							    sortAndDisplayList(num_clients,0);
							    
							    //printf("%d",num_clients);
					    	}else if(strcmp(list[6],command_str)==0 && logged_in){ //send
					    		//----------------------------//
								//----------------------------// if present then execute code below
								// if(validateIPaddress(secondWord))
								if(secondWord ==NULL || remain == NULL || (secondWord!= NULL && inet_pton(AF_INET,secondWord,&(sa->sin_addr)) != 1)){
									cse4589_print_and_log("[%s:ERROR]\n", command_str);
									cse4589_print_and_log("[%s:END]\n", command_str);
									break;
								}
								
								if(secondWord!= NULL && remain!= NULL){
									if(strcmp(secondWord,addr)==0){
										cse4589_print_and_log("[%s:ERROR]\n", command_str);
										
									}else{
										int foundClient = 0;
										for(int i=0;i<num_clients;i++){
											if(strcmp(secondWord,connectedLoggedInClients[i].ipstr)==0){
												//send the request telling server about send command,ip,buffer length
												
												createSendingString(client_socket,stdinBuffer);
												foundClient = 1;
											}
										}

										if(!foundClient){
											
											cse4589_print_and_log("[%s:ERROR]\n", command_str);
										}else{
											cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
											foundClient = 0;
										}
									}
								}else{
									cse4589_print_and_log("[%s:ERROR]\n", command_str);
								}

								
								
								
								
								
							}else if(strcmp(list[7],command_str)==0 && logged_in){ //broadcast
								
								
								char modifiedStr[BUFFSIZE];
								if(secondWord == NULL){
									cse4589_print_and_log("[%s:ERROR]\n", command_str);
									cse4589_print_and_log("[%s:END]\n", command_str);
									break;
								}else{
									cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
								}
								
								
								strcpy(modifiedStr,firstWord);
								
								if(secondWord != NULL){
									strcat(modifiedStr," padding ");
									strcat(modifiedStr,secondWord);
								}
								

								if(remain != NULL){
									strcat(modifiedStr," ");
									strcat(modifiedStr,remain);
								}
								
					    		createSendingString(client_socket,modifiedStr);
					    	}else if(strcmp(list[8],command_str)==0 && logged_in){ //block client with specified ip
					    		int found = 0;
					    		if(secondWord ==NULL || (secondWord!=NULL && (strcmp(secondWord,addr)==0 )) || (secondWord != NULL && inet_pton(AF_INET,secondWord,&(sa->sin_addr)) != 1)){
					    			cse4589_print_and_log("[%s:ERROR]\n", command_str);
					    		}else{
					    			for(int i=0;i<num_clients;i++){
										if(strcmp(addr,connectedLoggedInClients[i].ipstr)==0){
											//send the request telling server about blocking the client
											int alreadyExist = 0;
											for(int s = 0; s<MAXCLIENTS-1;s++){
												if(connectedLoggedInClients[i].blockedClients[s] != NULL && strcmp(connectedLoggedInClients[i].blockedClients[s],secondWord) == 0){
							    					cse4589_print_and_log("[%s:ERROR]\n", command_str);
							    					alreadyExist=1;
							    					found = 1;
							    				}
											}
											if(!alreadyExist){
												for(int s = 0; s<MAXCLIENTS-1;s++){
													if(connectedLoggedInClients[i].blockedClients[s] == NULL){
								    					int len = strlen(secondWord);
								    					connectedLoggedInClients[i].blockedClients[s] = malloc(len);
								    					strncpy(connectedLoggedInClients[i].blockedClients[s],secondWord,len);
								    					cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
								    					break;
								    				}
												}
												createSendingString(client_socket,stdinBuffer);
												found = 1;
											}
												
												
												
												
										}
									}
									if(!found){
										cse4589_print_and_log("[%s:ERROR]\n", command_str);
									}else
										found = 0;
					    		}
					    		
								
					    		
					    	}else if(strcmp(list[9],command_str)==0 && logged_in){ //unblock client with specified ip
					    		int found = 0;
					    		if(secondWord == NULL || (secondWord != NULL && inet_pton(AF_INET,secondWord,&(sa->sin_addr)) != 1)){
					    			cse4589_print_and_log("[%s:ERROR]\n", command_str);
					    		}else{
					    			int foundBlocked = 0;
						    		for(int i=0;i<num_clients;i++){
										if(strcmp(addr,connectedLoggedInClients[i].ipstr)==0){
											
						    				createSendingString(client_socket,stdinBuffer);
						    				found = 1;
						    				for(int s = 0; s<MAXCLIENTS-1;s++){
						    					
						    					if(connectedLoggedInClients[i].blockedClients[s] !=NULL && strcmp(connectedLoggedInClients[i].blockedClients[s],secondWord) == 0){
						    						//strcpy(connectedLoggedInClients[i].blockedClients[s],'\0');
						    						connectedLoggedInClients[i].blockedClients[s] = NULL;
						    						foundBlocked =1;
						    					}
						    				}
										}
									}
									if(!found || !foundBlocked){
										cse4589_print_and_log("[%s:ERROR]\n", command_str);
									}else{
										cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
										found = 0;
										foundBlocked = 0;
									}
					    		}
					    	}else if(strcmp(list[10],command_str)==0 && logged_in){ //logout
					    		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
					    		createSendingString(client_socket,stdinBuffer);
					    		logged_in = 0;
					    		acceptBuffer = 1;
					    		FD_CLR(client_socket,&read_fds);
					    		close(client_socket);
					    		listFlag = 0;
					    		break;
					    	}else if(strcmp(list[11],command_str)==0){ //EXIT
					    		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
					    		//command exit
					    		cse4589_print_and_log("[%s:END]\n", command_str);
					    		logged_in = 0;
					    		acceptBuffer = 0;

					    		// createSendingString(client_socket,stdinBuffer);
					    		
					    		for(int sock=0;sock<=sockmax;sock++){
					    			if(FD_ISSET(sock,&master)){
					    				FD_CLR(sock,&master);
						    			FD_CLR(sock,&read_fds);
							    		close(sock);

					    			}
					    			
						    		
					    		}
					    		FD_ZERO(&master);
					    		FD_ZERO(&read_fds);
					    		
					    		
					    		exit(0);
					    	}else{
					    		cse4589_print_and_log("[%s:ERROR]\n", command_str);
					    	}
				    		cse4589_print_and_log("[%s:END]\n", command_str);
				    		break;

			    		}else{
			    			cse4589_print_and_log("[%s:ERROR]\n", command_str);
			    			cse4589_print_and_log("[%s:END]\n", command_str);
			    			break;
			    		}
		    		}
		    		
		    		
					
				}else if(FD_ISSET(i,&read_fds)){
				
					
					if(i == client_socket){
						if(newConnectionPassList && logged_in){
						
							int bytes_rcvd = receivebuffer(client_socket,5,receivedStr); //first receive length
				    		
				    		num_clients = breakTheResponse(receivedStr);
				    		
				    		newConnectionPassList = 0;
						
							
							
						
						}else{//for messages
							
							
							int bytes_rcvd = receivebuffer(client_socket,5,receivedStr); //first receive length
							
							// printf("\nEvent message Received\n");
							breakTheResponse(receivedStr);
							if(bytes_rcvd < 0){
								
								close(client_socket);
								FD_CLR(client_socket,&master);
								FD_CLR(client_socket,&read_fds);
								// exit(0);
						    }
						}
						memset(receivedStr,'\0',sizeof(receivedStr));
					}
					
					
				}
			}
			
			
			
		}
		
	}
	return 0;
}
