# Mini Proxy FTP

## Programmation Réseau : Réalisation d’un mini-programme proxy FTP

On désire utiliser le client FTP sous DOS. Celui-ci ne dispose pas du mode passif. Par contre, on ne peut aller sur le réseau public à cause d’un problème de décalage de signalisation. On vous demande donc de réaliser un proxy qui permette de prendre en charge le mode actif d’un côté et de relayer en mode passif de l’autre. Un utilisateur souhaitant établir une session FTP via le proxy s’identifiera en tant que nomlogin@nomserveur. Le programme client établit une connexion de contrôle avec le proxy et lui transmet la commande USER nomlogin@nomserveur qui permet au proxy d’établir une connexion de contrôle sur la machine de nom nomserveur et d’identifier l’utilisateur à l’aide de nomlogin. Le proxy doit pouvoir tourner sur n’importe quelle machine. Le reste des échanges est conforme au protocole FTP. Le travail à réaliser consiste à développer un proxy FTP actif/passif permettant de récupérer le résultat de la commande utilisateur ls en communiquant avec le serveur en mode passif et en mode actif avec le client. Le proxy doit gérer plusieurs sessions FTP.

### 1

Dans un premier temps, il est demandé de gérer les connexions de contrôle de part et d’autre afin de relayer les commandes et réponses du protocole FTP.

### 2

Dans un second temps on gérera les connexions de données.
L’évaluation du travail se fera lors de la dernière séance de TP.
