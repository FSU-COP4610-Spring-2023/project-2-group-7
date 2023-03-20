#include <linux/init.h>
#include <linux/module.h>
#include <linux/linkage.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/random.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");

/* System call stub */
extern int (*STUB_initialize_bar)(void);
extern int (*STUB_customer_arrival)(int, int);
extern int (*STUB_close_bar)(void);

#define BUF_LEN 100000
char msg[BUF_LEN];
static struct proc_ops pops;
static struct proc_dir_entry *proc_entry;
bool opened = false;
bool closing = true;
bool closed = true;
struct timespec64 initTime;
time64_t closingTime;
static int procfs_buf_len;
const int MIN = 0;
const int MAX = 32;
int currOcup = 0;
int custServed = 0;
int custWaiting = 0;
int grpWaiting = 0;
int numFreshman = 0;
int numSophomores = 0;
int numJuniors = 0;
int numSeniors = 0;
int numProfessors = 0;
int groupsServed = 0;
int cleanBarSeats = 32;
enum state { offline = 1, idle = 2, loading = 3, cleaning = 4, moving = 5 };
enum student { freshman = 0, sophomore = 1, junior = 2, senior = 3, grads = 4 };

typedef struct {
	int group_id; // group_id could be a random number between INT_MIN and INT_MAX to keep track of groups
	int type;
} customer;

typedef struct group{
	int cust_group[8];
	int group_id;
	int size;
	int type;
	struct list_head list;
} Group;

typedef struct {
	bool occupied; //true open and false for occupied
	int type;
	bool clean;
	struct timespec64 time_sat;
} seat;

typedef struct {
	seat seats[8];
	int cleanSeats;
} table;

typedef struct {
	struct list_head queue;
	table tables[4];
} bar;

typedef struct {
	int state;
	int currTable;
} waiter;

struct thread_parameter
{
	int id;
	struct task_struct *kthread;
	struct mutex mutex;
};

struct thread_parameter thread1;

bar Bar;
waiter Waiter;

void loadGroup(void) {
	// list_del_head

	// struct list_head temp;
	// struct list_head move_list;
	Group* g = NULL;
	struct timespec64 currTrime;
	int i, start, j;
	bool added = false;
	ktime_get_real_ts64(&currTrime);

	g = list_first_entry(&Bar.queue, Group, list);	

	for (i=0; i<4; i++){
		if (Bar.tables[i].cleanSeats >= g->size){
			if (Waiter.currTable != i){
				Waiter.state = 5;
				mutex_unlock(&thread1.mutex);
				ssleep(2);
				mutex_lock_interruptible(&thread1.mutex);
				Waiter.currTable = i;
			}
			Waiter.state = 3;
						
			start = 8 - Bar.tables[i].cleanSeats;
			for (j = start; j<start+g->size; j++){
				Bar.tables[i].seats[j].type = g->type;
				Bar.tables[i].seats[j].occupied = true;
				Bar.tables[i].seats[j].clean = false;
				Bar.tables[i].seats[j].time_sat = currTrime;
				Bar.tables[i].cleanSeats--;
				cleanBarSeats--;
			}
			i = 5;
			added = true;
		}
	}
	if (added = true){
		grpWaiting--;
		custWaiting -= g->size;
		currOcup += g->size;
		switch(g->type){
			case 0:	numFreshman += g->size;
				break;
			case 1: numSophomores += g->size;
				break;
			case 2:	numJuniors += g->size;
				break;
			case 3: numSeniors += g->size;
					break;
			case 4: numProfessors += g->size;
				break;
		}

		list_del(&g->list); /* removes entry from list */
		kfree(g);
	}
}

