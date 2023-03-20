#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>
#include <linux/seq_file.h>

MODULE_LICENSE("Dual BSD/GPL");

#define BUF_LEN 100

char msg[BUF_LEN];
static struct proc_dir_entry *proc_entry;
bool opened = false;
struct timespec64 initTime;
static int procfs_buf_len;

static int timer_open(struct inode *inode, struct file *file)
{
    struct timespec64 ts;
    time64_t elapsedSec;
    long elapsedNano;
    printk(KERN_INFO "proc_open\n");
    if (opened == false)
    {
        opened = true;
        ktime_get_real_ts64(&ts);
        initTime = ts;
        printk(KERN_INFO "current time: %lld.%ld\n", ts.tv_sec, ts.tv_nsec);
        snprintf(msg, sizeof(msg), "current time: %lld.%ld\n", ts.tv_sec, ts.tv_nsec);
    }
    else
    {
        ktime_get_real_ts64(&ts);
        elapsedSec = ts.tv_sec - initTime.tv_sec;
        elapsedNano = ts.tv_nsec - initTime.tv_nsec;
        if (elapsedNano < 0)
        {
            elapsedSec = elapsedSec - 1;
            elapsedNano = elapsedNano + 1000000000;
        }
        printk(KERN_INFO "current time: %lld.%ld\n", ts.tv_sec, ts.tv_nsec);
        printk(KERN_INFO "elapsed time: %lld.%ld\n", elapsedSec, elapsedNano);
        snprintf(msg, sizeof(msg), "current time: %lld.%ld\n", ts.tv_sec, ts.tv_nsec);
        snprintf(msg + strlen(msg), sizeof(msg), "elapsed time: %lld.%ld\n", elapsedSec, elapsedNano);
    }

    return 0;
}

static ssize_t procfile_read(struct file *file, char *ubuf, size_t count, loff_t *ppos)
{
    printk(KERN_INFO "proc_read\n");
    procfs_buf_len = strlen(msg);

    if (*ppos > 0 || count < procfs_buf_len)
        return 0;

    if (copy_to_user(ubuf, msg, procfs_buf_len))
        return -EFAULT;

    *ppos = procfs_buf_len;

    printk(KERN_INFO "gave to user %s\n", msg);

    return procfs_buf_len;
}

static struct proc_ops procfile_fops =
    {
        .proc_open = timer_open,
        .proc_read = procfile_read,
};

static int timer_init(void)
{

    proc_entry = proc_create("timer", 0666, NULL, &procfile_fops);
    if (proc_entry == NULL)
        return -ENOMEM;
    return 0;
}

static void timer_exit(void)
{
    proc_remove(proc_entry);
    return;
}

module_init(timer_init);
module_exit(timer_exit);