#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <asm/uaccess.h>



MODULE_LICENSE ("GPL");


int keyb_init(void);
void keyb_exit(void);
uint8_t keyboard_to_ascii(uint8_t key);
int queue_open(struct inode *inode, struct file *filp);
int queue_release(struct inode *inode, struct file *filp);
ssize_t queue_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
irq_handler_t irq_handler (int irq, void *dev_id, struct pt_regs *regs);


struct file_operations keyb_fops =
{
	.read = queue_read,

    .open = queue_open,

    .release = queue_release
};

int keyb_major = 61;
char *queue_buffer;
int front = 0;
int rear = 0;
int ct = 0;
int maxsize = 255;

enum KEYCODE {
	NULL_KEY = 0,
	Q_PRESSED = 0x10,
	Q_RELEASED = 0x90,
	W_PRESSED = 0x11,
	W_RELEASED = 0x91,
	E_PRESSED = 0x12,
	E_RELEASED = 0x92,
	R_PRESSED = 0x13,
	R_RELEASED = 0x93,
	T_PRESSED = 0x14,
	T_RELEASED = 0x94,
	Z_PRESSED = 0x15,
	Z_RELEASED = 0x95,
	U_PRESSED = 0x16,
	U_RELEASED = 0x96,
	I_PRESSED = 0x17,
	I_RELEASED = 0x97,
	O_PRESSED = 0x18,
	O_RELEASED = 0x98,
	P_PRESSED = 0x19,
	P_RELEASED = 0x99,
	A_PRESSED = 0x1E,
	A_RELEASED = 0x9E,
	S_PRESSED = 0x1F,
	S_RELEASED = 0x9F,
	D_PRESSED = 0x20,
	D_RELEASED = 0xA0,
	F_PRESSED = 0x21,
	F_RELEASED = 0xA1,
	G_PRESSED = 0x22,
	G_RELEASED = 0xA2,
	H_PRESSED = 0x23,
	H_RELEASED = 0xA3,
	J_PRESSED = 0x24,
	J_RELEASED = 0xA4,
	K_PRESSED = 0x25,
	K_RELEASED = 0xA5,
	L_PRESSED = 0x26,
	L_RELEASED = 0xA6,
	Y_PRESSED = 0x2C,
	Y_RELEASED = 0xAC,
	X_PRESSED = 0x2D,
	X_RELEASED = 0xAD,
	C_PRESSED = 0x2E,
	C_RELEASED = 0xAE,
	V_PRESSED = 0x2F,
	V_RELEASED = 0xAF,
	B_PRESSED = 0x30,
	B_RELEASED = 0xB0,
	N_PRESSED = 0x31,
	N_RELEASED = 0xB1,
	M_PRESSED = 0x32,
	M_RELEASED = 0xB2,

	ZERO_PRESSED = 0x29,
	ONE_PRESSED = 0x2,
	NINE_PRESSED = 0xA,

	POINT_PRESSED = 0x34,
	POINT_RELEASED = 0xB4,

	SLASH_RELEASED = 0xB5,

	BACKSPACE_PRESSED = 0xE,
	BACKSPACE_RELEASED = 0x8E,
	SPACE_PRESSED = 0x39,
	SPACE_RELEASED = 0xB9,
	ENTER_PRESSED = 0x1C,
	ENTER_RELEASED = 0x9C,
};

module_init(keyb_init);
module_exit(keyb_exit);

/*Initialize the module − register the IRQ handler*/
int keyb_init(void)
{
	int result;
	
	printk("This is beginning of the keyb_init");
	result = register_chrdev(keyb_major, "keyboard", &keyb_fops);
	
	if (result < 0)
	{
		printk("<1>memory: cannot obtain major number %d\n", keyb_major);
		return result;
	}	

	queue_buffer = (char *) kmalloc(256, GFP_KERNEL);
	printk("This is keyb_init.");
	
	/*Request IRQ 1, the keyboard IRQ, to go to our irq_handler SA_SHIRQ means we're willing to have othe handlers on this IRQ. SA_INTERRUPT can be used to make the handler into a fast interrupt.*/
	return request_irq(1, (irq_handler_t) irq_handler, IRQF_SHARED, "test_keyboard_irq_handler", (void *)(irq_handler));
}

void keyb_exit(void)
{
	unregister_chrdev(keyb_major, "keyboard");
	kfree(queue_buffer);
	front = rear = ct = 0;
	printk("Byebye, this is keyb_exit. ");

}

int queue_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int queue_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/*This function services keyboard interrupts.*/
irq_handler_t irq_handler (int irq, void *dev_id, struct pt_regs *regs)
{
	/*Read keyboard status*/
	char pressed;

	pressed = keyboard_to_ascii(inb(0x60));//get the ascii of keyboard
	if (pressed != '\0')
	{
		
		queue_buffer[rear] = pressed;
		rear = (rear+1)%maxsize;
		ct++;

	}	
	return (irq_handler_t) IRQ_HANDLED; 
}


ssize_t queue_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{

	int i, b;
    if( ct == 0 )
	    return 0;
	else if (count > ct)
	{
		if( copy_to_user(buf, queue_buffer + front, ct) != 0 )
			return -EFAULT;
	    else
	    {	
	    	b = ct;
	    	front = (front+ct)%maxsize;  
	    	ct=0;  
	    	//printk("b: %d,  ct:%d\n", b, ct);	
	    	return b;
	    }
	}
	else
	{
		if( copy_to_user(buf, queue_buffer + front, count) != 0 )
			return -EFAULT;
	    else
	    {	
	    	front = (front+count)%maxsize;
	    	ct = ct-count;
	    	//printk("count: %d,  ct:%d\n", count, ct);
	    	return count;
	    }

	}	
   
	
}


static char* _qwertzuiop = "qwertzuiop"; // 0x10-0x1c
static char* _asdfghjkl = "asdfghjkl";
static char* _yxcvbnm = "yxcvbnm";
static char* _num = "123456789";

uint8_t keyboard_to_ascii(uint8_t key)
{
	//printk("This is keyboard_to_ascii function.\n");
	if(key == NULL_KEY)
		return '\0';
	if(key == 0x1C)
		return '\n';
	if(key == 0x39)
		return ' ';
	if(key == 0xE)
		return '\r';
	if(key == POINT_RELEASED)
		return '.';
	if(key == SLASH_RELEASED)
		return '/';
	if(key == ZERO_PRESSED)
		return '0';
	if(key >= ONE_PRESSED && key <= NINE_PRESSED)
		return _num[key - ONE_PRESSED];
	if(key >= 0x10 && key <= 0x1C)
	{
		return _qwertzuiop[key - 0x10];
	}
	else if(key >= 0x1E && key <= 0x26)
	{
		return _asdfghjkl[key - 0x1E];
	}
	else if(key >= 0x2C && key <= 0x32)
	{
		return _yxcvbnm[key - 0x2C];
	}
	return 0;
}

/*
ssize_t queue_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	char *tmp;
    tmp = buf+count-1;
	printk("This queue_write function \n");
    copy_from_user(queue_rear, tmp, 1); //transfers the data from user space to kernel space.
    queue_rear += 1;
    return sizeof(char);
}

?interrupt function build the queue, then when we cat, the read function will read the input. 
*/
