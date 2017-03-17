/*

	IRCBOT.C
	This is simple ircbot which connect to server, register connect and check if there is error in pass, user and nick messages.
	If there is error with nick message, it will try another nickname or close the connect.
	This bot also listen ping messages and answer with pong. It also join to x channel and if kicked, then rejoin. 

	The bot is not ready, but it is sample of my code skills. If you just google how to code ircbot, I recommend to read: 
	http://beej.us/guide/bgnet/output/html/multipage/index.html
	https://tools.ietf.org/html/rfc2812

	Whit these links you can get started.	...Or just wait I completed this bot.


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
char buf[MESSAGE_SIZE];	

/*
		HOX! Fill careful followed things and read comment! Them will help you to configure bot
*/

int c = 2;																//Number of channels where to join. 
int n = 4;																//number of nicknames
int a = 1;																//Starting number of alternative nicknames. 0 is reserved for first nickname what bot will try.
const char port[4 + 1] = "xxxx";                           //Server's port.
const char hostname[26 + 1] = "xxxx";                      //Server's hostname.
const char startinginfo[2][59 + 1] = { "PASS none\r\n", "USER guest 8 * :NAME\r\n" }; // Send these messages to register connect. First password, then nick and then user message. 8 set user invisible fot other user. For register you also need nick messages. It will come later.
const char nicknames[4][9 + 1] = { "NAME1", "NAME2", "NAME3", "NAME4" };  // Set your nickname and alternative nicknames. Remember set n value to respond the number of nicknames.
const char channelnames[2][60 + 1] = { "#CHANNEL1", "#CHANNEL2" };					//join to channel. Use # before channel name. Remember set c to respond the number of channels.
const char msgchannel[2][60 + 1] = { "PRIVMSG #CHANNEL1 :Hi! I am bot.\r\n","PRIVMSG #CHANNEL2 :Hi! I am bot.\r\n"};	//Message to channel. For now, you need to set this up manually. "PRIMSG #channelName : 'message' \r\n" Do not use ' ' in your message.


/*
		Configure end.
*/

const char *ping = "PING :";                                                    //Code is using this for searching ping message.
char errorcode1[2][3 + 1] = { "461", "462" };                                   //Error codes for pass message and user message
char errorcode2[6][3 + 1] = { "431", "432", "433", "436", "437", "484" };       //Error codes for nick message
char *errorcheck;
char *pingmessage;
char startofnick[18 + 1] = "NICK ";
char endofcommand[4 + 1] = "\r\n";
char starofchan[6] = "JOIN :";
char channel[74 + 1];
char *hastag;
char *kicked;


void connecting(void)                           //Take connect to server.
{
	memset(&hints, 0, sizeof(hints));							// Set zero to struct of addrinfo.
	hints.ai_family = AF_INET;									  // Force socket to use IPv4.
	hints.ai_socktype = SOCK_STREAM;							// Force socket for stream.


	statusinfo = getaddrinfo(hostname, port, &hints, &result);	//Fill information (from hostname and port) to struct addrinfo

	if (result == NULL)
	{
		perror("Problem with struct addrinfo\n");

	}

	if (statusinfo != 0)                                       //!= because getaddrinfo returns nonzero if error and zero for succeeds.
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
	pingmessage = strstr(buf, ping);                            //Search ping message from buf.
	if (pingmessage != NULL)                                   // if ping is found.
	{
		pingmessage[1] = 'O';
		int a = send(sock, pingmessage, strlen(pingmessage), 0);  // send answer where ping's 'i' is 'o'.
		if (a < 0)
		{
			perror("Problem with sending pong");
		}
	}
}

