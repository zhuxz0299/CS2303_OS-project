/**
 * Kernel module that communicates with /proc file system.
 *
 * This provides the base logic for Project 2 - displaying task information
 */

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>

#define BUFFER_SIZE 128
#define PROC_NAME "pid"

/* the current pid */
static long l_pid;

/**
 * Function prototypes
 */
static ssize_t proc_read(struct file *file, char *buf, size_t count, loff_t *pos);
static ssize_t proc_write(struct file *file, const char __user *usr_buf, size_t count, loff_t *pos);

static struct proc_ops proc_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write};

/* This function is called when the module is loaded. */
static int proc_init(void)
{
    // creates the /proc/pid entry
    proc_create(PROC_NAME, 0666, NULL, &proc_ops);
    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

/* This function is called when the module is removed. */
static void proc_exit(void)
{
    // removes the /proc/pid entry
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

/**
 * This function is called each time the /proc/pid is read.
 *
 * This function is called repeatedly until it returns 0, so
 * there must be logic that ensures it ultimately returns 0
 * once it has collected the data that is to go into the
 * corresponding /proc file.
 */
static ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
    int rv = 0;
    char buffer[BUFFER_SIZE];
    static int completed = 0;
    struct task_struct *tsk = NULL;

    if (completed)
    {
        completed = 0;
        return 0;
    }

    tsk = pid_task(find_vpid(l_pid), PIDTYPE_PID);
    // 函数find_vpid()通过进程ID查找到对应的pid结构体，并返回一个指向该结构体的指针。
    // pid_task()函数则将pid结构体转换成对应的task_struct结构体，并返回一个指向该结构体的指针。
    // PIDTYPE_PID是一个enum常量,表示pid类型

    if (tsk == NULL)
        return -1; // no such pid
    rv = sprintf(buffer, "command = [%s], pid = [%ld], state = [%d]\n", tsk->comm, l_pid, tsk->__state);
    // sprintf()函数返回输出的字符串的长度，也就是写入到缓冲区中的字符数，如果发生错误则返回一个负数。

    completed = 1;

    // copies the contents of kernel buffer to userspace usr_buf
    if (copy_to_user(usr_buf, buffer, rv))
        rv = -1;

    return rv;
}

/**
 * This function is called each time we write to the /proc file system.
 */
static ssize_t proc_write(struct file *file, const char __user *usr_buf, size_t count, loff_t *pos)
{
    char *k_mem;
    char buffer[BUFFER_SIZE];

    // allocate kernel memory
    k_mem = kmalloc(count, GFP_KERNEL);

    /* copies user space usr_buf to kernel buffer */
    if (copy_from_user(k_mem, usr_buf, count))
    {
        printk(KERN_INFO "Error copying from user\n");
        return -1;
    }

    /**
     * kstrol() will not work because the strings are not guaranteed
     * to be null-terminated.
     *
     * sscanf() must be used instead.
     */ 

    sscanf(k_mem, "%s", buffer);
    if (kstrtol(buffer, 10, &l_pid))
        printk(KERN_ERR "Error in converting\n");
    kfree(k_mem);

    return count;
}

/* Macros for registering module entry and exit points. */
module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module");
MODULE_AUTHOR("SGG");