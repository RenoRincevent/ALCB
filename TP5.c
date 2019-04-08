#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/list.h>

#define LICENCE "GPL"
#define AUTEUR "Emmanuel CAUSSE emmanuel.causse@univ-tlse3.fr"
#define DESCRIPTION "Module Ecriture/lecture 2 periph,buffer variable"
#define DEVICE "Siame10"
#define BUFFER_SIZE 4
#define MIN( p1, p2 ) ( p1 <= p2 ) ? p1 : p2

//Numero du peripherique
//32 bits avec 12 bits réservés pour le nombre majeur et 20 pour le nombre mineur
dev_t dev;

static ssize_t buffer_read(struct file *f, char *buff, size_t size, loff_t *offp);
static ssize_t buffer_write(struct file *f, const char * buff, size_t size, loff_t *offp);
static int buffer_open(struct inode *in, struct file *f);
static int buffer_release(struct inode *in, struct file *f);

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.read = &buffer_read,
	.write = &buffer_write,
	.open = &buffer_open,
	.release = &buffer_release
};

typedef struct {
	size_t bufSize;
	char* bufData;	
	struct list_head liste;
} BUFFER;

struct cdev *my_cdev;
struct list_head contenu;
BUFFER my_data;

int flagRead = 0;
int flagWrite = 0;
int dataSize = 0;


//DONE
void lfree (void) {
	BUFFER *element;
	struct list_head *ptr;
    struct list_head *next;

	list_for_each_safe(ptr,next,&contenu){
		 element = list_entry(ptr, BUFFER, liste);
		 list_del(ptr);
		 kfree(element->bufData);
		 kfree(element);
		 
	}
}

//DONE
//called when insmod TP.ko
static int buffer_init(void){
	
	int err;

	// Allocationn dynamique pour les paires majeur/mineur
	if(alloc_chrdev_region(&dev,0,2,"sample") == -1){
		printk(KERN_ALERT "Erreur alloc_chrdev_region\n");
		return -EINVAL;
	}

	printk(KERN_ALERT "Init alloc (majeur,mineur)=(%d,%d)\n",MAJOR(dev),MINOR(dev));
	
	//Enregistrement du peripherique
	my_cdev = cdev_alloc();
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	
	err=cdev_add(my_cdev,dev,2);
	if(err){printk(KERN_ALERT "Erreur cdev_add \n");}

	INIT_LIST_HEAD(&contenu);
	
	return 0;
}

/*Verifie l'etat du peripherique
Initialise le peripherique (premiere ouverture)
Identifier le mineur (eventuellement mettre a jour f_op)
*/
//DONE
static int buffer_open(struct inode *in, struct file *f){
    printk(KERN_ALERT "OPEN\n");
    my_fops.read = NULL;
    my_fops.write = NULL;
    if(iminor(in) == 0){//cas ou on read
        my_fops.read = &buffer_read;
    }
    else if(iminor(in) == 1){//cas du write
        my_fops.write = &buffer_write;
    }
    return 0;
}


static ssize_t buffer_read(struct file *f, char *buff, size_t size, loff_t *offp){

	int sizeToCopy = 0;
	int effectiveSize = 0;
	int flagCourant = 0;
    BUFFER *element;

	printk(KERN_ALERT "Read called!\n");
	
	//printk(KERN_ALERT "BUFFER Taille= %d",dataSize);
    
	if(dataSize != 0) {
            //tester first entry
			element = list_first_entry(&contenu, BUFFER, liste);
			sizeToCopy = MIN (element->bufSize, size);

			if((element->bufSize != 0) && (!flagCourant)) {
				flagCourant = 1;
				if(copy_to_user(buff, element->bufData, sizeToCopy) == 0) {
					element->bufSize = 0;
					dataSize = dataSize - sizeToCopy;
					effectiveSize = effectiveSize + sizeToCopy;
					
				}
				else {
					printk(KERN_ALERT "ERROR : copy_to_user.\n");
					return -EFAULT;
					
				}
			}
			
			if(element->bufSize == 0){
                list_del(&(element->liste));
                kfree(element->bufData);
                kfree(element);
                
            }
	}

	flagRead = 1;
	
	return sizeToCopy;
}

//DONE
static ssize_t buffer_write(struct file *f, const char * buff, size_t size, loff_t *offp){
    BUFFER *nouvelElement;
    
    printk(KERN_ALERT "Write called\n");

	//Un écrit et l'appel prec etait écriture : on écrase
	//if(flagWrite) {
	//	lfree();
	//	flagWrite = 0;
      //  dataSize = 0;
	//}


	//if(my_data.bufSize != 0){
		//kfree(my_data.bufData);
        	//my_data.bufSize = 0;
	//}

	size = MIN(size,BUFFER_SIZE);

	nouvelElement = (BUFFER*) kmalloc(sizeof(BUFFER), GFP_KERNEL);
	nouvelElement->bufData = (char *)kmalloc(size * sizeof(char), GFP_KERNEL);
	nouvelElement->bufSize = size - copy_from_user(nouvelElement->bufData,buff,size);
	
	//Ajout du nouveau noeud a la liste
	INIT_LIST_HEAD(&nouvelElement->liste);
	list_add_tail(&nouvelElement->liste, &contenu);

	dataSize = dataSize + nouvelElement->bufSize;
    //printk(KERN_ALERT "BUFFER taille = %d ",dataSize);
    //printk(KERN_ALERT "BUFFER = %s",nouvelElement->bufData);
	return nouvelElement->bufSize;
	
}

//DONE
static int buffer_release(struct inode *in, struct file *f){
    
   	printk(KERN_ALERT "RELEASE\n");

	//Cas ou c'était une lecture, on écrase le contenu après lecture
	if (flagRead) {
		lfree();
		flagRead = 0;
		flagWrite = 0;
		
	}
	//Cas ou c'était une ecriture
	else{
		flagWrite = 1;
	}

	return 0;
}


//DONE
//called when rmmod TP
static void buffer_cleanup(void){
	/* liberation */
	lfree();
	unregister_chrdev_region(dev,2);
	cdev_del(my_cdev);	
	printk(KERN_ALERT "Goodbye \n");
}

module_init(buffer_init);
module_exit(buffer_cleanup);

MODULE_LICENSE(LICENCE);
MODULE_AUTHOR(AUTEUR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_SUPPORTED_DEVICE(DEVICE);

