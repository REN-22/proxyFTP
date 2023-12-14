#include "./fonctions.h"
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PROXYADDR "127.0.0.1" // Définition de l'adresse IP d'écoute
#define PROXYPORT "44444"     // Définition port d'écoute, si 0: dynamique
#define LISTENLEN 1           // Taille de la file des demandes de connexion
#define MAXBUFFERLEN 1024     // Taille du tampon pour les échanges de données
#define MAXHOSTLEN 64         // Taille d'un nom de machine
#define MAXPORTLEN 64         // Taille d'un numéro de port

int main() {
  int ecode;                          // Code retour des fonctions
  char serverAddr[MAXHOSTLEN];        // Adresse du serveur
  char serverPort[MAXPORTLEN];        // Port du server
  char hostname[MAXBUFFERLEN] = "\0"; // nom d'hote du serveur // Ajouté
  pid_t idChild;                      // PID des processus enfants
  int descSockRDV;                    // Descripteur de socket de rendez-vous
  int descSockCOM;                    // Descripteur de socket de communication
  int descSockServer;    // Socket de communication avec le serveur // Ajouté
  struct addrinfo hints; // Contrôle la fonction getaddrinfo
  struct addrinfo *res;  // résultat de la fonction getaddrinfo
  struct sockaddr_storage myinfo; // Informations sur la connexion de RDV
  struct sockaddr_storage from;   // Informations sur le client connecté
  socklen_t len;                  // Variable utilisée pour stocker les
                                  // longueurs des structures de socket

  // Initialisation de la socket de RDV IPv4/TCP
  descSockRDV = socket(AF_INET, SOCK_STREAM, 0);
  if (descSockRDV == -1) {
    perror("Erreur création socket RDV\n");
    exit(2);
  }
  // Publication de la socket au niveau du système
  // Assignation d'une adresse IP et un numéro de port
  // Mise à zéro de hints
  memset(&hints, 0, sizeof(hints));
  // Initialisation de hints
  hints.ai_flags =
      AI_PASSIVE; // mode serveur, nous allons utiliser la fonction bind
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_family = AF_INET; // seules les adresses IPv4 seront présentées par
                             // la fonction getaddrinfo

  // Récupération des informations du serveur
  ecode = getaddrinfo(PROXYADDR, PROXYPORT, &hints, &res);
  if (ecode) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ecode));
    exit(1);
  }
  // Publication de la socket
  ecode = bind(descSockRDV, res->ai_addr, res->ai_addrlen);
  if (ecode == -1) {
    perror("Erreur liaison de la socket de RDV");
    exit(3);
  }
  // Nous n'avons plus besoin de cette liste chainée addrinfo
  freeaddrinfo(res);

  // Récuppération du nom de la machine et du numéro de port pour affichage à
  // l'écran
  len = sizeof(struct sockaddr_storage);
  ecode = getsockname(descSockRDV, (struct sockaddr *)&myinfo, &len);
  if (ecode == -1) {
    perror("SERVEUR: getsockname");
    exit(4);
  }
  ecode = getnameinfo((struct sockaddr *)&myinfo, sizeof(myinfo), serverAddr,
                      MAXHOSTLEN, serverPort, MAXPORTLEN,
                      NI_NUMERICHOST | NI_NUMERICSERV);
  if (ecode != 0) {
    fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(ecode));
    exit(4);
  }
  printf("L'adresse d'ecoute est: %s\n", serverAddr);
  printf("Le port d'ecoute est: %s\n", serverPort);

  // Definition de la taille du tampon contenant les demandes de connexion
  ecode = listen(descSockRDV, LISTENLEN);
  if (ecode == -1) {
    perror("Erreur initialisation buffer d'écoute");
    exit(5);
  }

  len = sizeof(struct sockaddr_storage);

  // Accepte toute les connections entrantes
  while (true) {
    // Attente connexion du client
    // Lorsque demande de connexion, creation d'une socket de communication avec
    // le client
    descSockCOM = accept(descSockRDV, (struct sockaddr *)&from, &len);
    if (descSockCOM == -1) {
      perror("Erreur accept\n");
      exit(6);
    }

    ecode = -1; // Code d’erreur si le proxy rencontre un problème

    idChild = fork();
    switch (idChild) {
    case -1:
      perror("Erreur création processus, essayez de vous reconnecter");
      break;
    case 0:
      // Authentifie l’utilisateur, si OK lance le cours normal du proxy
      if (loginServer(&descSockCOM, &descSockServer, hostname)) {
        printf("\n--- ENTRÉE DANS LA BOUCLE PRINCIPALE ---\n"); // Debug msg
        ecode = mainLoop(&descSockCOM, &descSockServer, hostname);

        printf("\nFermeture de la connection,\nClient: %d, Serveur %d, Code : "
               "%d\n",
               descSockCOM, descSockServer, ecode);
        // Fermeture des sockets
        close(descSockCOM);
        close(descSockServer);
        close(descSockRDV);
      }
      printf("\nFermeture processus\n");
      exit(ecode); // Met fin au processus fils
    }
  }

  printf("\nAu revoir !\n");
  return ecode;
}
