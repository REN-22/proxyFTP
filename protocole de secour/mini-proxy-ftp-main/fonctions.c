#include "./simpleSocketAPI.h"
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVPORT "21"     // Définition du port du serveur distant
#define MAXBUFFERLEN 1024 // Taille du tampon pour les échanges de données

int readFromSock(int *descSock, char *left, char *right, char *buffer) {
  int bufferLen; // Nombre de caractères lus dans le buffer

  printf("%d\n", *descSock); // Debug message

  // Lit l’entrée utilisateur et la coupe en 2 au premier espace
  bufferLen = read(*descSock, buffer, MAXBUFFERLEN - 1);
  buffer[bufferLen] = '\0';
  sscanf(buffer, "%s %[^\n]", left, right);

  // Affiche le contenu du buffer, et le découpage pour debug
  printf("Message reçu\n%s\n", buffer);
  printf("COMMANDE/CODE : %s\t", left);
  printf("PARAMÈTRE/MESSAGE : %s\n", right);

  return bufferLen;
}

bool loginServer(int *sockClient, int *sockServer, char *hostname) {
  char welcome[MAXBUFFERLEN] = // message d’accueil
      "220 Veuillez vous authentifier (username@hostname)\t\n";
  char cmd[MAXBUFFERLEN];      // commande utilisateur
  char param[MAXBUFFERLEN];    // paramètre de la commande
  char username[MAXBUFFERLEN]; // nom d'utilisateur
  bool connected = false;      // socket initialisé ou non
  char code[MAXBUFFERLEN];     // code de retour du serveur
  char msg[MAXBUFFERLEN];      // message associé au retour serveur
  char buffer[MAXBUFFERLEN];   // Tampon de communication

  // Affichage du message d’accueil
  write(*sockClient, welcome, strlen(welcome));

  while (true) {

    printf("\nATTENTE DE RÉCEPTION DEPUIS CLIENT "); // Debug message
    // Réception de l’entrée client
    readFromSock(sockClient, cmd, param, buffer);

    // effectue une action en fonction de la commande utilisateur
    if (strcmp(cmd, "QUIT") == 0) {
      printf("\n! QUIT reçu depuis le client, envoi 221 puis fermeture\n");
      strcpy(buffer, "221 À bientôt\r\n");
      write(*sockClient, buffer, strlen(buffer));
      return false; // Met fin au proxy si commande QUIT
    } else if (strcmp(cmd, "USER") == 0) {
      // Séparation nom d’utilisateur et nom d’hôte
      sscanf(param, "%[^@]@%s", username, hostname);
      if (strlen(hostname) <= 1) {
        // Si mauvais formatage username@hostname, indique erreur identifiants
        strcpy(buffer, "430 mauvais formatage identifiants\r\n");
        write(*sockClient, buffer, strlen(buffer));
        printf("\nERREUR TRANSMISE AU CLIENT\n%s\n", buffer); // Debug
        continue;
      }
      // Connection au serveur avec le nom d’hôte
      if (connect2Server(hostname, SERVPORT, sockServer) == 0) {
        printf("\n! Connection au serveur %s sur le port %s RÉUSSIE\n",
               hostname,
               SERVPORT); // Debug
        connected = true;
      } else {
        printf("\n! Connection au serveur %s sur le port %s ÉCHOUÉE\n",
               hostname,
               SERVPORT); // Debug
        // Si erreur connexion, indique erreur identifiants
        strcpy(buffer, "530 mauvais nom d’hôte ou erreur de connexion\r\n");
        write(*sockClient, buffer, strlen(buffer));
        printf("\nERREUR TRANSMISE AU CLIENT\n%s\n", buffer); // Debug
        continue;
      };
    } else if (strcmp(cmd, "SYST") == 0) {
      // Si commande SYST, indique une erreur temporaire au client
      strcpy(buffer, "534 veuillez vous connecter avant\r\n");
      write(*sockClient, buffer, strlen(buffer));
      printf("\nERREUR TRANSMISE AU CLIENT\n%s\n", buffer); // Debug
      continue;
    }

    // si le socket a bien été initialisé, traite la réponse du serveur
    if (connected) {
      readFromSock(sockServer, code, msg, buffer);
      // Si le serveur a bien répondu à la connexion
      if (strcmp(code, "220") == 0) {
        // Envoi nom d’utilisateur au serveur
        sprintf(buffer, "USER %s\r\n", username);
        write(*sockServer, buffer, strlen(buffer));
        printf("\nNOM UTILISATEUR TRANSMIS AU SERVEUR : %s\n", buffer); // Debug

        /*
        cmd[0] = '\0';    // Vidange des variables
        param[0] = '\0';  // Vidange des variables
        buffer[0] = '\0'; // Vidange des variables
        code[0] = '\0';   // Vidange des variables
        msg[0] = '\0';    // Vidange des variables
        buffer[0] = '\0'; // Vidange des variables
        */

        return true;
      } else {
        printf("Le serveur %s n’a pas répondu 220, RÉESSAI\n",
               hostname); // Debug
      }
    }
  }
}

