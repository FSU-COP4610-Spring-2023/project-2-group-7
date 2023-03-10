#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

/* System call stub */
int (*STUB_initialize_bar)(void) = NULL;
int (*STUB_customer_arrival)(int, int) = NULL;
int (*STUB_close_bar)(void) = NULL;
EXPORT_SYMBOL(STUB_initialize_bar);
EXPORT_SYMBOL(STUB_customer_arrival);
EXPORT_SYMBOL(STUB_close_bar);

/* System call wrapper */
SYSCALL_DEFINE0(initialize_bar)
{
    printk(KERN_NOTICE "Inside SYSCALL_DEFINE1 block. %s\n", __FUNCTION__);
    if (STUB_initialize_bar != NULL)
        return STUB_initialize_bar();
    else
        return -ENOSYS;
}
/* System call wrapper */
SYSCALL_DEFINE2(customer_arrival, int, number_of_customers, int, type)
{
    printk(KERN_NOTICE "Inside SYSCALL_DEFINE2 block. %s\n", __FUNCTION__);
    if (STUB_customer_arrival != NULL)
        return STUB_customer_arrival(number_of_customers, type);
    else
        return -ENOSYS;
}
/* System call wrapper */
SYSCALL_DEFINE0(close_bar)
{
    printk(KERN_NOTICE "Inside SYSCALL_DEFINE3 block. %s\n", __FUNCTION__);
    if (STUB_close_bar != NULL)
        return STUB_close_bar();
    else
        return -ENOSYS;
}