void main(void)
{
	connecting(); // Connect to the server

	//Now register connection to server with sending pass message, nick message and user message and answer to ping

	for (i = 0; i <= 1; i++)
	{
		send(sock, startinginfo[i], strlen(startinginfo[i]), 0); //send pass and user messages
	}
	char nick[30];												            //Then code build nick messages.
	memcpy(nick, startofnick, strlen(startofnick));
	memcpy(nick + strlen(startofnick), nicknames[0], strlen(nicknames[0]));
	memcpy(nick + strlen(startofnick) + strlen(nicknames[0]), endofcommand, strlen(endofcommand) + 1);
	send(sock, nick, strlen(nick), 0);

	for (d = 1; d <= 20; d++)                           //Listen 20 times to check error code from pass, nick or user messages
	{																		                //Also send join message and message to channel
	begin:
		memset(buf, 0, sizeof(buf));										  //Set buf to zero.
		recvmessage = recv(sock, buf, sizeof(buf) - 1, 0);//Receive messages. -1 is for avoid buffer overflow. It is reserve '\0' to end of the message
		buf[recvmessage] = '\0';                          //Set "\0" to end of the buf.

		//printf("%s\n", buf);                            //Print received messages.

		pingcheck();

		for (i = 0; i <= 1; i++)                          //For each received messages, code check if there is any error numeric code.
			errorcheck = strstr(buf, errorcode1[i]);				//Code will check error codes from errorcode1 array. Return NULL if there is no error
			if (errorcheck != NULL)									        // Check if there is error
			{ 
				printf("Error in pass or user message\n");	
				exit(0);											                //If there is error, exit.
			}

		}

		for (i = 0; i <= 5; i++)                         //For each received messages, code check if there is error in nick message
		{
			errorcheck = strstr(buf, errorcode2[i]);       //Check if there is error in nick message.
			if (errorcheck != NULL)                        //If there is error...
			{
				if (a == n)                                  //If s = 4, all nicknames are returned with error
				{
					exit(1);
				}
				for (p = a; p < n; p++)                       //If there was error, send alternative nick message.
				{
					memcpy(nick, startofnick, strlen(startofnick));	            //Build nick message with alternative nickname
					memcpy(nick + strlen(startofnick), nicknames[a], strlen(nicknames[a]));
					memcpy(nick + strlen(startofnick) + strlen(nicknames[a]), endofcommand, strlen(endofcommand) + 1);
					send(sock, nick, strlen(nick), 0);
					a = ++a;									                   //And add one to s variable. Next time it will send next nick if needed.
					goto begin;                                 //Go back to begin of for loop and start all over again


				}
			}
		}

		char *welcome;
		char replyirc[32] = ":";                           //Code is building char string to detect IRC welcome$
		char numericCode[5] = " 001";

		strcat(replyirc, hostname);							           //Copy hostname to replyirc. Now replyirc is like ":hostname"
		strcat(replyirc, numericCode);                     //Copy numericCode to replyirc. Now replyirc is like ":hostname 001"

		welcome = strstr(buf, replyirc);                   //Search welcome message from buf.
		if (welcome != NULL)                               //If there is welcome message.
		{
			for (i = 0; i < c; i++)
			{
				memcpy(channel, starofchan, strlen(starofchan));										  //Build up join messages. For example "JOIN :#channelName\r\n"
				memcpy(channel + strlen(starofchan), channelnames[i], strlen(channelnames[i]));
				memcpy(channel + strlen(starofchan) + strlen(channelnames[i]), endofcommand, strlen(endofcommand) + 1);
				//printf("JOIN message is %s\n", channel);
				send(sock, channel, strlen(channel), 0);												      // Send join message to server
				send(sock, msgchannel[i], strlen(msgchannel[i]), 0);									//And then say hello.
			}
		}
	}

//After for loop, start while loop if connection is succesfull
	while (conn == 0)
	{

		memset(buf, 0, sizeof(buf));                                           //Set buf to zero.
		recvmessage = recv(sock, buf, sizeof(buf) - 1, 0);						         //Receive messages. -1 is for avoid buffer overflow. It is reserve '\0' to end of th$
		buf[recvmessage] = '\0';                                               //Set "\0" to end of the buf.
		//printf("%s\n", buf);													                       //Print received messages.
		if (recvmessage == -1)                                                 //If -1, there is error
		{
			perror("Problem with receiving message\n");
			exit(0);

		}
		if (recvmessage == 0)                                                   //If recv return zero connection is closed.
		{
			printf("connection closed\n");
			close(sock);                                                          //Close socket
			exit(0);
		}
	

	pingcheck();															             //This check if there is ping message

	
		kicked = strstr(buf, " KICK ");											 //Search if bot is kicked from channel
		if (kicked != NULL)                                  // if KICK is found.
		{

			hastag = strstr(buf, "#");											  //Search # from message. Not necessary. Name can be found from kicked message also. For historic reason, I have been let this here.
			//printf("search channel: %s\n", hastag);			
					
			for (i = 0; i < c; i++)
			{
				chaname3 = strstr(hastag, channelnames[i]);		  //Then compare chaname and used channel names
				if (chaname3 != NULL)
				{
					
					memcpy(channel, starofchan, strlen(starofchan));			//Build up rejoin message
					memcpy(channel + strlen(starofchan), channelnames[i], strlen(channelnames[i]));
					memcpy(channel + strlen(starofchan) + strlen(channelnames[i]), endofcommand, strlen(endofcommand) + 1);
					printf("Rejoin message is %s\n", channel);
					send(sock, channel, strlen(channel), 0);						  // Send join message to server
				}
			}

		
		}

	}
}



