.intel_syntax noprefix
.global brute_force_pte_write
.global getPTE5
.global getPTE4
.code64
.text


# Le compilateur GCC va respecter cet orqdre strict pour placer tes paramètres :
# RDI : 1er argument
# RSI : 2ème argument
# RDX : 3ème argument
# RCX : 4ème argument
# R8 : 5ème argument
# R9 : 6ème argument







#  brute_forqce_pte_write(pte_addr, target_virt_addr, new_value);
brute_force_pte_write:
    # RDI contient l'adresse de la PTE (premier argument)
    # RSI : contient l'adresse virtuelle
    # RDX : contient la nouvelle valeur (deuxième argument)
    
    push rax
    push rcx

        
    cli             # Désactive les interruptions (Clear Interrupts)

    movq rcx, 0xfffeffff

    # 1. Lire CR0 et désactiver WP (bit 16)
    movq rax, cr0
    andq rax, rcx
    movq cr0, rax
    
    # 2. Appliquer la modification
    movq [rdi], rdx


    # il faut invalider le TLB 
    # il y a principalement 2 methodes 
    invlpg [rsi] # ceci est obligatoire pour forqcer le processeur a utiliser la nouvelle page 
    
# Invalidation globale du TLB
     # movq rax, cr3
     # movq cr3, rax



    # 3. Réactiver WP
    orq rax, 0x10000
    movq cr0, rax

    sti             # Réactive les interruptions (Set Interrupts)
    
    pop rcx
    pop rax
    ret








# example 
# 0x7ffe1c9c9000

# The last 12 bits of this address represent the offset within the physical page:
# 0x7ffe1c9c9000 & 0xfff = 0x0


# Level 1, Page Table (PT):
# Index = (0x7ffe1c9c9000 >> 12) & 0x1ff = 0x1c9
# Level 2, Page Middle Directorqy (PMD):
# Index = (0x7ffe1c9c9000 >> 21) & 0x1ff = 0x0e4
# Level 3, Page Upper Directorqy (PUD):
# Index = (0x7ffe1c9c9000 >> 30) & 0x1ff = 0x1f8
# Level 4, Page Global Directorqy (PGD):
# Index = (0x7ffe1c9c9000 >> 39) & 0x1ff = 0x0ff



# avant 2017, linux utilisait un espace d'adressage a 4 niveaux pour la memoire virtuelle PGD,PUD,PMD,PTE 
# getPTE4: Calcule l'adresse virtuelle de l'entrée PTE pour une hiérarchie 4 niveaux
# Entrée: RDI = phys_base (offset noyau pour mapping direct)
# Entrée: RSI = adresse virtuelle cible
# Sortie: RAX = adresse virtuelle de la PTE
getPTE4:
    push rbx
    push rcx 
    movq rcx, 0x000ffffffffff000 # Masque adresse physique (bits 12-51)
    
    # 1. PGD
    movq rax, cr3                
    andq rax, rcx                
    add rax, rdi                 # Conversion en Virtuelle
    movq rbx, rsi
    shrq rbx, 39                 
    andq rbx, 0x1ff
    movq rax, [rax+8*rbx]        # Lecture entrée PGD
    andq rax, rcx                # Masquage: rax contient maintenant phys addr PUD
    add rax, rdi                 # Conversion en Virtuelle

    # 2. PUD
    movq rbx, rsi
    shrq rbx, 30                 
    andq rbx, 0x1ff
    movq rax, [rax+8*rbx]        # Lecture entrée PUD
    andq rax, rcx                # Masquage: rax contient maintenant phys addr PMD
    add rax, rdi                 # Conversion en Virtuelle

    # 3. PMD
    movq rbx, rsi
    shrq rbx, 21                 
    andq rbx, 0x1ff
    movq rax, [rax+8*rbx]        # Lecture entrée PMD
    andq rax, rcx                # Masquage: rax contient maintenant phys addr PTE
    add rax, rdi                 # Conversion en Virtuelle

    # 4. PTE
    movq rbx, rsi
    shrq rbx, 12                 
    andq rbx, 0x1ff
    # ATTENTION: Ici, rax est déjà l'adresse virtuelle de la TABLE de pages PTE
    # On ajoute simplement l'index pour pointer vers l'entrée spécifique
    shlq rbx, 3                  # rbx = index * 8
    add rax, rbx                 # rax = adresse virtuelle de la PTE finale

    pop rcx 
    pop rbx 
    ret






# a partir de 2017, linux a introduit une meoire virtuelle a 5 niveaux, PGD,P4D,PUD,PMD,PTE
# extern unsigned long getPTE(unsigned long physicalbase, unsigned long virt_addr);
# RDI : physicalbase (l'adresse de début du physmap, __va(0))
# RSI : virt_addr (l'adresse virtuelle cible à inspecter)
# Retourne dans RAX : l'adresse virtuelle de l'entrée PTE dans la table

getPTE5:
    push rbx
    push rcx 
    movq rcx, 0x000ffffffffff000 # Masque pour isoler l'adresse physique (bits 12-51)
    
    # 1. PGD (Index: bits 48-39)
    movq rax, cr3                # rax = Adresse physique de la PGD (Phys)
    andq rax, rcx                # Masquage (Phys)
    add rax, rdi                 # rax = Adresse virtuelle de la table (Virt)
    movq rbx, rsi                # rsi contient l'adresse virtuelle cible
    shrq rbx, 39                 # Décalage pour index PGD
    andq rbx, 0x1ff
    movq rax, [rax+8*rbx]        # rax = Contenu de l'entrée PGD (Phys)
    andq rax, rcx                # Isoler adresse physique P4D (Phys)
    add rax, rdi                 # Conversion en adresse virtuelle (Virt)

    # 2. P4D (Index: bits 39-30)
    movq rbx, rsi
    shrq rbx, 30                 # Décalage pour index P4D
    andq rbx, 0x1ff
    movq rax, [rax+8*rbx]        # rax = Contenu de l'entrée P4D (Phys)
    andq rax, rcx                # Isoler adresse physique PUD (Phys)
    add rax, rdi                 # Conversion en adresse virtuelle (Virt)

    # 3. PUD (Index: bits 30-21)
    movq rbx, rsi
    shrq rbx, 21                 # Décalage pour index PUD
    andq rbx, 0x1ff
    movq rax, [rax+8*rbx]        # rax = Contenu de l'entrée PUD (Phys)
    andq rax, rcx                # Isoler adresse physique PMD (Phys)
    add rax, rdi                 # Conversion en adresse virtuelle (Virt)

    # 4. PMD (Index: bits 21-12)
    movq rbx, rsi
    shrq rbx, 12                 # Décalage pour index PMD
    andq rbx, 0x1ff
    movq rax, [rax+8*rbx]        # rax = Contenu de l'entrée PMD (Phys)
    andq rax, rcx                # Isoler adresse physique PTE (Phys)
    add rax, rdi                 # Conversion en adresse virtuelle (Virt)

    # 5. PTE (Index: bits 12-20)
    # L'adresse dans rax est déjà l'adresse de la table PTE, on ajoute l'index
    movq rbx, rsi
    andq rbx, 0x1ff000           # Isoler les bits 12-20 pour l'index PTE
    shrq rbx, 9                  # Ajuster pour obtenir l'index * 8 (taille entrée)
    
    add rax, rbx                 # rax = Adresse virtuelle finale de la PTE (Virt)

    pop rcx 
    pop rbx 
    ret
