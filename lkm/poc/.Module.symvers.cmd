cmd_/usr/src/lkm/poc/Module.symvers := sed 's/\.ko$$/\.o/' /usr/src/lkm/poc/modules.order | scripts/mod/modpost    -o /usr/src/lkm/poc/Module.symvers -e -i Module.symvers   -T -
