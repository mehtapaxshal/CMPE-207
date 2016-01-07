#include <stdio.h>
#include <gtk/gtk.h>		//for GTK widget GUI
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdlib.h>
#include <glib.h>		//include GTK libraries


int clientSocket;
struct sockaddr_in serverAddr;
socklen_t addr_size;
char rqust[20],ack[20],buffer[30];
static const int BUFFER_SIZE= 2*1024;
int flag=0;
pthread_t thread;
pthread_attr_t attr;
GtkWidget *window, *button, *table;
pthread_t thread1;
pthread_attr_t attr1;

/****************video receive function**********************/

void video_read(int clientSocket)
{
	FILE *fps;	
	int bytesReceived;
	fps=fopen("vod.h264","w");
	if(fps)
	{
		char buff[BUFFER_SIZE];
		while(1)
		{	
			bytesReceived=recv(clientSocket,buff,sizeof (buff),0);
			
			if (bytesReceived < 0) 
				perror("recv");
			if (bytesReceived == 0) 
				break;   
			printf("The no. of bytes received of n is %d\n",bytesReceived);
			
			if(fwrite(buff,1,bytesReceived,fps)!=(size_t) bytesReceived)
			{
				perror("fwrite\n");
				break;
			}
			else
			{			
			flag=1;
			}
		}	
		
		fclose(fps);
	}

	pthread_exit(NULL);
}

/****************video display function**********************/

void VLCservice()
{

	while(1)
	{
		if(flag==1)
		break;	
		else		//flag=0;
		sleep(1);	
	}	
	
	system("vlc --rate=0.52 --no-playlist-autostart -vvv vod.h264 --tcp-caching =300 vlc://quit");	
	
	pthread_exit(NULL);
}

/****************send request for light**********************/

static void lightOn(GtkWidget *widget, gpointer data)
{	
	send(clientSocket,"A",sizeof(char),0);	
	recv(clientSocket,buffer,sizeof(buffer),0);
	printf("Server reply: %s\n",buffer);
	
}

static void lightOff(GtkWidget *widget, gpointer data)
{
	send(clientSocket,"B",sizeof(char),0);	
	recv(clientSocket,buffer,sizeof(buffer),0);
	printf("Server reply: %s\n",buffer);
}

/**************send request for LED (alert)******************/

static void ledOn(GtkWidget *widget, gpointer data)
{	
	send(clientSocket,"H",sizeof(char),0);	
	recv(clientSocket,buffer,sizeof(buffer),0);
	printf("Server reply: %s\n",buffer);
	
}

static void ledOff(GtkWidget *widget, gpointer data)
{
	send(clientSocket,"I",sizeof(char),0);	
	recv(clientSocket,buffer,sizeof(buffer),0);
	printf("Server reply: %s\n",buffer);
}

/****************send request for motor**********************/

static void motorOn(GtkWidget *widget, gpointer data)
{
	send(clientSocket,"E",sizeof(char),0);		
	recv(clientSocket,buffer,sizeof(buffer),0);
	printf("Server reply: %s\n",buffer);
}

static void motorOff(GtkWidget *widget, gpointer data)
{
	send(clientSocket,"F",sizeof(char),0);	
	recv(clientSocket,buffer,sizeof(buffer),0);
	printf("Server reply: %s\n",buffer);

}

/****************send request for video**********************/

static void video(GtkWidget *widget, gpointer data)
{


	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);	

	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_JOINABLE);	

	send(clientSocket,"C",sizeof(char),0);	

	if(pthread_create(&thread, &attr, (void * (*) (void *))VLCservice,NULL) < 0)
		perror("pthread_create error\n");

	if(pthread_create(&thread1, &attr1, (void * (*) (void *))video_read,(int*)clientSocket) < 0)
		perror("pthread_create error\n");
	
	pthread_join(thread1,NULL);
	pthread_join(thread,NULL);

}

/****************send request for status**********************/

static void Status(GtkWidget *widget, gpointer data)
{

	send(clientSocket,"G",sizeof(char),0);  

	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,"%s", message);

	gtk_window_set_title(GTK_WINDOW(dialog), "Alert!");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	

        recv(clientSocket,buffer,sizeof(buffer),0);
        char *message = buffer;

	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,"%s", message);

	gtk_window_set_title(GTK_WINDOW(dialog), "Status");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	printf("Server reply: %s\n",buffer);
        
}

