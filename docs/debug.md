echo "1 modify 0x401000" > /dev/cf2


### dans qemu
lsmod

cat /sys/module/my_attack/sections/.text
cat /sys/module/my_attack/sections/.data 
cat /sys/module/my_attack/sections/.bss

### dans vscode



### pour charger les symboles lies au LKM2

-exec add-symbol-file /home/daniel/Desktop/KERNEL/LKM2/my_attack.ko 0xffffffffc0000000 -s .data 0xffffffffc0005040 -s .bss 0xffffffffc00054c0
-exec set substitute-path /usr/src/LKM2 /home/daniel/Desktop/KERNEL/LKM2

### pour que le debugger puisse faire le lien entre les fichiers et les symboles
### tres tres important
-exec set substitute-path /usr/src/LKM2 /home/daniel/Desktop/KERNEL/LKM2

-exec info functions modify_page_permissions


### pour lister toutes les fonctions connues grace aux symboles de deboggage (attention tres gros et tres long)
-exec info functions


### les commandes ci dessous ne fonctionnent pas
-exec info functions -f cf_attack.c
-exec info functions attack.S::.*




### pour poser un breakpoint hard

### dans qemu 
grep device_write /proc/kallsyms

### resultat
ffffffff818e1af0 T nvmem_device_write
ffffffff8267a040 r __ksymtab_nvmem_device_write
ffffffff82693e74 r __kstrtabns_nvmem_device_write
ffffffff826a9899 r __kstrtab_nvmem_device_write
ffffffff829dec20 d dev_attr_mdio_bus_device_writes
ffffffffc0001060 t device_write	[my_attack]


### dans vscode
-exec hbreak *0xffffffffc0001060

### dans qemu
~ # echo "1 inspect" >/dev/cf2





### pour poser un breakpoint soft
-exec break cf_attack.c:498



### pour voir le code d'une fonction ou d'un fichier 
-exec list device_write
-exec list cf_attack.c:1,500


### dans le shell pour obtenir la liste des fonctions d'un LKM
nm ./LKM2/my_attack.ko | grep " T "


# pour voir la valeur rax et pour pouvoir la copier
-exec p/x $rax
-exec p/x $rdi

### pour voir une zone memoire (PGD,PUD,)

-exec monitor xp/512gx  0x4786000
-exec monitor xp/512gx  0x4797000
-exec monitor xp/512gx  0x479b000
-exec monitor xp/512gx  0x4795000


> monitor : Indique à GDB de ne pas interpréter cette commande lui-même, mais de la transmettre directement au serveur QEMU qui exécute l'émulation.
> xp : C'est la version "physique" de la commande x (examine). Elle demande au moniteur QEMU d'inspecter la mémoire en utilisant des adresses physiques (et non virtuelles).
> /512 : Le nombre d'unités à afficher.
> g : La taille de chaque unité (g pour giant), soit 8 octets (64 bits). C'est idéal pour lire des entrées de tables de pages ou des pointeurs 64 bits.
> x : Le format d'affichage (hexadécimal).
> 0x10d664000 : L'adresse physique de départ dans la mémoire système.