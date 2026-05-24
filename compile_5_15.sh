#!/bin/bash

# Détection du mode : si on passe "debug" en argument, on active le mode debug
MODE_DEBUG=false
if [ "$1" == "debug" ]; then
    MODE_DEBUG=true
    echo "!!! DEBUG MODE : Symbols activated, remove of kernel protections !!!"
fi

sudo docker run --rm \
    -v $(pwd)/5.15:/usr/src/linux \
    -v $(pwd)/lkm/poc:/usr/src/lkm/poc \
    kernel-builder-515 /bin/bash -c "
    
    # --- ÉTAPE 1 : CONFIGURATION DU NOYAU ---
    cd /usr/src/linux
   
   
    if [ '$MODE_DEBUG' = 'true' ]; then
        #make mrproper
        make x86_64_defconfig
        ./scripts/config --file .config --enable CONFIG_DEBUG_INFO
        ./scripts/config --file .config --enable CONFIG_DEBUG_INFO_DWARF4 
        ./scripts/config --file .config --enable CONFIG_GDB_SCRIPTS
    else
        make mrproper
        make x86_64_defconfig

        # 1. Suppression de Kprobes
        ./scripts/config --file .config --disable CONFIG_KPROBES
        ./scripts/config --file .config --disable CONFIG_HAVE_KPROBES
        ./scripts/config --file .config --disable CONFIG_KPROBES_ON_FTRACE
        ./scripts/config --file .config --disable CONFIG_KRETPROBES

        echo 'Configuration du Noyau en mode PRODUCTION (Hardened)...'
        ./scripts/config --file .config --disable CONFIG_FTRACE
        ./scripts/config --file .config --disable CONFIG_FUNCTION_TRACER
        ./scripts/config --file .config --disable CONFIG_STACK_TRACER
        ./scripts/config --file .config --disable CONFIG_TRACEPOINTS
        ./scripts/config --file .config --disable CONFIG_KGDB
        ./scripts/config --file .config --disable CONFIG_DEBUG_FS

        # 3. IMMUABILITÉ MÉMOIRE
        ./scripts/config --file .config --enable CONFIG_STRICT_KERNEL_RWX
        ./scripts/config --file .config --enable CONFIG_STRICT_MODULE_RWX
        ./scripts/config --file .config --disable CONFIG_X86_PTDUMP

        # 4. DURCISSEMENT AVANCÉ
        ./scripts/config --file .config --enable CONFIG_X86_SMAP
        ./scripts/config --file .config --enable CONFIG_X86_SMEP
        ./scripts/config --file .config --enable CONFIG_RANDOMIZE_BASE
        ./scripts/config --file .config --enable CONFIG_RANDOMIZE_MEMORY
        ./scripts/config --file .config --enable CONFIG_CC_STACKPROTECTOR_STRONG
        ./scripts/config --file .config --enable CONFIG_SLAB_FREELIST_RANDOM
        ./scripts/config --file .config --enable CONFIG_SLAB_FREELIST_HARDENED
        ./scripts/config --file .config --enable CONFIG_INIT_STACK_ALL_ZERO
    fi

    


    make olddefconfig
    make -j\$(nproc)
    
    # Compile of LKMs
    cd /usr/src/lkm/poc && make clean && make
" 2>&1 | tee compilation_totale.log