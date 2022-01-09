#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

/* portul folosit */
#define PORT 2095

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/*functia de tratare a semnalului*/
void sighandler(int sig){
  while (0 < waitpid(-1, NULL, WNOHANG)){

  }
}

#define MAX 80

char msg[100];		           // mesajul primit de la client 
char message[MAX];
int client;
int nr_clienti = 3, nr_clienti_conectati = 0;

int main ()
{

  struct sockaddr_in server; // structura folosita de server care contine diferite informatii
  struct sockaddr_in from;	 // structura folosita pentru a stoca informatii despre clienti
  char msgrasp[100] = " ";   // mesaj de raspuns pentru client
  int sd;			               // descriptorul de socket 
 

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1){

      perror ("[server] Eroare la socket().\n");
      return errno;
    }


  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server)); // am initializat structura server
  bzero (&from, sizeof (from));     // am initializat structura from

  
  /* umplem structura folosita de server */
  server.sin_family = AF_INET; 	              // stabilirea familiei de socket-uri 
  server.sin_addr.s_addr = htonl(INADDR_ANY); // acceptam orice adresa 
  server.sin_port = htons(PORT);              // utilizam un port utilizator 


  int flag = 1;   //atunci cand imi da eroare la bind
  if(-1 == setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))){
    perror("setsockopt fail");
  }
  

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1){

      perror ("[server] Eroare la bind().\n");
      return errno;
    }


  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 5) == -1){

      perror ("[server] Eroare la listen().\n");
      return errno;
    }

 
  int length = sizeof(from);
  
  /* servim in mod concurent clientii... */  
  printf ("[server] Asteptam la portul %d, %d clienti...\n\n", PORT, nr_clienti);
  fflush (stdout);

   while (1){

      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      printf("[1]Nr clientilor inainte de a accepta un client este %d\n", nr_clienti_conectati);

      client = accept (sd, (struct sockaddr *) &from, &length);
      nr_clienti_conectati++;

      printf("[2]Nr clientilor dupa acceptarea unui client este %d\n", nr_clienti_conectati);

      /* eroare la acceptarea conexiunii de la un client */
      if (client < 0)
      {
	       perror ("[server] Eroare la accept().\n");
	       continue;
	    }


      int pid = fork();
      if(pid != 0){

          //parinte

          close(client); //inchidem socketul
          if(signal(SIGCHLD, sighandler) == SIG_ERR){
            perror("signal()");
            return 1;
          }
          continue;
      }
      else
      {

        //copil
        
        /* s-a realizat conexiunea, se astepta mesajul */
        int length;
        printf ("[server] Astept mesajul de la client\n");
        fflush (stdout);
        
        /* citirea mesajului */
        if (read (client, msg, 100) <= 0){

          perror ("[server] Eroare la read() de la client.\n");
          close (client);	/* inchidem conexiunea cu clientul */
          continue;	    	/* continuam sa ascultam */
        }

    
        printf ("[server] %s join!\n\n", msg);
        fflush (stdout);
       
       
        /*pregatim mesajul de raspuns */
        bzero(msgrasp,100);
        strcat(msgrasp,"Hello ");
        strcat(msgrasp,msg);
        
        //printf("[server]Trimitem mesajul inapoi...%s\n", msgrasp);
        
        
        /* returnam mesajul clientului */
        if (write (client, msgrasp, 100) <= 0){

          perror ("[server] Eroare la write() catre client.\n");
          continue;		/* continuam sa ascultam */
        }
        else{
         // printf ("[server]Mesajul a fost trasmis cu succes.\n");
        }

        //am terminat cu acest client, inchidem conexiunea
       
        close (client);
        //exit(0);
        
    } /*else*/

    if(nr_clienti_conectati == nr_clienti){
      printf("Terminat!\n");
      return 0;
    }

  } /* while */	

} /* main */
