#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/compiler.h>
#include <net/tcp.h>
#include <linux/namei.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/list.h>

MODULE_AUTHOR("Osanne Gbayere");
MODULE_DESCRIPTION ("Extensions to the firewall") ;
MODULE_LICENSE("GPL");

/* Locks */

DEFINE_MUTEX (devLock); /* Device Lock */

/* Constants */

static int BUFFER_LENGTH = 900;

static char* PROC_ENTRY_FILENAME = "firewallExtension";

static int check_validity(char *program, int port);

static int device_open = 0;  

/* Linked list of firewall rules */
struct rule_list {
  char *device;
  int size;
  int port;
  struct rule_list *next;
};

/* firewall rules */
struct rule_list* rules; 

/* process file to interface with firewallSetup.c */
static struct proc_dir_entry *Our_Proc_File;

/* Read-Write Semaphore for rule replacement policy */
static struct rw_semaphore rws;

/* make IP4-addresses readable */
#define NIPQUAD(addr) \
((unsigned char *)&addr)[0], \
((unsigned char *)&addr)[1], \
((unsigned char *)&addr)[2], \
((unsigned char *)&addr)[3]

struct nf_hook_ops *reg;

/* the firewall hook - called for each outgoing packet */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 3, 0)
#error "Kernel version < 4.4 not supported!"
/* kernels < 4.4 need another firewallhook! */
#endif

unsigned int FirewallExtensionHook (void *priv, 
  struct sk_buff *skb,
  const struct nf_hook_state *state) {

  struct tcphdr *tcp;
  struct tcphdr _tcph;
  struct sock *sk;
  struct mm_struct *mm;

  struct path path;
  pid_t mod_pid;
  char *program_path; // Path of the program 
  int port, res;
  char *buf;
  char cmdlineFile[BUFFER_LENGTH];

  sk = skb->sk;
  if (!sk) {
    /* firewall: netfilter called with empty socket */
    return NF_ACCEPT;
  }

  if (sk->sk_protocol != IPPROTO_TCP) {
    /* firewall: netfilter called with non-TCP-packet */
    return NF_ACCEPT;
  }

  /* get the tcp-header for the packet */
  tcp = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(struct tcphdr), &_tcph);
  if (!tcp) {
    /* Could not get tcp-header meaning non TCP pacekt */
    return NF_ACCEPT;
  }

  if (tcp->syn) {

    /* current is pre-defined pointer to task structure of currently 
    running task */
    mod_pid = current->pid;
    snprintf (cmdlineFile, BUFFER_LENGTH, "/proc/%d/exe", mod_pid);
    res = kern_path (cmdlineFile, LOOKUP_FOLLOW, &path);
    if (res) {
      printk(KERN_INFO "kern_path failed\n");
      
      return -EFAULT;
    }

    buf = (char *)kcalloc(100, sizeof(char), GFP_KERNEL);
    program_path = d_path(&path, buf, 100*sizeof(char));

    path_put(&path);

    if (in_irq() || in_softirq() || !(mm = get_task_mm(current))) {
      return NF_ACCEPT;
    }
    mmput(mm);

    /* get the port of the connection */
    port = ntohs (tcp->dest);

    /* if not valid connection, drop it */
    if(check_validity(program_path, port)==0){
      tcp_done (sk);  /*terminate Connectionction immediately */
      return NF_DROP;
    }

    /* free the buffer as it is no longer needed */
    kfree(buf);
  }

  
  /* otherwise accept the connection */
  return NF_ACCEPT;    
}

static struct nf_hook_ops firewallExtension_ops = {
 .hook    = FirewallExtensionHook,
 .pf      = PF_INET,
 .priority = NF_IP_PRI_FIRST,
 .hooknum = NF_INET_LOCAL_OUT
};

/* check whether the connection should be dropped or accepted
*  based on the firewall rules */
static int check_validity(char *program, int port){
  struct rule_list *head;
  int validity = 1;

  /* lock the read semaphore */
  down_read(&rws);

  head = rules;

  /* loop until the exact rule is found or 
  *  until the are no more rules */
  while(head){

    /* if an exact port is found then there must
       be a rule for the exact program, if not then drop 
       the connection. */
    if(port == head->port){
      if(strcmp(program, head->device)==0){
        validity = 1;
        break;
      }
      validity = 0;
    }
    head = head->next;
  }

  /* release the lock */
  up_read(&rws);

  return validity;
}

