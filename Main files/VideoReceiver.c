/*
*
*Server Side of a Program which receives a video file from the user and writes it to a new file name.
*Program exits once transfer is complete.
*
*/
#include<stdio.h> //For printf, file handling, and identifying eof
#include<arpa/inet.h> //To define internet operation
#include<stdlib.h> // For reading port number and filename arguemnts from user
#include<string.h> //To copy bytes into packets for window to send to server (strcpy())
#include<unistd.h> // In order to set the timeout for when to resend packets sleep()
#include<sys/socket.h> //To open the datagram socket
#include<netinet/in.h> //To incorporate IP Address, Port number, and AF_INET (IPv4) members
#include<stdbool.h> //Used to label packets as ACK'd or not
#define IP_ADDRESS "127.0.0.1" // localhost

void displayMessage(const char *errMsg)
{
	printf("Error: %s\n", errMsg);
	exit(1);
}
int WINDOW_SIZE = 5;
void main(int argc, char *argv[])
{

if(argc < 3){
	displayMessage("Invalid arguments! Hint: ./obj_file file_name port_no");
}
int PORT = atoi(argv[2]);    //user assigned port number
struct sockaddr_in socketIP, clientIP;
//Opening the file to write incoming data to

FILE *fileToWrite;
char packetBuffer[500];
char outgoingACKs[1];
char window[WINDOW_SIZE][500];
int sequenceNumber = 0;    //Will be reset after 5 ACKs are sent
bool reOrder[WINDOW_SIZE]; //Simple boolean to indicate whether packets are in order, or not
int serverSocket;
socklen_t socketIP_length = sizeof(clientIP);

socketIP.sin_family = AF_INET;   //IPv4
socketIP.sin_port = htons(PORT);
socketIP.sin_addr.s_addr = inet_addr(IP_ADDRESS);


serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

if(serverSocket == -1)    //error in socket
{
printf("Socket not created\n");
exit(0);
}
else
printf("Socket created successfully\n");


if(bind(serverSocket, (struct sockaddr*)&socketIP, sizeof(socketIP))!=0)   //binding
printf("Not Binded\n");
else
printf("Binded\n");

printf("The server is listening\n");


fileToWrite = fopen(argv[1], "wb");   //open file

while(recvfrom(serverSocket, packetBuffer, sizeof(packetBuffer), 0, (struct sockaddr *)&clientIP, &socketIP_length) > 0)
{
	printf("Recieving packet with sequence number: %d\n", packetBuffer[0]);
	strcpy(window[sequenceNumber], packetBuffer);   // Incoming bytes are saved
	outgoingACKs[0] = packetBuffer[0];    //Incoming bytes are buffered
	sendto(serverSocket, outgoingACKs, sizeof(outgoingACKs), 0, (struct sockaddr *)&clientIP, socketIP_length);   //sending ack to client
	printf("Sending Ack %d for sequence number: %d\n", outgoingACKs[0], packetBuffer[0]);

	if(!strcmp(packetBuffer, "f\x00")){
	    		printf("End of File\nThe Video file received successfully.\n");
	    		fclose(fileToWrite);
	    		printf("File Closed.\n");
	    		close(serverSocket);
	    		printf("Socket Closed.\n");
	    		printf("Video file has been created in your directory\n");
  }
	//Acknowledging receipt of Packet to Client Program
	if(sequenceNumber != (int) packetBuffer[0])   //Out of order Packet received
	{
		reOrder[sequenceNumber]=false;
		memset(&packetBuffer, 0, sizeof(packetBuffer));//Store in buffer for now, until turn for sequence comes
	}
	else{
		reOrder[sequenceNumber]=true;   //Packet received is in order
		fwrite(&packetBuffer[1], sizeof(char), sizeof(packetBuffer) - 1, fileToWrite); //Correctr data is written in order to file
		memset(&packetBuffer, 0, sizeof(packetBuffer));
	}
	sequenceNumber++;

	if(sequenceNumber == WINDOW_SIZE)
	{
		sequenceNumber = 0;
		int n=0;
		for(n=0;n<WINDOW_SIZE;n++){
			if (reOrder[n]==false){   //now write the bytes buffered before in order
				fwrite(&window[n], sizeof(char), sizeof(packetBuffer) - 1, fileToWrite); //Packets which were received out of order
				//are now written to the file in the corresponding window frame size
			}
		}
	}
}
}