void removeGroup(void){
	
	int i;
	int currTable = Waiter.currTable;
	time64_t drinkingTime;
	struct timespec64 currTime;
	bool unloading = false;
	ktime_get_real_ts64(&currTime);
	for (i=0; i<8; i++){
		if (Bar.tables[currTable].seats[i].occupied == true)
		{
			drinkingTime = currTime.tv_sec - Bar.tables[currTable].seats[i].time_sat.tv_sec;
			switch(Bar.tables[currTable].seats[i].type){
				case 0: 
					if (drinkingTime >= 5){
							Bar.tables[currTable].seats[i].occupied = false;
							numFreshman -= 1;
							currOcup -= 1;
							custServed++;
							unloading = true;
						}
					break;
				case 1: 
					if (drinkingTime >= 10){
							Bar.tables[currTable].seats[i].occupied = false;
							numSophomores -= 1;
							currOcup -= 1;
							custServed++;
							unloading = true;
						}
					break;
				case 2:
					if (drinkingTime >= 15){
							Bar.tables[currTable].seats[i].occupied = false;
							numJuniors -= 1;
							currOcup -= 1;
							custServed++;
							unloading = true;
						}
					break;
				case 3: 
					if (drinkingTime >= 20){
							Bar.tables[currTable].seats[i].occupied = false;
							numSeniors -= 1;
							currOcup -= 1;
							custServed++;
							unloading = true;
						}
						break;
				case 4: 
					if (drinkingTime >= 25){
							Bar.tables[currTable].seats[i].occupied = false;
							numProfessors -= 1;
							currOcup -= 1;
							custServed++;
							unloading = true;
						}
					break;
			}
		}
	}
	if (unloading == true)
	{
		Waiter.state = 3;
		mutex_unlock(&thread1.mutex);
		ssleep(1);
		mutex_lock_interruptible(&thread1.mutex);
	}
	else{
		Waiter.state = 5;
		mutex_unlock(&thread1.mutex);
		ssleep(2);
		mutex_lock_interruptible(&thread1.mutex);
		Waiter.currTable = (Waiter.currTable + 1) % 4;
	}
	
	
}
void cleanTables(void){

	int i, j;
	for (i = 0; i<4; i++){
		if (Bar.tables[i].cleanSeats < 8){
			Waiter.state = 4;
			Waiter.currTable = i;
			mutex_unlock(&thread1.mutex);
			ssleep(10);
			mutex_lock_interruptible(&thread1.mutex);

			for (j = 0; j<8; j++){
					Bar.tables[i].seats[j].clean = true;
			}
		}
		Bar.tables[i].cleanSeats = 8;
		Waiter.state = 5;
		mutex_unlock(&thread1.mutex);
		ssleep(2);
		mutex_lock_interruptible(&thread1.mutex);
	}
	cleanBarSeats = 32;
}
int elapsedTime(void)
{
	struct timespec64 ts;
	time64_t elapsedSec;
	long elapsedNano;
	ktime_get_real_ts64(&ts);
	elapsedSec = ts.tv_sec - initTime.tv_sec;
	elapsedNano = ts.tv_nsec - initTime.tv_nsec;
	if (elapsedNano < 0) {
		elapsedSec = elapsedSec - 1;
		elapsedNano = elapsedNano + 1000000000;
	}
	if (closed == true)
	{
		return closingTime;
	}

	return elapsedSec;
}

/******************************************************************************!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
int thread_run(void *data)
{
	struct thread_parameter *parm = data;
	
	while (!kthread_should_stop())
	{
		if (mutex_lock_interruptible(&parm->mutex) == 0) {
			if (closed == false){
				Group* g = NULL;
				if(!list_empty(&Bar.queue))
					g = list_first_entry(&Bar.queue, Group, list);
				if (grpWaiting > 0 && cleanBarSeats >= g->size) {
					
					Waiter.state = 3;
					mutex_unlock(&parm->mutex);
					ssleep(1);
					mutex_lock_interruptible(&parm->mutex);
					loadGroup();
				} 
				else if (currOcup > 0) {
					removeGroup();
				}
				else if (currOcup == 0 && cleanBarSeats < 32 && Waiter.currTable == 0)
				{
					cleanTables();
				}
				else if (grpWaiting == 0 && currOcup == 0 && cleanBarSeats == 32 && closing == true)
				{
					Waiter.state = 1;
					if (Waiter.currTable != 0)
					{
						Waiter.state = 5;
						mutex_unlock(&parm->mutex);
						ssleep(2);
						mutex_lock_interruptible(&parm->mutex);
						Waiter.currTable = 0;
					}
					closingTime = elapsedTime();
					closed = true;
				}
				else{
					Waiter.state = 5;
					mutex_unlock(&parm->mutex);
					ssleep(2);
					mutex_lock_interruptible(&parm->mutex);
					Waiter.currTable = (Waiter.currTable + 1) % 4;
				}	
			} else {
				ssleep(1);
			}
			mutex_unlock(&parm->mutex);
		}
	}

	return 0;
}
void thread_init_parameter(struct thread_parameter *parm)
{
	static int id = 1;

	parm->id = id++;
	
	mutex_init(&parm->mutex);
	parm->kthread = kthread_run(thread_run, parm, "barstool", parm->id);
}
/******************************************************************************/