static ssize_t kernelWrite (struct file *file, 
  const char __user *buffer, size_t 
  count, loff_t *offset) {

  int errno;
  struct rule_list *head, *temp, *r, *item;
  char* filename ;

  r = kmalloc(sizeof(struct rule_list), GFP_KERNEL);
  head = r;

  errno = copy_from_user(r, (void *)buffer, sizeof(struct rule_list));
  if( errno != 0){
    head = NULL;
    goto end;
  }

  filename = kcalloc(r->size + 1, sizeof(char), GFP_KERNEL);
  errno = copy_from_user(filename, (void *)r->device, r->size + 1);

  if( errno != 0){
    head = r;
    head->device = NULL;
    head->next = NULL;
    goto end;
  }

  r->device = filename;

  if(errno != 0) 
    return -EINVAL;

  while(errno == 0){
    temp = kmalloc(sizeof(struct rule_list), GFP_KERNEL);
    errno = copy_from_user(temp, (void *)r->next, sizeof(struct rule_list)); 
    if(errno != 0){
      kfree(temp);
      break;
    }
    filename = kcalloc(temp->size + 1, sizeof(char), GFP_KERNEL);
    errno = copy_from_user(filename, (void *)temp->device, temp->size + 1);
    if( errno != 0){
      kfree(filename);
      break;
    }

    temp->device = filename;

    r->next = temp;
    r=r->next;     
  }
  r->next=NULL;

end:
  temp = rules;

  down_write(&rws);
  rules = head;
  up_write(&rws);

  /* Deallocate rules list */
  while(temp){
    item = temp;
    temp = temp->next;
    kfree(item->device);
    kfree(item);
  }

  return 0;  
}

/* 
    Reading from the proc file
*/
static ssize_t kernelRead(struct file *filp, 
         char *buffer,  /* buffer to fill with data */
         size_t length, /* length of the buffer */
loff_t * offset) {
  struct rule_list *head;
  
  /* lock the read semaphore */
  down_read(&rws);

  head = rules;
  while(head){
    printk(KERN_INFO "Firewall rule: %d %s\n", head->port, head->device);
    head = head->next;
  }

  /* release the lock */

  up_read(&rws);
  return 0;

}

static int procfs_open(struct inode *inode, struct file *file)
{
  mutex_lock (&devLock);
  if (device_open) {
    mutex_unlock (&devLock);
    return -EAGAIN;
  }
  device_open++;
  mutex_unlock (&devLock);

  try_module_get(THIS_MODULE);
  return 0; /* success */
}

/*
*  The proc file is closed
*/
static int procfs_close(struct inode *inode, struct file *file)
{
  mutex_lock (&devLock);
  device_open--;         
  mutex_unlock (&devLock);

  module_put(THIS_MODULE);
  return 0;        
}

static const struct file_operations File_Ops_4_Our_Proc_File = {
  .owner = THIS_MODULE,
  .write = kernelWrite,
  .read = kernelRead,
  .open = procfs_open,
  .release = procfs_close,
};

int init_module(void)
{
  int errno;

  rules = NULL;

  Our_Proc_File = proc_create_data (PROC_ENTRY_FILENAME, 0644, NULL, 
    &File_Ops_4_Our_Proc_File, NULL);

  if (Our_Proc_File == NULL){
    printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
      PROC_ENTRY_FILENAME);
    return -1;
  }

  errno = nf_register_hook (&firewallExtension_ops); /* register the hook */
  if (errno) {
    printk (KERN_INFO "Firewall extension could not be registered!\n");
  } 

  /* initialise semaphore */
  init_rwsem(&rws);

  return errno;
}


void cleanup_module(void)
{
  struct rule_list *item;

  remove_proc_entry(PROC_ENTRY_FILENAME, NULL);
  nf_unregister_hook (&firewallExtension_ops); 

  while(rules){
    item = rules;
    rules = rules->next;
    kfree(item->device);
    kfree(item);
  }
}  