int mainLoop(int *sockClient, int *sockServer, char *hostname) {
  int sockDataClient; // Socket de données avec le client
  int sockDataServer; // Socket de données avec le serveur
  unsigned int ip1, ip2, ip3, ip4, pt1, pt2, pt3; // bits d’ip et de port
  char buffer[MAXBUFFERLEN];                      // Tampon de communication
  char code[MAXBUFFERLEN];                        // code de retour du serveur
  char msg[MAXBUFFERLEN];      // message associé au retour serveur
  char cmd[MAXBUFFERLEN];      // commande utilisateur
  char param[MAXBUFFERLEN];    // parametre de la commande
  char clientIP[MAXBUFFERLEN]; // adresse ip du client
  char clientDataPort[5];      // Port de la connexion données client
  char serverDataPort[5];      // Port de la connexion données serveur

  while (true) {
    printf("\nATTENTE DE RÉCEPTION DEPUIS SERVEUR "); // Debug message
    readFromSock(sockServer, code, msg, buffer);

    if (strcmp(code, "229") == 0) {
      // Lance le canal de données si le serveur répond au mode passif
      printf("\n! 229 reçu depuis le serveur, démarage connexion données\n");
      sscanf(msg, "%*[^(](|||%d|)", &pt3);
      sprintf(serverDataPort, "%d", pt3);
      if (connect2Server(hostname, serverDataPort, &sockDataServer) == 0) {
        printf("\n! Connection au serveur %s sur le port %s RÉUSSIE\n",
               hostname,
               serverDataPort); // Debug
        sprintf(buffer, "200 PORT %s:%s OK\r\n", clientIP, clientDataPort);
        write(*sockClient, buffer, strlen(buffer));
        printf("\nPORT OK AU CLIENT\n%s", buffer); // Debug
      } else {
        printf("\n! Connection au serveur %s sur le port %s ÉCHOUÉE\n",
               hostname,
               serverDataPort); // Debug
      };
    } else if (strcmp(code, "150") == 0) {
      // Transfère au client le message indiquant le départ du transfert
      write(*sockClient, buffer, strlen(buffer));
      printf("\nRetour serveur transmis au client\n%s\n", buffer); // Debug
      // Transfère les données au client sur la socket de données, si besoin
      printf("\nATTENTE DE RÉCEPTION DEPUIS CANAL DONNÉES SERVEUR "); // Debug
      readFromSock(&sockDataServer, code, msg, buffer);
      write(sockDataClient, buffer, strlen(buffer));
      // Ferme les connexions
      printf("\nDONNÉES SERVEUR TRANSMISES AU CLIENT\n%s", buffer); // Debug
      close(sockDataClient); // Fermeture connexion données
      close(sockDataServer); // Fermeture connexion données
      printf("\n! fin du transfert de données\n"); // Debug
      // Lecture du code de retour après fermeture
      printf("\nATTENTE DE RÉCEPTION CODE DE RETOUR SERVEUR "); // Debug
      readFromSock(sockServer, code, msg, buffer);
      write(*sockClient, buffer, strlen(buffer));
      printf("\nRetour serveur transmis au client\n%s\n", buffer); // Debug
    } else if (strlen(buffer) > 0) {
      // Transmet au client la réponse du serveur, si besoin
      write(*sockClient, buffer, strlen(buffer));
      printf("\nRETOUR SERVEUR TRANSMIS AU CLIENT\n%s", buffer); // Debug
    }

    if (strcmp(code, "221") == 0) {
      // Met fin au proxy comme indiqué par le serveur
      printf("\n! 221 reçu depuis le serveur");
      close(sockDataClient);
      close(sockDataServer);
      return 0;
    }

    /*
    code[0] = '\0';   // Vidange des variables
    msg[0] = '\0';    // Vidange des variables
    buffer[0] = '\0'; // Vidange des variables
    */

    printf("\nATTENTE DE RÉCEPTION DEPUIS CLIENT "); // Debug message
    readFromSock(sockClient, cmd, param, buffer);

    if (strcmp(cmd, "PORT") == 0) {
      printf("\n! PORT reçu depuis le client\n");
      // reconstruction de l’ip et du port du client à partir de PORT
      sscanf(param, "%d,%d,%d,%d,%d,%d", &ip1, &ip2, &ip3, &ip4, &pt1, &pt2);
      sprintf(clientIP, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
      printf("IP client reconstruite : %s\n", clientIP); // Debug
      sprintf(clientDataPort, "%d", pt1 * 256 + pt2);
      printf("Port client reconstruit : %s\n", clientDataPort); // Debug
      // Initialise la connexion données client
      if (connect2Server("localhost", clientDataPort, &sockDataClient) == 0) {
        printf("\n! Connection au client sur le port %s RÉUSSIE\n",
               clientDataPort); // Debug
      } else {
        printf("\n! Connection au client sur le port %s ÉCHOUÉE\n",
               clientDataPort); // Debug
      };
      // Indique au serveur la volonté de passer en passif
      // write(*sockServer, "PASV\r\n", 6); // Interdit sur ftp.fau.de
      write(*sockServer, "EPSV\r\n", 6);
      printf("Passage en mode passif demandé au serveur\n");
    } else if (strlen(buffer) > 0) {
      // Envoie l’entrée utilisateur au serveur si besoin
      write(*sockServer, buffer, strlen(buffer));
      printf("\nENTRÉE UTILISATEUR TRANSMISE AU SERVEUR\n%s", buffer); // Debug
    }

    /*
    cmd[0] = '\0';    // Vidange des variables
    param[0] = '\0';  // Vidange des variables
    buffer[0] = '\0'; // Vidange des variables
    */
    /*
    clientIP[0] = '\0';       // Vidange des variables
    clientDataPort[0] = '\0'; // Vidange des variables
    serverDataPort[0] = '\0'; // Vidange des variables
    */
  }
}
