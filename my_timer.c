#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Project 2 Part 2");

#define MODULE_NAME "Timer"
#define MODULE_PERMISSIONS 0644
#define MODULE_PARENT NULL
#define STRING_LENGTH 256

static struct file_operations fileOps; // Points to proc file definitions.
static char* currentTimeMsg;
static char* elapsedTimeMsg;
static int readCounter = 0;
static int readProc;
struct timespec currentTime; // sec and nsec

// static long seconds = 0;
// static long nanoseconds = 0;
long secondsElapsed = 0;
long nanoSecondsElapsed = 0;


static long seconds = 0;
static long nanoseconds = 0;

//Define Open Proc to initialize our file & write
int OpenAndWrite(struct inode *sp_inode, struct file *sp_file)
{
	// Increment the readCounter
	readCounter++;
	readProc = 1;

	// Allocate space for currentTimeMsg and elapsedTimeMsg
	currentTimeMsg = kmalloc(sizeof(char)*STRING_LENGTH, __GFP_RECLAIM | __GFP_IO | __GFP_FS);

	elapsedTimeMsg = kmalloc(sizeof(char)*STRING_LENGTH, __GFP_RECLAIM | __GFP_IO | __GFP_FS);

	// To return error
	if(currentTimeMsg == NULL || elapsedTimeMsg == NULL)
	{
		return -ENOMEM;
	}

	//set current time struct
	currentTime = current_kernel_time();


	// Calculate the time difference

	if(nanoseconds > currentTime.tv_nsec)
	{
		secondsElapsed = currentTime.tv_sec - seconds - 1;
		nanoSecondsElapsed = currentTime.tv_nsec - nanoseconds + 1000000000;
	}
	else
	{
		secondsElapsed = currentTime.tv_sec - seconds;
		nanoSecondsElapsed = currentTime.tv_nsec - nanoseconds;
	}


	// Set messages
	sprintf(currentTimeMsg, "Current Time: %ld.%ld\n",currentTime.tv_sec, currentTime.tv_nsec);

	seconds = currentTime.tv_sec;
	nanoseconds = currentTime.tv_nsec;

	// Set elapsed Time Msg
	if(readCounter > 1)
	{
		sprintf(elapsedTimeMsg, "Elapsed Time: %ld.%ld\n",secondsElapsed, nanoSecondsElapsed);

		// Append elapsedTimeMsg to currentTimeMsg
		strcat(currentTimeMsg, elapsedTimeMsg);

	}

	printk(KERN_NOTICE "%s\n", currentTimeMsg);

	return 0;
}

// Copy to User
ssize_t Read(struct file *sp_file, char __user *buffer, size_t size, loff_t *offset) {

	int length = 0;

	readProc = !readProc;

	if(readProc)
	{
		return 0;
	}

	// Get the length of message
	length = strlen(currentTimeMsg);
	// Copy message to user
	copy_to_user(buffer, currentTimeMsg, length);
	return length;

}

	// Free Module
int FreeData(struct inode *sp_inode, struct file *sp_file) {

    printk(KERN_NOTICE "freeData.\n");

	// Free data
    kfree(currentTimeMsg);
    kfree(elapsedTimeMsg);

    return 0;
}

// Initialize Module
static int initializeModule(void) {

   fileOps.open = OpenAndWrite;
   fileOps.read = Read;
   fileOps.release = FreeData;


    if (!proc_create(MODULE_NAME, MODULE_PERMISSIONS, MODULE_PARENT, &fileOps))
	{
        printk(KERN_WARNING "error: could not create proc file.\n");
        remove_proc_entry(MODULE_NAME, MODULE_PARENT);

        return -ENOMEM;
    }

    return 0;
}



// Exit Module
static void exitModule(void) {
    printk(KERN_NOTICE "Removing /proc/%s.\n", MODULE_NAME);
    remove_proc_entry(MODULE_NAME, MODULE_PARENT);
}

module_init(initializeModule);
module_exit(exitModule);
