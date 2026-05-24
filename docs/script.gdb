# Connexion à QEMU
set architecture i386:x86-64
target remote localhost:1234

# Poser le breakpoint sur ta fonction
b modify_page_permissions

# Continuer l'exécution après le breakpoint initial
#continue

