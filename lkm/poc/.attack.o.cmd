cmd_/usr/src/lkm/poc/attack.o := gcc -Wp,-MMD,/usr/src/lkm/poc/.attack.o.d -nostdinc -isystem /usr/lib/gcc/x86_64-linux-gnu/10/include -I./arch/x86/include -I./arch/x86/include/generated  -I./include -I./arch/x86/include/uapi -I./arch/x86/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/compiler-version.h -include ./include/linux/kconfig.h -D__KERNEL__ -fmacro-prefix-map=./= -D__ASSEMBLY__ -fno-PIE -m64 -Wa,-gdwarf-2 -gdwarf-4  -DMODULE  -c -o /usr/src/lkm/poc/attack.o /usr/src/lkm/poc/attack.S

source_/usr/src/lkm/poc/attack.o := /usr/src/lkm/poc/attack.S

deps_/usr/src/lkm/poc/attack.o := \
  include/linux/compiler-version.h \
    $(wildcard include/config/CC_VERSION_TEXT) \
  include/linux/kconfig.h \
    $(wildcard include/config/CPU_BIG_ENDIAN) \
    $(wildcard include/config/BOOGER) \
    $(wildcard include/config/FOO) \

/usr/src/lkm/poc/attack.o: $(deps_/usr/src/lkm/poc/attack.o)

$(deps_/usr/src/lkm/poc/attack.o):
