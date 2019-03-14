#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/kdev_t.h>

#define LICENCE "GPL"
#define AUTEUR "Emmanuel CAUSSE emmanuel.causse@univ-tlse3.fr"
#define DESCRIPTION "Module Ecriture/lecture destructrice"
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
} BUFFER;

struct cdev *my_cdev;
BUFFER my_data;

static int buffer_init(void){
	
	int err;

	// Allocationn dynamique pour les paires majeur/mineur
	if(alloc_chrdev_region(&dev,0,1,"module_rw") == -1){
		printk(KERN_ALERT "Erreur alloc_chrdev_region\n");
		return -EINVAL;
	}

	printk(KERN_ALERT "Init alloc (majeur,mineur)=(%d,%d)\n",MAJOR(dev),MINOR(dev));

	my_data.bufSize = 0;
	
	//Enregistrement du peripherique
	my_cdev = cdev_alloc();
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	err=cdev_add(my_cdev,dev,1);

	if(err){printk(KERN_ALERT "Erreur cdev_add \n");}

	return 0;
}

/*Verifie l'etat du peripherique
Initialise le peripherique (premiere ouverture)
Identifier le mineur (eventuellement mettre a jour f_op)
*/
static int buffer_open(struct inode *in, struct file *f){
    printk(KERN_ALERT "OPEN\n");
    return 0;
}


static ssize_t buffer_read(struct file *f, char *buff, size_t size, loff_t *offp){

	int sizeToCopy = MIN(my_data.bufSize,BUFFER_SIZE);

	printk(KERN_ALERT "Read called!\n");
    
	if(my_data.bufSize != 0){
		if(copy_to_user(buff, my_data.bufData, sizeToCopy)==0){
			my_data.bufSize = 0;
			kfree(my_data.bufData);
		}
		else{return -EFAULT;}
	}
	
	return sizeToCopy;
}

static ssize_t buffer_write(struct file *f, const char * buff, size_t size, loff_t *offp){
	printk(KERN_ALERT "Write called\n");
	if(my_data.bufSize != 0){
		kfree(my_data.bufData);
        my_data.bufSize = 0;
	}

	size = MIN(size,BUFFER_SIZE);

	my_data.bufData = (char *)kmalloc(BUFFER_SIZE * sizeof(char), GFP_KERNEL);
	my_data.bufSize = size - copy_from_user(my_data.bufData,buff,size);
    printk(KERN_ALERT "BUFFER = %s ",my_data.bufData);
	return my_data.bufSize;
	
}

static int buffer_release(struct inode *in, struct file *f){
    
    printk(KERN_ALERT "RELEASE\n");
    return 0;
}

static void buffer_cleanup(void){
	/* liberation */
	unregister_chrdev_region(dev,1);
	cdev_del(my_cdev);	
	printk(KERN_ALERT "Goodbye \n");
}

module_init(buffer_init);
module_exit(buffer_cleanup);

MODULE_LICENSE(LICENCE);
MODULE_AUTHOR(AUTEUR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_SUPPORTED_DEVICE(DEVICE);

