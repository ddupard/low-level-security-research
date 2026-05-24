cmd_/usr/src/lkm/poc/modules.order := {   echo /usr/src/lkm/poc/my_attack.ko; :; } | awk '!x[$$0]++' - > /usr/src/lkm/poc/modules.order
