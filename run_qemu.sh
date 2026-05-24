#!/bin/bash

# Ce script s'exécute sur ta machine physique (Ubuntu) dans ~/Desktop/KERNEL
cd "$HOME/Desktop/KERNEL"

echo "-------------------------------------------------------"
echo "  LABORATOIRE DE TEST - COGNITIVE FIREWALL"
echo "-------------------------------------------------------"
echo "Choisissez la version du noyau à tester :"
echo "1) Kernel 4.7  (Vulnérable Dirty COW)"
echo "2) Kernel 4.8  (Intro XDP)"
echo "3) Kernel 4.9  (Patché Dirty COW)"
echo "4) Kernel 5.15 (LTS)"
echo "q) Quitter"
echo "-------------------------------------------------------"

read -p "Votre choix [1-4] : " choice

case $choice in
    1)
        VERSION="4.7"
        KERNEL_PATH="./4.7/arch/x86/boot/bzImage"
        ;;
    2)
        VERSION="4.8"
        KERNEL_PATH="./4.8/arch/x86/boot/bzImage"
        ;;
    3)
        VERSION="4.9"
        KERNEL_PATH="./4.9/arch/x86/boot/bzImage"
        ;;        
   
        
    4)


        VERSION="5.15"
        KERNEL_PATH="./5.15/arch/x86/boot/bzImage"

        # --- INJECTION ET EXÉCUTION AUTOMATIQUE DU LKM ---
        LKM2_PATH="./lkm/poc/my_attack.ko"
        
        if [ -f "$LKM2_PATH" ]; then
            echo "building initramfs for Kernel 5.15..."
            
            mkdir -p .initramfs_root
            cd .initramfs_root
            zcat ../initramfs.cpio.gz | cpio -idmv &>/dev/null
            
            # 1. Copie du binaire à la racine
            cp "../$LKM2_PATH" .
            
            # 2. REÉCRITURE PROPRE DU SCRIPT INIT DISCRET
            # On monte les systèmes de fichiers essentiels, on injecte, et on donne la main
            cat << 'EOF' > init
#!/bin/sh
# Systèmes de fichiers minimum requis pour le noyau
mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t devtmpfs devtmpfs /dev 2>/dev/null

echo "======================================================="
echo "  COGNITIVE FIREWALL LABS - AUTOMATIC INJECTION"
echo "======================================================="


if [ -f /my_attack.ko ]; then
    insmod /my_attack.ko
else
    echo "[ERROR] my_attack.ko not found. "
fi
echo "======================================================="

# Lancement du shell utilisateur standard
exec /bin/sh
EOF

            chmod +x init
            
            # 3. Re-paquetage hermétique
            find . -print0 | cpio --null -ov --format=newc 2>/dev/null | gzip -9 > ../initramfs.cpio.gz
            
            cd ..
            rm -rf .initramfs_root
            echo "-> Initramfs ready."
        else
            echo "Error: LKM not found in $LKM2_PATH."
        fi

   
         qemu-system-x86_64 \
             -kernel "$KERNEL_PATH" \
             -initrd "$HOME/Desktop/KERNEL/initramfs.cpio.gz" \
             -nographic \
             -append "console=ttyS0 loglevel=7 nosmep nosmap nokaslr" \
             -s -S
            

  
        exit 0
        ;;
    q)
        echo "Sortie du labo."
        exit 0
        ;;
    *)
        echo "Choix invalide."
        exit 1
        ;;
esac

echo "Launch of Kernel $VERSION in QEMU..."

# Lancement propre de QEMU
# Note : J'ai retiré le "-s -S" par défaut. Remets-le si tu veux figer QEMU pour y brancher GDB.
qemu-system-x86_64 \
  -kernel "$KERNEL_PATH" \
  -initrd "$HOME/Desktop/KERNEL/initramfs.cpio.gz" \
  -nographic \
  -append "console=ttyS0 nokaslr loglevel=7"
























