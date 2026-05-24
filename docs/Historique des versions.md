#KERNEL

Version	Date de sortie	Importance pour ton Labo
v2.6.0	Décembre 2003	Introduction du modèle de threads moderne (NPTL).
v3.0	Juillet 2011	Changement de numérotation (20ème anniversaire).
v3.10	Juin 2013	    Version LTS massivement utilisée dans les vieux serveurs.
v4.0	Avril 2015	    Introduction du "Live Patching" (modifier le kernel sans reboot).
v4.7	Juillet 2016	Ta cible actuelle : Contient nativement Dirty COW.
v4.8	Octobre 2016	Dernière version vulnérable a Dirty COW 
v4.9    Décembre 2016   Première version intégrant le correctif pour Dirty COW
v5.0	Mars 2019	    Amélioration majeure du support GPU et chiffrement Adiantum.
v5.10	Décembre 2020	Version LTS (Long Term Support) très stable pour le SaaS. 
v6.0	Octobre 2022	Support amélioré pour les architectures processeurs récentes.
v6.6	Octobre 2023	Version LTS actuelle, base de sécurité moderne.



### 1. La chronologie des faits
Sortie du Kernel 4.8 : 2 octobre 2016.
Découverte de Dirty COW : Mi-octobre 2016 (révélée publiquement le 19 octobre).
Correctif (Patch) : Linus Torvalds a intégré le patch dans la branche principale le 20 octobre 2016.
Conclusion : La version "pure" 4.8 (celle que tu as obtenue via git archive v4.8) a été publiée juste avant que la faille ne soit rendue publique. 
Elle contient donc encore le code vulnérable du mécanisme de Copy-On-Write.
### 2. Comment obtenir la version "sécurisée" ?
Pour voir l'exploit échouer, tu as deux options pour ton prochain build Docker :
Utiliser une version mineure corrigée : Par exemple, la 4.8.3 ou supérieure. C'est dans ces versions "stable" que le patch a été rétro-porté.
Appliquer le patch manuellement : C'est un excellent exercice pour ton futur Tech Lead. Tu télécharges le fichier .patch de la CVE-2016-5195 
et tu l'appliques sur tes sources 4.8 avant de compiler.


## UBUNTU

Voici la correspondance entre les versions majeures du noyau Linux et les versions d'Ubuntu (principalement les versions LTS, 
car ce sont les plus stables pour le développement SaaS) :
Correspondance Kernel / Ubuntu (LTS)

Version Kernel      Version Ubuntu (Nom de code)    Date de sortie    Version GCC par défaut
v2.6.x6.06          LTS (Dapper Drake)              Juin 2006           GCC 4.0
v3.212.04           LTS (Precise Pangolin)          Avril 2012          GCC 4.6
v3.1314.04          LTS (Trusty Tahr)               Avril 2014          GCC 4.8
v4.416.04           LTS (Xenial Xerus)              Avril 2016          GCC 5.4
v4.1518.04          LTS (Bionic Beaver)             Avril 2018          GCC 7.3
v5.420.04           LTS (Focal Fossa)               Avril 2020          GCC 9.3
v5.1522.04          LTS (Jammy Jellyfish)           Avril 2022          GCC 11.2
v6.824.04           LTS (Noble Numbat)              Avril 2024          GCC 13.2


Cas particulier : le Kernel 4.7
Le noyau v4.7 (celui que tu compiles actuellement pour Dirty COW) est sorti entre Ubuntu 16.04 (v4.4) et Ubuntu 16.10 (v4.8). [cite: 2026-01-23]

Pourquoi Ubuntu 16.04 est ton meilleur choix : C'est la version LTS qui utilisait le GCC 5. Comme le noyau 4.7 a été écrit alors que GCC 5 
était le standard, ils parlent le "même langage". [cite: 2026-01-24]

Le saut technologique : À partir d'Ubuntu 18.04, GCC est passé à la version 7+, introduisant des protections de sécurité et des optimisations 
strictes qui font "paniquer" les vieux noyaux non préparés. [cite: 2026-01-24]


dirtyCOW exploite une execution desynchronisee entre des instructions de differents threads dont l'un ecrit dans la memoire d'un ifchier protege en 
ecriture ce qui oblige le noyau a faire une copie alors que l'autre efface la copie. Normalement le noyau devrait toujours refair eune copie, mais parce 
qu'il y avait un bug, dans certaines conditions l'ecriture se fait sur le fichier original et pas sur la copie. 
C'est un bug qui se produit parce que le noyau switche entre differents threads et ce type de bug peut se produire sur tout systeme 
parallele et plus le systeme est parallelise et plus le bug a de chnaces d'exister.  


Il existe des attaques récentes appelées "GPU.zip" ou des failles de fuite de mémoire sur les drivers NVIDIA/AMD qui reposent 
exactement sur ce que tu as décrit : un thread accède à une zone mémoire qu'un autre thread n'a pas encore fini de "nettoyer", 
permettant ainsi de lire des données résiduelles d'autres applications (comme tes mots de passe ou tes clés API de trading).