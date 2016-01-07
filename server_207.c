#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <wiringPi.h>       //For Raspberry PI
#include <pthread.h>

#define ERROR -1
#define MAX_CLIENTS 5



/****************Defining COnstant***************/
char s;
static const int BUFFER_SIZE=2*1024;
int i,c,d,j,systemflag,bytesRead,flag=0,flag1=0,flag2=0,flag3=0,k=0,m=528;
FILE* fp;

/***************Function to Turn on the camera****************/

void cameraOn(void* newSock)
{
	flag=1;
        printf("start Recording\n");
			
	system("raspivid -o project.h264 -t 30000");
			
	printf("Stopped Recording\n");
	flag=0;
	pthread_exit(NULL);
}

/***************Function to handle the Electrical appliances and to know current status****************/

void DeviceHandle(int newsockfd)
{
	int n;
	wiringPiSetup ();
	pinMode (0, OUTPUT) ;
	pinMode (5, OUTPUT);
	pinMode (7, OUTPUT);
	recv(newsockfd,&s,sizeof s,0);
        printf("Command received from client\n");
	
	switch(s)
{

case 'A':
                /**********Turning light on*********/
                printf("Light ON\n");
		flag1=1;
		digitalWrite(7,HIGH);
		n=send(newsockfd,"Light On\n",30,0);
		break;
case 'B':
                /**********Turning light off*********/
                printf("Light OFF\n");
                flag1=0;
		digitalWrite(7,LOW);
	
		n=send(newsockfd,"Light Off\n",30,0);
		break;

case 'C':
{
                pthread_t thread;
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

                char buffer[BUFFER_SIZE];
		

		if(pthread_create(&thread,&attr,(void *(*)(void *))cameraOn,(void*)&newsockfd)<0)
		{
			perror("pthread_create error\n");
		} 
		
		FILE *fps;
		sleep(3);
		fps=fopen("project.h264","r");
		if(fps)
		{
			printf("After fopen\n");
			if(flag==1)
			{
                            while(1)
                            {
                                printf("Reading the file\n");
				bytesRead = fread(buffer,1,sizeof(buffer),fps);
		
                                if (bytesRead <= 0) break;
              
				int sendbytes=send(newsockfd,buffer,bytesRead,0);
				
                            }

			}
		printf("About to close file\n");

		fclose(fps);			
		}
		
		close(newsockfd);
		pthread_join(thread,NULL);
                printf("File is closed\n");
}
                break;

case 'E':
                /**********Turning Motor on*********/
		flag2=1;
		digitalWrite(0,HIGH);
		
		n=send(newsockfd,"Motor On\n",30,0);
		break;

case 'F':
                /**********Turning Motor off*********/
		flag2=0;
		digitalWrite(0,LOW);
		
		n=send(newsockfd,"Motor Off\n",30,0);
		break;
case 'H':
                /**********Turning buzzer on as an alarm indication*********/
                flag3=1;
		digitalWrite (5, HIGH) ;
				
		n=send(newsockfd,"led On\n",30,0);
		break;
	
case 'I':
                /**********Turning buzzer off*********/
                flag3=0;
		digitalWrite (5, LOW) ;		
		n=send(newsockfd,"led Off\n",30,0);
		break;
case 'G':
                /**********Status of all the appliances*********/

		if((flag1==1) && (flag2==1) && (flag3==1))
		{
                    n=send(newsockfd,"LightON\tMotorOn\tLedON\n",30,0);
		
		}

                else if(flag1==0 && flag2==0 && flag3==0)
		{
                    n=send(newsockfd,"Lightoff\tMotoroff\tLedOff\n",30,0);
		}

                else if(flag1==1 && flag2==0 &7 && flag3==0)
		{
                    n=send(newsockfd,"LightOn\tMotorOff\tLedOff\n",30,0);
		} 		

                else if(flag1==0 && flag2==1 && flag3==0)
                {
                    n=send(newsockfd,"LightOff\tMotorOn\tLedOff\n",30,0);
                }

                else if(flag1==0 && flag2==1 && flag3==1)
                {
                    n=send(newsockfd,"LightOff\tMotorOn\tLedOn\n",30,0);
                }       

                else if(flag1==1 && flag2==0 && flag3==1)
                {
                    n=send(newsockfd,"LightOn\tMotorOff\tLedOn\n",30,0);
                }

                else if(flag1==0 && flag2==0 && flag3==1)
                {
                    n=send(newsockfd,"LightOff\tMotorOff\tLedOn\n",30,0);
                }

                else if(flag1==1 && flag2==1 && flag3==0)
                {
                    n=send(newsockfd,"LightOn\tMotorOn\tLedOff\n",30,0);
                }

                else
                    n=send(newsockfd,"redlightOn\n",30,0);

                break;


}

}
	
int main(int argc, char* argv[])

{

	int sockfd, newsockfd, portno;
	socklen_t clilen;
	int n;
	struct sockaddr_in serv_addr, cli_addr;

	pthread_t thread1;
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1,PTHREAD_CREATE_JOINABLE);    

	if (argc < 2) 
	{
         	printf("ERROR, You have entered wrong Input.Please try again...\n");
         	exit(1);
	}

/********Socket*********/

     	sockfd = socket(AF_INET, SOCK_STREAM, 0);
     	if (sockfd < 0)
     	printf("ERROR in opening socket");

     	bzero((char *) &serv_addr, sizeof(serv_addr));
     	portno = atoi(argv[1]);
     	serv_addr.sin_family = AF_INET;
     	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
     	serv_addr.sin_port = htons(portno);

/********Binding*********/

     	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        printf("ERROR in binding Socket");

/********Listen*********/


     	if((listen(sockfd,MAX_CLIENTS))==ERROR)
     	{
             perror("Error in Listen:");
             exit(-1);

     	}


     	clilen = sizeof(cli_addr);
     	printf("Server is Listening...\n");
	while(1)
	{ 

    	/********Accept*********/

    	if((newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen))==ERROR)

        {
                perror("Error in Accept:");
                exit(-1);
        }
 
        if(pthread_create(&thread1,&attr1,(void *(*)(void *))DeviceHandle,(int*)newsockfd)<0)
	{
			perror("pthread_create error\n");
	} 
		
}
     close(newsockfd);
     close(sockfd);

     return 0;
}


