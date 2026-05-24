#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <capstone/capstone.h>

#define BUF_SIZE 4096

int main() {
    csh handle;
    cs_insn *insn;
    size_t count;
    unsigned char buffer[BUF_SIZE];
    size_t bytes_read = fread(buffer, 1, BUF_SIZE, stdin);

    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) return -1;

    count = cs_disasm(handle, buffer, bytes_read, 0x1000, 0, &insn);
    if (count > 0) {
        for (size_t i = 0; i < count; i++) {
            // Construire la chaîne hexadécimale des opcodes
            char hex_bytes[32] = {0};
            char tmp[4];
            for (size_t j = 0; j < insn[i].size; j++) {
                snprintf(tmp, sizeof(tmp), "%02x ", insn[i].bytes[j]);
                if (strlen(hex_bytes) + strlen(tmp) < sizeof(hex_bytes)) {
                    strcat(hex_bytes, tmp);
                }
            }

            // Affichage aligné : Adresse | Opcodes Hexa | Instructions
            printf("0x%"PRIx64":\t%-24s\t%s\t\t%s\n", 
                   insn[i].address, 
                   hex_bytes, 
                   insn[i].mnemonic, 
                   insn[i].op_str);
        }
        cs_free(insn, count);
    } else {
        fprintf(stderr, "Erreur de lecture ou flux binaire invalide.\n");
    }
    cs_close(&handle);
    return 0;
}