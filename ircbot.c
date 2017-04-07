/*
IRCBOT.C

This is simple ircbot which connect to server, register connect and check if there is error in pass, user and nick messages.
If there is error with nick message, it will try another nickname or close the connect.
This bot also listen ping messages and answer with pong. It also join to x channel and if kicked, then rejoin.
Bot will read received messages line by line.

If you just google how to code ircbot, I recommend to read:
http://beej.us/guide/bgnet/output/html/multipage/index.html
https://tools.ietf.org/html/rfc2812
Whit these links you can get started.   ...Or just wait I completed this bot.
*/

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#define MESSAGE_SIZE  512

struct addrinfo hints;
struct addrinfo *result;

int statusinfo;
int sock;
int conn;
int i, d, p;
int recvmessage;
int welcomeIsDone = 0;
char buf[MESSAGE_SIZE];
int bytenumber;
char buf2[10000];
const char *searchNull;
char destBuf[10000];
/*
HOX! Fill careful followed things and read comment! Them will help you to configure bot
*/

int c = 1;                                                                                      //Number of channels where to join.
int n = 2;                                                                                      //number of nicknames
int a = 1;                                                                                      //Starting number of alternative nicknames. 0 is reserved for first nickname what bot will try.
const char port[4 + 1] = "XXXX";									//Server's port.
const char hostname[26 + 1] = "XXXXX";						//Server's hostname.
const char startinginfo[2][59 + 1] = { "PASS none\r\n", "USER guest 8 * :bot\r\n" };			// Send these messages to register connect. First password, then nick and then user message. 8 set user invisible fot other user. For register you al$
const char nicknames[2][9 + 1] = { "XXXXX", "XXXXX" };						// Set your nickname and alternative nicknames. Remember set n value to respond the number of nicknames.
const char channelnames[1][60 + 1] = { "#XXXXX" };							//join to channel. Use # before channel name. Remember set c to respond the number of channels.
const char msgchannel[1][60 + 1] = { "PRIVMSG #XXXXXX :Hi! I am bot.\r\n" };				//Message to channel. For now, you need to set this up manually. "PRIMSG #channelName : 'message' \r\n" Do not use ' ' in your message.


/*
Configure end.
*/

const char *ping = "PING :";									//Code is using this for searching ping message.
char errorcode1[2][5 + 1] = { " 461 ", " 462 " };						//Error codes for pass message and user message
char errorcode2[6][5 + 1] = { " 431 ", " 432 ", " 433 ", " 436 ", " 437 ", " 484 " };       	//Error codes for nick message
char *errorcheck;
char *pingmessage;
char startofnick[18 + 1] = "NICK ";
char endofcommand[4 + 1] = "\r\n";
char starofchan[6] = "JOIN :";
char channel[74 + 1];
char *hastag;
char *kicked;
char *chaname3;
char *welcome;

void connecting(void)									//Take connect to server.
{
	memset(&hints, 0, sizeof(hints));						// Set zero to struct of addrinfo.
	hints.ai_family = AF_INET;							// Force socket to use IPv4.
	hints.ai_socktype = SOCK_STREAM;						// Force socket for stream.


	statusinfo = getaddrinfo(hostname, port, &hints, &result);      //Fill information (from hostname and port) to struct addrinfo

	if (result == NULL)
	{
		perror("Problem with struct addrinfo\n");

	}

	if (statusinfo != 0)						//!= because getaddrinfo returns nonzero if error and zero for succeeds.
	{

		fprintf(stderr, "Getaddrinfo error: %s\n", gai_strerror(statusinfo)); //Print standard error and translate it to human readable.
		exit(0);
	}

	sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol); //Create socket for to communication

	if (sock == -1)
	{
		perror("Problem with socket\n");                      //socket gives -1 if there is error
		close(sock);                                          //Close socket.
		freeaddrinfo(result);                                 //Free information from result
		exit(0);
	}

	conn = connect(sock, result->ai_addr, result->ai_addrlen);      //Take connect.


	if (conn == -1)
	{
		perror("Problem with connect\n");                        // Zero for successful and -1 for error.
		close(sock);                                             //Close socket.
		freeaddrinfo(result);                                    //Free information from result.
		exit(0);
	}

	freeaddrinfo(result);                                       //Free informations from result. We do not need them anymore.
}
void pingcheck(void)

