//scp Makefile root@vm-dyn-0-200:/root <- copier un fichier

/*root# make
root# insmod hello.ko
Hello, World
root# rmmod hello
Goodbye cruel world
root#*/



#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#define LICENCE "GPL"
#define AUTEUR "Emmanuel CAUSSE emmanuel.causse@univ-tlse3.fr"
#define DESCRIPTION "Module Hello World"
#define DEVICE "Siame10"


char *NomUtilisateur="Manu";
module_param(NomUtilisateur, charp, S_IRUGO);

static int hello_init(void){
  printk(KERN_ALERT "Hello %s!\n", NomUtilisateur);
  return 0;
}

static void hello_cleanup(void){
  printk(KERN_ALERT "Goodbye %s\n", NomUtilisateur);
}

module_init(hello_init);
module_exit(hello_cleanup);

/* Types de licences supportées :                                                                */
/*      "GPL"                        GNU Public Licence V2 ou ultérieure                         */
/*      "GPL v2"                     GNU Public Licence v2                                       */
/*      "GPL and additional rights   GNU Public Licence v2 et droits complémentaires             */
/*      "Dual BSD/GPL"               Licence GPL ou BSD au choix                                 */
/*      "Dual MPL/GPL"               Licence GPL ou Mozilla au choix                             */
/*      "Propietary"                 Produit à diffusion non libre (commercial)                  */
MODULE_LICENSE(LICENCE);
/* Classiquement : Nom Email */ 
MODULE_AUTHOR(AUTEUR);
/* Ce que fait votre module */
MODULE_DESCRIPTION(DESCRIPTION);
/* Périphériques supportés */
MODULE_SUPPORTED_DEVICE(DEVICE);