/****************handle delete event for GUI********************/

static gboolean delete_event(GtkWidget *widget, GdkEvent *event,
	gpointer data)
{
	g_print("delete event occurred\n");
	
	close(clientSocket);
	exit (0); /* return FALSE, instead, to destroy the main window */
}

/****************destroy event for GUI**********************/

static void destroy(GtkWidget *widget, gpointer data )
{
	
	gtk_main_quit();
}

/********************main function**************************/

int main(int argc, char *argv[])
{
	
	while(1)
	{
	char buffer[1024];
	int i=0;

	int port=atoi(argv[2]);
	
	if(argc <=2)
	{
		printf("missing parameters\n");
		return 1;
	}	
	
	clientSocket = socket(PF_INET, SOCK_STREAM, 0);
  
	if(clientSocket == -1)
	printf("Socket not created\n");


	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

	memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));  

        addr_size = sizeof(serverAddr);
	
  
        if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size) < 0)
        {
                perror("Connection error\n");
                return;     
        }

	

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	g_signal_connect(G_OBJECT(window), "delete_event",
		G_CALLBACK(delete_event), NULL);
	g_signal_connect(G_OBJECT(window), "destroy",G_CALLBACK(destroy), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);


	table=gtk_table_new(3,3,TRUE);

	gtk_container_add(GTK_CONTAINER(window), table);

	button = gtk_button_new_with_label("Light Turn ON");
	g_signal_connect(G_OBJECT(button), "clicked",
		G_CALLBACK(lightOn), NULL);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
		G_CALLBACK(gtk_widget_destroy), G_OBJECT(window));
	

	gtk_table_attach_defaults(GTK_TABLE(table),button,0,1,0,1);	

	button = gtk_button_new_with_label("Light Turn OFF");
	g_signal_connect(G_OBJECT(button), "clicked",
		G_CALLBACK(lightOff), NULL);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
		G_CALLBACK(gtk_widget_destroy), G_OBJECT(window));
		

	gtk_table_attach_defaults(GTK_TABLE(table),button,2,3,0,1);	

	button = gtk_button_new_with_label("Motor Turn ON");
	
	g_signal_connect(G_OBJECT(button), "clicked",
		G_CALLBACK(motorOn), NULL);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
		G_CALLBACK(gtk_widget_destroy), G_OBJECT(window));
		

	gtk_table_attach_defaults(GTK_TABLE(table),button,0,1,1,2);	

	button = gtk_button_new_with_label("Motor Turn OFF");
	g_signal_connect(G_OBJECT(button), "clicked",
		G_CALLBACK(motorOff), NULL);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
		G_CALLBACK(gtk_widget_destroy), G_OBJECT(window));
		

	gtk_table_attach_defaults(GTK_TABLE(table),button,2,3,1,2);	

	button = gtk_button_new_with_label("Led ON");
	g_signal_connect(G_OBJECT(button), "clicked",
		G_CALLBACK(ledOn), NULL);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
		G_CALLBACK(gtk_widget_destroy), G_OBJECT(window));
		

	gtk_table_attach_defaults(GTK_TABLE(table),button,0,1,2,3);

	button = gtk_button_new_with_label("Led OFF");
	g_signal_connect(G_OBJECT(button), "clicked",
		G_CALLBACK(ledOff), NULL);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
		G_CALLBACK(gtk_widget_destroy), G_OBJECT(window));
		

	gtk_table_attach_defaults(GTK_TABLE(table),button,2,3,2,3);

	button = gtk_button_new_with_label("Status");
	g_signal_connect(G_OBJECT(button), "clicked",
		G_CALLBACK(Status), NULL);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
		G_CALLBACK(gtk_widget_destroy), G_OBJECT(window));
		

	gtk_table_attach_defaults(GTK_TABLE(table),button,1,2,3,4);	

	button = gtk_button_new_with_label("Video");
	g_signal_connect(G_OBJECT(button), "clicked",
		G_CALLBACK(video), NULL);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
		G_CALLBACK(gtk_widget_destroy), G_OBJECT(window));
	

	gtk_table_attach_defaults(GTK_TABLE(table),button,1,2,4,5);	
	
	gtk_table_set_row_spacings(GTK_TABLE(table),10);

	gtk_widget_show_all(window);
	
	gtk_main();

	}
	return 0;
}
