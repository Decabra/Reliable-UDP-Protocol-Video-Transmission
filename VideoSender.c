
/*
**
**The Client Side of a Program Which Sends a Video File to the Server Side
**
*/
#include<stdio.h> //For printf, file handling, and identifying eof
#include<arpa/inet.h> //To define internet operation
#include<stdlib.h> // For reading port number and filename arguemnts from user
#include<string.h> //To copy bytes into packets for window to send to server (strcpy())
#include<unistd.h> // In order to set the timeout for when to resend packets sleep()
#include<sys/socket.h> //To open the datagram socket
#include<netinet/in.h> //To incorporate IP Address, Port number, and AF_INET (IPv4) members
#include<stdbool.h> //Used to label packets as ACK'd or not
#include<sys/types.h> //For allocationg size objects

void displayMessage(const char *errMsg)
{
	printf("Error: %s\n", errMsg);
	exit(1);
}
int WINDOW_SIZE = 5;
int main(int argc, char *argv[])
{

if(argc < 3){
	displayMessage("Invalid arguments! Hint: ./obj_file file_name port_no" );
}

int socketPort = atoi(argv[2]);  //Port number provided by user to open socket on
struct sockaddr_in socketIP;
socklen_t socketIP_length = sizeof(socketIP);

//Opening the file to be transferred

FILE *fileToSend;
char packetBuffer[500];
int sequenceNumber = 0;    //Will be reset after 5 ACKs
char outgoingbytesBuffer[1];   //Buffer to store acks
bool ACKs[WINDOW_SIZE];   //Simple boolean to indicate whether packsts are ACK'd or not
char window[WINDOW_SIZE][500];
char fileName[32];

socketIP.sin_family = AF_INET;
socketIP.sin_port = htons(socketPort);   //user assigned port
socketIP.sin_addr.s_addr = INADDR_ANY;
//Creating Socket
int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
if(clientSocket == -1)   //Error in making socket
{
	printf("Socket not created\n");
	exit(0); // Program terminates
}
else
	printf("Socket created successfully\n");

printf("Enter Absolute Path of file: ");
//scanf("%s", fileName);

fileToSend = fopen(argv[1], "rb");

if(!fileToSend){
	printf("Unable to open the File\n");
	return 0;
}
printf("File Opened Successfully\n");


int u=0;
for (u=0;u<WINDOW_SIZE;u++){   //initializing to no ACKs received(yet)
ACKs[u]=false;
}

while(1)   //till end of file
{
	packetBuffer[0] = sequenceNumber;   //for sending sequence numbers to server
	fread(&packetBuffer[1], sizeof(char), sizeof(packetBuffer) - 1, fileToSend);   //Reading the bytes to be Sent
	sendto(clientSocket, packetBuffer,sizeof(packetBuffer), 0, (struct sockaddr *)&socketIP, socketIP_length); //Sending the bytes in form of packets
  printf("Sending packet with sequence number: %d\n", packetBuffer[0]);
	strcpy(window[sequenceNumber], packetBuffer);   //Bytes are saved in accordance to their sequence numbers
	if(recvfrom(clientSocket, outgoingbytesBuffer, sizeof(outgoingbytesBuffer), 0, (struct sockaddr *)&socketIP, &socketIP_length) > 0)
	{
    printf("Receiving Ack %d for sequence number: %d\n", outgoingbytesBuffer[0], packetBuffer[0]);
		ACKs[(int) outgoingbytesBuffer[0]] = true;     //if sender has sent acknowledgement on packet being sent successufuly
		memset(&outgoingbytesBuffer, 0, sizeof(outgoingbytesBuffer));
	}
	memset(&packetBuffer, 0, sizeof(packetBuffer));
	sequenceNumber++;   //Send next packet

	if(sequenceNumber == WINDOW_SIZE)  //for last packet in window
	{
		int i = 0;
		sequenceNumber = 0; //Start a new window of 5 packets
		for(i = 0; i < WINDOW_SIZE; i++)
		{
			if(ACKs[i] == false)     //Timeout
			{
				sleep(2);
				if(recvfrom(clientSocket, outgoingbytesBuffer, sizeof(outgoingbytesBuffer), 0, (struct sockaddr *)&socketIP, &socketIP_length) > 0)//if ack received after 2 sec
				{
          printf("Receiving Ack %d for sequence number: %d\n", outgoingbytesBuffer[0], packetBuffer[0]);
					ACKs[(int) outgoingbytesBuffer[0]] = true;
					memset(&outgoingbytesBuffer, 0, sizeof(outgoingbytesBuffer));
				}
				if(ACKs[i] == false) //Server does not acknowledge packet which has been sent for additional two seconds
				{
					sendto(clientSocket, window[i], sizeof(window[i]), 0, (struct sockaddr *)&socketIP, socketIP_length);  //Dispatching packet to receiver side again
          printf("Sending packet with sequence number: %d\n", window[i][0]);
					sleep(2);   //wait again for the ack
					if(recvfrom(clientSocket, outgoingbytesBuffer, sizeof(outgoingbytesBuffer), 0, (struct sockaddr *)&socketIP, &socketIP_length) > 0)
					{
            printf("Receiving Ack %d for sequence number: %d\n", outgoingbytesBuffer[0], window[i][0]);
						ACKs[(int) outgoingbytesBuffer[0]] = true;
						memset(&outgoingbytesBuffer, 0, sizeof(outgoingbytesBuffer));
					}
				}
			}
			ACKs[i] = false;
		}
		for(i = 0; i < WINDOW_SIZE; i++)
		{
			memset(&window[i], 0, sizeof(window[i]));
		}
	}
if (feof(fileToSend)) {
					printf("Successfully File sent!\n");
					sendto(clientSocket, "f\x00", 2, 0, (struct sockaddr *) &socketIP, sizeof(socketIP));
					printf("End of File\n");
					fclose(fileToSend);
					break;//Runs till last byte of file
				}
}//end while loop
close(clientSocket);    //close socket
return 0;
}
