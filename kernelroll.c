#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <asm/unistd.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("Siddhant Kalgutkar");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("This module replaces the open system call for text files with a custom open system call which prints HelloWorld in the current console");


static void **sys_call_table = NULL;

static asmlinkage long (*old_open) (const char __user *filename, int flags, umode_t mode);

static void print_string(char *str) 
{ 
    struct tty_struct *my_tty = get_current_tty(); 
    if (my_tty) { 
        const struct tty_operations *ttyops = my_tty->driver->ops; 
        (ttyops->write)(my_tty, /* The tty itself */ 
                        str, /* String */ 
                        strlen(str)); /* Length */ 
        (ttyops->write)(my_tty, "\015\012", 2); 
    } 
} 
 
static asmlinkage long my_open(const char __user *filename, int flags, umode_t mode)
{
        size_t len = strlen(filename);
        const char *ext = filename + len - 4;
        if (strncmp(ext, ".txt", 4) == 0)
        {
            print_string("Hello world!"); 
        }
        return old_open(filename, flags, mode);
}

static int __init syscall_init(void)
{
        sys_call_table = (void **)kallsyms_lookup_name("sys_call_table");
        pr_info("Found sys_call_table at %p\n", sys_call_table);
        old_open = sys_call_table[__NR_open];
        sys_call_table[__NR_open] = my_open;
        pr_info("Old open: %p; My open: %p\n", old_open, my_open);
        return 0;
}

static void __exit syscall_exit(void)
{
        pr_info("Syscall module unloaded");
        sys_call_table[__NR_open] = old_open;
}

module_init(syscall_init);
module_exit(syscall_exit);