int my_initilize_bar(void)
{
	if (Waiter.state != 1)
		return 1;
	else if (Waiter.state == 1)
	{
		int i, j;
		
		ktime_get_real_ts64(&initTime);
		// initTime = ts;
		Waiter.state = 2;
		Waiter.currTable = 0;
		currOcup = 0;
		closed = false;
		closing = false;
		INIT_LIST_HEAD(&Bar.queue);
		for (i = 0; i < 4; i++) 
		{
			for (j = 0; j < 8; j++) 
			{
				Bar.tables[i].seats[j].clean = true;
				Bar.tables[i].seats[j].occupied = false;
				Bar.tables[i].seats[j].type = 0;
				Bar.tables[i].cleanSeats = 8;
			}
		}
		return 0;
	}
	else
		return -1;
}

int my_customer_arrival(int number_of_customers, int type)
{
	if (closing == false){
		if (number_of_customers > 8 || number_of_customers < 1 || type > 4 || type < 0)
			return 1;
		

		int group_id;
		int size = number_of_customers;
		int i;
		get_random_bytes(&group_id, sizeof(group_id));

		Group* g;
		g = kmalloc(sizeof(Group) * 1, __GFP_RECLAIM);

		for (i = 0; i<number_of_customers; i++)
		{
			g->cust_group[i] = type;
			
		}
		g->size = size;
		g->group_id = group_id;
		g->type = type;
		list_add_tail(&g->list, &Bar.queue);

		custWaiting += number_of_customers;
		grpWaiting += 1;
	}

	return 0;
}

void delete_groups(void) {
	struct list_head move_list;
	struct list_head *temp;
	struct list_head *dummy;
	// int i;
	Group *g;

	INIT_LIST_HEAD(&move_list);

	/* move items to a temporary list to illustrate movement */
	list_for_each_safe(temp, dummy, &Bar.queue) { /* forwards */
		g = list_entry(temp, Group, list);

		list_move_tail(temp, &move_list); /* move to back of list */
		
	}
	
	list_for_each_safe(temp, dummy, &Bar.queue) {
		g = list_entry(temp, Group, list);
		list_del(temp);	/* removes entry from list */
		kfree(g);
	}
}
int my_close_bar(void)
{
	
	if (closing == true)
		return 1;
	else {
		closing = true;
		grpWaiting = 0;
		delete_groups();
		
	}
	// printk(KERN_NOTICE "%s\n", __FUNCTION__);
	return 0;
}


char* studentType(int type)
{
	char* custType;

	switch(type)
	{
		case 0:
			custType = "F";
			break;
		case 1:
			custType = "O";
			break;
		case 2:
			custType = "J";
			break;
		case 3:
			custType = "S";
			break;
		case 4:
			custType = "P";
			break;
	}

	return custType;
}

void printEmptyBar(void){
	int i, j;
	for(i = 3; i >= 0; i--)
	{
		if ( i == 0)
			snprintf(msg + strlen(msg), sizeof(msg), "[*] Table %d: ", i + 1);
		else
			snprintf(msg + strlen(msg), sizeof(msg), "[ ] Table %d: ", i + 1);
		for(j = 0; j < 8; j++)
		{
			snprintf(msg + strlen(msg), sizeof(msg), "C ");
		}
		snprintf(msg + strlen(msg), sizeof(msg), "\n");
	}
}

