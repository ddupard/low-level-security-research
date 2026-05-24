Voici la carte des grandes unités logiques que tu vas croiser dans ton exploration :

## 1. Le Process Management (kernel/)
C'est le Cœur et le Cerveau. Il gère la vie et la mort des processus.
Fonctions : Création de processus (fork), ordonnancement (scheduler), signaux, interruption.
Objet clé : La task_struct (la carte d'identité de chaque processus).
Répertoires : kernel/, kernel/sched/.

le process management est divise en sous unites

### A. Le Créateur : Fork & Life Cycle (fork.c, exit.c)
C'est le service "État Civil" et "RH".Rôle : Il gère la naissance 
(fork, vfork, clone) et la mort (exit) des processus.Mécanisme : C'est ici que la task_struct est allouée et initialisée. 
Pour Dirty COW, c'est crucial car c'est ici que l'on définit si la mémoire est partagée ou copiée (Copy-On-Write) lors d'un clone.Fonctions clés : do_fork(), copy_process(), do_exit().

### B. Le Chef d'Orchestre : The Scheduler (kernel/sched/)
C'est l'unité la plus complexe. Elle décide quel processus a le droit d'utiliser le CPU à un instant $T$.Rôle : Gérer l'équité entre les programmes, 
la priorité des tâches et le passage d'un processus à un autre (Context Switch).Algorithme : Linux utilise principalement le Completely Fair Scheduler (CFS).Fichiers clés : sched/core.c, sched/fair.c.

### C. Le Communicateur : Signals & IPC (signal.c, ipc/)
C'est le système nerveux qui permet d'envoyer des ordres aux processus.Rôle : Gérer les interruptions logicielles. Par exemple, quand tu fais un kill -9 ou quand un programme fait une erreur de segmentation (SIGSEGV).Mécanisme : Il vérifie si le processus a le droit d'envoyer un signal à un autre.Fonctions clés : do_send_sig_info(), get_signal().

### D. L'Administrateur : PIDs & Namespaces (pid.c, nsproxy.c)
C'est l'unité qui gère l'isolement et l'identification.Rôle : Garantir que chaque processus a un ID unique et gérer les "Namespaces" (la base des containers comme Docker).Mécanisme : Il permet à un processus de croire qu'il est seul sur le système (virtualisation des ressources).


### E. Les Ouvriers : Kthreads & Workqueues (kthread.c, workqueue.c)
Ce sont les processus internes du noyau.Rôle : Exécuter des tâches de fond qui n'ont pas besoin d'une interface utilisateur (comme l'écriture différée sur le disque ou la gestion de la température).Particularité : Ils n'ont pas d'espace utilisateur (mm_struct est souvent NULL).














## 2. Le Memory Management (mm/)
C'est le Gestionnaire d'Espace. Il gère la mémoire physique et virtuelle.
Fonctions : Allocation de RAM, pagination, protection de la mémoire, et le fameux Copy-On-Write (COW) que tu étudies.
Objet clé : mm_struct et les VMA (Virtual Memory Areas).
Répertoires : mm/.

Le Memory Management (MM) est sans doute l'unité la plus complexe du noyau, car elle doit faire le pont entre le processeur (le matériel) et les processus (le logiciel). Pour Dirty COW, c'est ici que tout se joue.

Voici les sous-unités logiques du répertoire mm/ :

### A. La Gestion de l'Espace d'Adressage Virtuel (VMA)
C’est l'unité qui définit "ce que le processus croit posséder". Elle gère les segments de mémoire (code, pile, tas, fichiers mappés).
Rôle : Créer, fusionner ou diviser des zones de mémoire virtuelle (struct vm_area_struct).
Fichier clé : mm/mmap.c.
Lien Dirty COW : C'est ici que mmap() définit les droits d'accès initiaux d'une zone.


### B. Le Gestionnaire de Fautes de Pages (Page Fault Handler)
C'est le "service d'urgence" du noyau. Lorsqu'un processus veut accéder à une adresse qui n'est pas encore en RAM, le processeur lève une exception.
Rôle : Déterminer si l'accès est légitime, charger la donnée depuis le disque ou créer une nouvelle page.
Fichier clé : mm/memory.c (fonction handle_mm_fault).
Lien Dirty COW : C'est cette unité qui déclenche le mécanisme de Copy-on-Write quand tu essaies d'écrire dans une page partagée.


### C. L'Allocateur de Pages Physiques (Buddy System)
C'est le gestionnaire des stocks réels de RAM.
Rôle : Allouer et libérer des blocs de pages physiques (souvent par puissances de 2) pour éviter la fragmentation.
Fichier clé : mm/page_alloc.c.


### D. L'Allocateur de Petits Objets (Slab/Slub Allocator)
Le noyau a souvent besoin de petites structures (comme un dentry ou un inode). Demander une page entière (4 Ko) serait du gâchis.
Rôle : Découper les pages en "caches" d'objets de taille fixe pour une performance maximale.
Fichier clé : mm/slub.c.


### E. Le Swap et la Réclamation (Page Reclaim)
C'est le service de nettoyage.
Rôle : Quand la RAM est pleine, cette unité décide quelles pages envoyer sur le disque (Swap) pour libérer de la place. Elle utilise l'algorithme LRU (Least Recently Used).
Fichier clé : mm/vmscan.c.





## 3. Le Virtual File System / VFS (fs/)
C'est l'Interface Universelle. C'est ici que tu travailles actuellement.
Fonctions : Faire en sorte que "tout soit un fichier" (disque, socket, procfs). Il traduit tes appels open, read, write pour les différents systèmes de fichiers (ext4, nfs, proc).
Objet clé : struct file, dentry, inode.
Répertoires : fs/.

Voici comment cette unité est découpée à l'intérieur du répertoire fs/ :

### A. La Couche d'Interface Système (System Call Interface)
C'est la porte d'entrée. Elle fait le pont entre l'espace utilisateur et le noyau.
Rôle : Recevoir les appels comme open(), read(), write() et les traduire en requêtes compréhensibles par le VFS.
Fichiers clés : fs/open.c, fs/read_write.c.
Lien Dirty COW : C'est ici que ton aventure a commencé avec l'appel open("/proc/self/pagemap").


### B. Le Cache des Entrées de Répertoire (dcache)
C'est le "GPS" du noyau. Comme nous l'avons vu, il évite de reparcourir tout le disque à chaque fois.
Rôle : Stocker en RAM la hiérarchie des dossiers et des noms de fichiers sous forme de dentry.
Fichier clé : fs/dcache.c.
Lien Dirty COW : Il permet de trouver instantanément l'objet pagemap dans l'arborescence de /proc.


### C. Le Cache d'Inodes (icache)
C'est l'unité de stockage des métadonnées (propriétaire, taille, permissions).
Rôle : Garder en mémoire les structures inode pour que le noyau n'ait pas à les relire sur le disque.
Fichier clé : fs/inode.c.


### D. Le Gestionnaire de Noms (Path Lookup)
C'est l'unité que tu es en train de décortiquer avec link_path_walk.
Rôle : Transformer une chaîne de caractères (un chemin comme /etc/passwd) en une suite de dentries et d'inodes.
Fichier clé : fs/namei.c.


### E. Le Cache de Pages (Page Cache)
C'est l'unité de performance pour les données elles-mêmes.
Rôle : Stocker le contenu des fichiers en RAM. Quand tu lis un fichier, le VFS regarde d'abord ici avant d'aller sur le disque.
Fichier clé : mm/filemap.c (le VFS délègue beaucoup à la gestion mémoire ici).


### F. La Gestion des Systèmes de Fichiers (File System Types)
C'est l'unité de multiplexage.
Rôle : Gérer l'enregistrement des différents formats (EXT4, BTRFS, PROCFS). Elle permet au VFS de savoir à quel pilote envoyer la commande finale.
Fichier clé : fs/filesystems.c.




## 4. Le Network Stack (net/)
C'est le Département Communication.
Fonctions : Gestion des protocoles (TCP/IP, UDP, Bluetooth), des sockets et du routage des paquets.
Répertoires : net/.

Voici comment cette unité logique est découpée dans le répertoire net/ :

### A. La Couche Sockets (BSD Sockets)
C'est l'interface entre tes programmes et le réseau.
Rôle : Fournir une API standard (socket(), bind(), connect(), send()). C'est elle qui donne l'illusion que le réseau est un "fichier" que l'on peut ouvrir et lire (interface avec le VFS).
Fichiers clés : net/socket.c.
Analogie : Le comptoir de la poste où tu déposes ton colis.


### B. Les Protocoles de Transport (TCP/UDP/ICMP)
C'est le cœur de la fiabilité.
Rôle : Gérer la découpe des données en segments, s'assurer qu'ils arrivent dans le bon ordre (TCP) ou les envoyer le plus vite possible sans vérification (UDP).
Fichiers clés : net/ipv4/tcp.c, net/ipv4/udp.c.
Analogie : Le service qui décide si ton colis doit être envoyé en recommandé avec accusé de réception ou en courrier normal.


### C. La Couche Réseau (IPv4 / IPv6)
C'est l'unité de routage.
Rôle : Ajouter les adresses IP de destination, gérer la fragmentation des paquets et décider par quel chemin (quelle carte réseau) le paquet doit partir.
Fichiers clés : net/ipv4/ip_input.c, net/ipv4/ip_output.c.
Analogie : Le centre de tri qui lit l'adresse sur l'enveloppe et décide du camion à charger.


### D. Le Core Networking & Sk_Buffs
C'est la plomberie interne du noyau.
Rôle : Gérer la structure sk_buff (Socket Buffer). C'est l'objet le plus important de net/. Un paquet réseau ne change jamais de place en mémoire ; on se passe simplement son pointeur sk_buff d'une couche à l'autre pour gagner en performance.
Fichiers clés : net/core/skbuff.c, net/core/dev.c.
Analogie : Le tapis roulant ultra-rapide qui transporte les colis sans que personne n'ait besoin de les porter.


### E. Le Neighbor Layer (ARP / Neighbor Discovery)
Le lien entre le logiciel et le matériel.
Rôle : Faire le lien entre une adresse IP (logique) et une adresse MAC (physique). Sans lui, le paquet ne peut pas sortir sur le câble.
Fichiers clés : net/core/neighbour.c.


### F. Netfilter & Firewalls
C'est l'unité de sécurité interne.
Rôle : Inspecter chaque paquet qui entre ou sort pour décider de le laisser passer, de le modifier (NAT) ou de le bloquer (iptables/nftables).
Fichiers clés : net/netfilter/.



## 5. L'Architecture Support (arch/)
C'est l'Adaptateur Matériel.
Fonctions : Code spécifique au processeur (x86_64, ARM). C'est ici que se trouvent les instructions assembleur de bas niveau, comme le entry_64.S qui t'a posé problème au début.
Répertoires : arch/x86/, arch/arm/.

Voici comment cette unité est découpée, particulièrement pour l'architecture x86 que tu utilises dans ton lab :

### A. Le Point d'Entrée et les Interruptions (Entry & IRQ)
C'est la porte blindée entre le monde extérieur (ton clavier, ton horloge) et le processeur.
Rôle : Gérer le passage du mode utilisateur au mode noyau (syscalls) et répondre aux signaux matériels (interruptions).
Fichiers clés : arch/x86/entry/entry_64.S.
Lien Dirty COW : C'est ici que le processeur "saute" lorsqu'une erreur de page (Page Fault) survient.

### B. La Gestion de la Mémoire Matérielle (MM)
C'est le bras armé de l'unité mm/ dont nous avons parlé.
Rôle : Manipuler les tables de pages réelles du processeur (PML4, PGD, PTE) et gérer le TLB (Translation Lookaside Buffer).
Répertoire : arch/x86/mm/.
Concept clé : La traduction des adresses virtuelles en adresses physiques.

### C. L'Initialisation et le Boot (Kernel Start)
C'est le protocole d'allumage de la machine.
Rôle : Configurer les registres du processeur, détecter la RAM disponible et lancer les autres processeurs (SMP).
Fichiers clés : arch/x86/kernel/head_64.S.

### D. Les Primitives de Synchronisation (Atomics & Locks)
C'est la fondation de la sécurité dans un monde multi-cœurs.
Rôle : Fournir des instructions "atomiques" (qui ne peuvent pas être interrompues) comme CMPXCHG.
Fichiers clés : arch/x86/include/asm/cmpxchg.h.
Lien Dirty COW : La faille repose justement sur une mauvaise synchronisation entre deux processeurs qui tentent de modifier la même page.

### E. Le Support des Périphériques Bas Niveau (Bus & PCI)
C'est l'unité qui sait comment parler aux bus de données de la carte mère.
Rôle : Énumérer les périphériques PCI et configurer les accès mémoire directe (DMA).



## 6. Les Device Drivers (drivers/)
C'est le Département des Périphériques.
Fonctions : Piloter le matériel réel (carte graphique, clavier, disque dur). C'est la partie la plus volumineuse du code source (~60%).
Répertoires : drivers/.

## 7. Inter-Process Communication / IPC (ipc/)
C'est la Messagerie Interne.
Fonctions : Permet à deux processus de se parler (mémoire partagée, files de messages, sémaphores).
Répertoires : ipc/.