{
	pingmessage = strstr(destBuf, ping);					//Search ping message from buf.
	if (pingmessage != NULL)                                    		// if ping is found.
	{
		pingmessage[1] = 'O';						//Replace I to O
		int a = send(sock, pingmessage, strlen(pingmessage), 0); 	// send answer where ping's 'i' is 'o'.
		printf("\n%s\n", pingmessage);
		if (a < 0)
		{
			perror("Problem with sending pong");
		}
	}
}

void main(void)
{
	connecting(); // Connect to the server.

	//Now register connection to server with sending pass message, nick message and user message and answer to ping.

	for (i = 0; i <= 1; i++)
	{
		send(sock, startinginfo[i], strlen(startinginfo[i]), 0);			//send pass and user messages
	}
	char nick[30];                                                          		//Then code build nick messages.
	memcpy(nick, startofnick, strlen(startofnick));
	memcpy(nick + strlen(startofnick), nicknames[0], strlen(nicknames[0]));
	memcpy(nick + strlen(startofnick) + strlen(nicknames[0]), endofcommand, strlen(endofcommand) + 1);
	send(sock, nick, strlen(nick), 0);

	do	//do - while until welcome message is sent to server
	{                           
		
	begin:
		memset(buf, 0, sizeof(buf));                                            //Set buf to zero.
		recvmessage = recv(sock, buf, sizeof(buf) - 1, 0);			//Receive messages. -1 is for avoid buffer overflow. It is reserve '\0' to end of the message.
		buf[recvmessage] = '\0';                                                //Set "\0" to end of the buf.
		memcpy(buf2 + strlen(buf2), buf, strlen(buf));				//Copy received message to working buffer.

		do{									//Do - while until there is no complete line available to read
			searchNull = strstr(buf2, "\r\n");				//Search end of the line from working buffer.
			if (searchNull != NULL)						//If end of the line is found.
			{
				bytenumber = searchNull - buf2;				//Then calculate position of the end of the line.
				memcpy(destBuf, buf2, bytenumber + 2);			//Copy one line to the destBuf.
				for (d = 0; d < bytenumber + 2; d++)			//Then shift bits and "delete" first line from working buffer.
				{
					for (i = 0; i < sizeof(buf2); i++)
					{
						buf2[i] = buf2[i + 1];
					}
				}
				printf("%s", destBuf);					//Print the line.
				pingcheck();						//Check is it ping message.
				for (i = 0; i <= 1; i++)				//For each received line, code check if there is any error numeric code.
				{
					errorcheck = strstr(destBuf, errorcode1[i]);                //Code will check error codes from errorcode1 array. Return NULL if there is no error.
					if (errorcheck != NULL)                                     //If there is error
					{
						printf("Error in pass or user message\n");
						exit(0);										        //If there is error, exit.
					}
				}
				for (i = 0; i <= 5; i++)				//For each received line, code check if there is error in nick message.
				{
					errorcheck = strstr(destBuf, errorcode2[i]);	//Check if there is error in nick message.
					if (errorcheck != NULL)				//If there is error...
					{
						if (a == n)				//If a == n, all nicknames are returned with error.
						{
							exit(1);
						}
						for (p = a; p < n; p++)			//If there was error, send alternative nick message.
						{
							memcpy(nick, startofnick, strlen(startofnick));     //Build nick message with alternative nickname
							memcpy(nick + strlen(startofnick), nicknames[a], strlen(nicknames[a]));
							memcpy(nick + strlen(startofnick) + strlen(nicknames[a]), endofcommand, strlen(endofcommand) + 1);
							send(sock, nick, strlen(nick), 0);
							a = ++a;                                        //And add one to s variable. Next time it will send next nickname to server if needed.
							memset(destBuf, '\0', sizeof(destBuf));
							goto begin;					//Go back to begin of do - while loop and start listen server if new nickname is valid.
						}
					}
				}
				char replyirc[32] = ":";			//Code is building char string to detect IRC welcome message.
				char numericCode[5] = " 001";

				strcat(replyirc, hostname);                    	//Copy hostname to replyirc. Now replyirc is like ":hostname"
				strcat(replyirc, numericCode);			//Copy numericCode to replyirc. Now replyirc is like ":hostname 001"
				welcome = strstr(destBuf, replyirc);		//Search welcome message from destBuf.
				if (welcome != NULL)				//If there is welcome message.
				{
					for (i = 0; i < c; i++)
					{
						memcpy(channel, starofchan, strlen(starofchan));                               //Build up join messages. For example "JOIN :#channelName\$
						memcpy(channel + strlen(starofchan), channelnames[i], strlen(channelnames[i]));
						memcpy(channel + strlen(starofchan) + strlen(channelnames[i]), endofcommand, strlen(endofcommand) + 1);
						//printf("JOIN message is %s\n", channel);
						send(sock, channel, strlen(channel), 0);                                        // Send join message to server
						send(sock, msgchannel[i], strlen(msgchannel[i]), 0);                            //And then send hello messages to channels.
					}
					welcomeIsDone = ++welcomeIsDone;		//If there was welcome message, add 1 to welcomeIsDone. This give permission to code move on.
				}
				memset(destBuf, '\0', sizeof(destBuf));			//Clear destBuf with \0.
			}

		} while (searchNull != NULL);						//While until searchNull is NULL. Then check if there is new messages available.
		
	} while (welcomeIsDone == 0);							//While until welcome messages is received.

	//After for do - while loop, start while loop if connection is succesfull
	//printf("\nwhile start\n");
	while (conn == 0)
	{
		memset(buf, 0, sizeof(buf));                                            //Set buf to zero.
		recvmessage = recv(sock, buf, sizeof(buf) - 1, 0);			//Receive messages. -1 is for avoid buffer overflow. It is reserve '\0' to end of the message.
		buf[recvmessage] = '\0';                                                //Set "\0" to end of the buf.
		memcpy(buf2 + strlen(buf2), buf, strlen(buf));				//Copy received message to working buffer.
		do{									//Do - while until there is no complete line available to read
			searchNull = strstr(buf2, "\r\n");				//Search end of the line from working buffer.
			if (searchNull != NULL)						//If end of the line is found.
			{
				bytenumber = searchNull - buf2;				//Then calculate position of the end of the line.
				memcpy(destBuf, buf2, bytenumber + 2);			//Copy one line to the destBuf.
				for (d = 0; d < bytenumber + 2; d++)			//Then shift bits and "delete" first line from working buffer.
				{
					for (i = 0; i < sizeof(buf2); i++)
					{
						buf2[i] = buf2[i + 1];
					}
				}
				printf("%s", destBuf);					//Print the line.
				if (recvmessage == -1)                                  //If -1, there is error.
				{
					perror("Problem with receiving message\n");
					exit(0);
				}
				if (recvmessage == 0)                                   //If recv return zero connection is closed.
				{
					printf("connection closed\n");
					close(sock);					//Close socket.
					exit(0);					//And exit.
				}
				pingcheck();						//This check if there is ping message
				kicked = strstr(destBuf, " KICK ");                    	//Search if bot is kicked from channel
				if (kicked != NULL)					// if KICK is found.
				{
					hastag = strstr(destBuf, "#");                  //Search # from message. Not necessary. Name can be found from kicked message also. For historic reason, I have let this here.
					//printf("search channel: %s\n", hastag);

					for (i = 0; i < c; i++)
					{
						chaname3 = strstr(hastag, channelnames[i]);        //Then find out the channel.
						if (chaname3 != NULL)
						{
							memcpy(channel, starofchan, strlen(starofchan));	//Build up rejoin message
							memcpy(channel + strlen(starofchan), channelnames[i], strlen(channelnames[i]));
							memcpy(channel + strlen(starofchan) + strlen(channelnames[i]), endofcommand, strlen(endofcommand) + 1);
							//printf("Rejoin message is %s\n", channel);
							send(sock, channel, strlen(channel), 0);		// Send join message to server
						}
					}
				}
				
				memset(destBuf, '\0', sizeof(destBuf));			//Clear destBuf with \0.
			}
		} while (searchNull != NULL);						//While until searchNull is NULL. Then check if there is new messages available.
	}
}