void printTables(bar b, waiter w)
{
	int i, j;
	char* waiterState;
	char* custType;
	char* seatStatus;
	struct list_head *temp;
	Group* g;

	switch(Waiter.state)
	{
		case 1:
			waiterState = "OFFLINE";
			break;
		case 2:
			waiterState = "IDLE";
			break;
		case 3:
			waiterState = "LOADING";
			break;
		case 4:
			waiterState = "CLEANING";
			break;
		case 5:
			waiterState = "MOVING";
			break;
	}

	if (mutex_lock_interruptible(&thread1.mutex) == 0) {

		snprintf(msg, sizeof(msg), "Waiter state: %s\n", waiterState);
		snprintf(msg + strlen(msg), sizeof(msg), "Current table: %d\n", Waiter.currTable+1);
		snprintf(msg + strlen(msg), sizeof(msg), "Elapsed time: %d seconds\n", elapsedTime());
		snprintf(msg + strlen(msg), sizeof(msg), "Current occupency: %d\n", currOcup);
		snprintf(msg + strlen(msg), sizeof(msg), "Bar status: ");
		if(numFreshman > 0)
		{
			snprintf(msg + strlen(msg), sizeof(msg), "%d F, ", numFreshman);
		}
		if(numSophomores > 0)
		{
			snprintf(msg + strlen(msg), sizeof(msg), "%d O, ", numSophomores);
		}
		if(numJuniors > 0)
		{
			snprintf(msg + strlen(msg), sizeof(msg), "%d J, ", numJuniors);
		}
		if(numSeniors > 0)
		{
			snprintf(msg + strlen(msg), sizeof(msg), "%d S, ", numSeniors);
		}
		if(numProfessors > 0)
		{
			snprintf(msg + strlen(msg), sizeof(msg), "%d P, ", numProfessors);
		}

		// need a loop that checks how many of each type
		snprintf(msg + strlen(msg), sizeof(msg), "\nNumber of customers waiting: %d\n", custWaiting);
		snprintf(msg + strlen(msg), sizeof(msg), "Number of groups waiting: %d\n", grpWaiting);
		snprintf(msg + strlen(msg), sizeof(msg), "Contents of queue\n");
		//loop to print groups
		if (grpWaiting > 0){
			list_for_each(temp, &Bar.queue) {
				g = list_entry(temp, Group, list);

				for (i = 0; i < g->size; i++){

					custType = studentType(g->cust_group[i]);
					snprintf(msg + strlen(msg), sizeof(msg), "%s ", custType);
				}
				snprintf(msg + strlen(msg), sizeof(msg), "(group id: %d)\n", g->group_id);
			}
		}

		snprintf(msg + strlen(msg), sizeof(msg), "Number of customers serviced: %d\n", custServed);
		snprintf(msg + strlen(msg), sizeof(msg), "\n");
		// int i, j;
		if (closed == false) {
			for (i = 3; i >= 0; i--) {
				if (w.currTable == i)
					snprintf(msg + strlen(msg), sizeof(msg),
						"[*] Table %d: ", i + 1);
				else
					snprintf(msg + strlen(msg), sizeof(msg),
						"[ ] Table %d: ", i + 1);
				for (j = 0; j < 8; j++) 
				{
					if(b.tables[i].seats[j].occupied == false && b.tables[i].seats[j].clean == true)
						seatStatus = "C";
					else if(b.tables[i].seats[j].occupied == false && b.tables[i].seats[j].clean == false)
						seatStatus = "D";
					else
						seatStatus = studentType(b.tables[i].seats[j].type);
					
					snprintf(msg + strlen(msg), sizeof(msg), "%s ", seatStatus);
				}
				snprintf(msg + strlen(msg), sizeof(msg), "\n");
			}
		} 
		else {
			printEmptyBar();
		}
		mutex_unlock(&thread1.mutex);
	}
		
}


static int bar_proc_open(struct inode *inode, struct file *file)
{
	
	printTables(Bar, Waiter);

	return 0;
}

static ssize_t bar_proc_read(struct file *file, char *ubuf, size_t count, loff_t *ppos)
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


static int bar_init(void) {
	STUB_initialize_bar = my_initilize_bar;
	STUB_customer_arrival = my_customer_arrival;
	STUB_close_bar = my_close_bar;
	Waiter.state = 1;
	pops.proc_open = bar_proc_open;
	pops.proc_read = bar_proc_read;
	
	proc_entry = proc_create("majorsbar", 0666, NULL, &pops);
	if (proc_entry == NULL)
		return -ENOMEM;

	thread_init_parameter(&thread1);
	if (IS_ERR(thread1.kthread))
	{
		printk(KERN_WARNING "error spawning thread");
		remove_proc_entry("majorsbar", NULL);
		return PTR_ERR(thread1.kthread);
	}
	return 0;
}

module_init(bar_init);

static void bar_exit(void) {
	STUB_initialize_bar = NULL;
	STUB_customer_arrival = NULL;
	STUB_close_bar = NULL;
	kfree(msg);
	kthread_stop(thread1.kthread); // stops thread very important
	proc_remove(proc_entry);
	return;
}

module_exit(bar_exit);


