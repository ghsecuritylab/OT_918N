
#include <slirp.h>

/* XXX: only DHCP is supported */

#define NB_ADDR 16

#define START_ADDR 15

#define LEASE_TIME (24 * 3600)

typedef struct {
    uint8_t allocated;
    uint8_t macaddr[6];
} BOOTPClient;

static BOOTPClient bootp_clients[NB_ADDR];

const char *bootp_filename;

static const uint8_t rfc1533_cookie[] = { RFC1533_COOKIE };

#ifdef DEBUG
#define dprintf(fmt, ...) \
if (slirp_debug & DBG_CALL) { fprintf(dfd, fmt, ##  __VA_ARGS__); fflush(dfd); }
#else
#define dprintf(fmt, ...)
#endif

static BOOTPClient *get_new_addr(struct in_addr *paddr,
                                 const uint8_t *macaddr)
{
    BOOTPClient *bc;
    int i;

    for(i = 0; i < NB_ADDR; i++) {
        bc = &bootp_clients[i];
        if (!bc->allocated || !memcmp(macaddr, bc->macaddr, 6))
            goto found;
    }
    return NULL;
 found:
    bc = &bootp_clients[i];
    bc->allocated = 1;
    paddr->s_addr = htonl(ntohl(special_addr.s_addr) | (i + START_ADDR));
    return bc;
}

static BOOTPClient *request_addr(const struct in_addr *paddr,
                                 const uint8_t *macaddr)
{
    uint32_t req_addr = ntohl(paddr->s_addr);
    uint32_t spec_addr = ntohl(special_addr.s_addr);
    BOOTPClient *bc;

    if (req_addr >= (spec_addr | START_ADDR) &&
        req_addr < (spec_addr | (NB_ADDR + START_ADDR))) {
        bc = &bootp_clients[(req_addr & 0xff) - START_ADDR];
        if (!bc->allocated || !memcmp(macaddr, bc->macaddr, 6)) {
            bc->allocated = 1;
            return bc;
        }
    }
    return NULL;
}

static BOOTPClient *find_addr(struct in_addr *paddr, const uint8_t *macaddr)
{
    BOOTPClient *bc;
    int i;

    for(i = 0; i < NB_ADDR; i++) {
        if (!memcmp(macaddr, bootp_clients[i].macaddr, 6))
            goto found;
    }
    return NULL;
 found:
    bc = &bootp_clients[i];
    bc->allocated = 1;
    paddr->s_addr = htonl(ntohl(special_addr.s_addr) | (i + START_ADDR));
    return bc;
}

static void dhcp_decode(const struct bootp_t *bp, int *pmsg_type,
                        const struct in_addr **preq_addr)
{
    const uint8_t *p, *p_end;
    int len, tag;

    *pmsg_type = 0;
    *preq_addr = NULL;

    p = bp->bp_vend;
    p_end = p + DHCP_OPT_LEN;
    if (memcmp(p, rfc1533_cookie, 4) != 0)
        return;
    p += 4;
    while (p < p_end) {
        tag = p[0];
        if (tag == RFC1533_PAD) {
            p++;
        } else if (tag == RFC1533_END) {
            break;
        } else {
            p++;
            if (p >= p_end)
                break;
            len = *p++;
            dprintf("dhcp: tag=%d len=%d\n", tag, len);

            switch(tag) {
            case RFC2132_MSG_TYPE:
                if (len >= 1)
                    *pmsg_type = p[0];
                break;
            case RFC2132_REQ_ADDR:
                if (len >= 4)
                    *preq_addr = (struct in_addr *)p;
                break;
            default:
                break;
            }
            p += len;
        }
    }
    if (*pmsg_type == DHCPREQUEST && !*preq_addr && bp->bp_ciaddr.s_addr) {
        *preq_addr = &bp->bp_ciaddr;
    }
}

static void bootp_reply(const struct bootp_t *bp)
{
    BOOTPClient *bc = NULL;
    struct mbuf *m;
    struct bootp_t *rbp;
    struct sockaddr_in saddr, daddr;
    struct in_addr dns_addr;
    const struct in_addr *preq_addr;
    int dhcp_msg_type, val;
    uint8_t *q;

    /* extract exact DHCP msg type */
    dhcp_decode(bp, &dhcp_msg_type, &preq_addr);
    dprintf("bootp packet op=%d msgtype=%d", bp->bp_op, dhcp_msg_type);
    if (preq_addr)
        dprintf(" req_addr=%08x\n", ntohl(preq_addr->s_addr));
    else
        dprintf("\n");

    if (dhcp_msg_type == 0)
        dhcp_msg_type = DHCPREQUEST; /* Force reply for old BOOTP clients */

    if (dhcp_msg_type != DHCPDISCOVER &&
        dhcp_msg_type != DHCPREQUEST)
        return;
    /* XXX: this is a hack to get the client mac address */
    memcpy(client_ethaddr, bp->bp_hwaddr, 6);

    if ((m = m_get()) == NULL)
        return;
    m->m_data += IF_MAXLINKHDR;
    rbp = (struct bootp_t *)m->m_data;
    m->m_data += sizeof(struct udpiphdr);
    memset(rbp, 0, sizeof(struct bootp_t));

    if (dhcp_msg_type == DHCPDISCOVER) {
        if (preq_addr) {
            bc = request_addr(preq_addr, client_ethaddr);
            if (bc) {
                daddr.sin_addr = *preq_addr;
            }
        }
        if (!bc) {
         new_addr:
            bc = get_new_addr(&daddr.sin_addr, client_ethaddr);
            if (!bc) {
                dprintf("no address left\n");
                return;
            }
        }
        memcpy(bc->macaddr, client_ethaddr, 6);
    } else if (preq_addr) {
        bc = request_addr(preq_addr, client_ethaddr);
        if (bc) {
            daddr.sin_addr = *preq_addr;
            memcpy(bc->macaddr, client_ethaddr, 6);
        } else {
            daddr.sin_addr.s_addr = 0;
        }
    } else {
        bc = find_addr(&daddr.sin_addr, bp->bp_hwaddr);
        if (!bc) {
            /* if never assigned, behaves as if it was already
               assigned (windows fix because it remembers its address) */
            goto new_addr;
        }
    }

    saddr.sin_addr.s_addr = htonl(ntohl(special_addr.s_addr) | CTL_ALIAS);
    saddr.sin_port = htons(BOOTP_SERVER);

    daddr.sin_port = htons(BOOTP_CLIENT);

    rbp->bp_op = BOOTP_REPLY;
    rbp->bp_xid = bp->bp_xid;
    rbp->bp_htype = 1;
    rbp->bp_hlen = 6;
    memcpy(rbp->bp_hwaddr, bp->bp_hwaddr, 6);

    rbp->bp_yiaddr = daddr.sin_addr; /* Client IP address */
    rbp->bp_siaddr = saddr.sin_addr; /* Server IP address */

    q = rbp->bp_vend;
    memcpy(q, rfc1533_cookie, 4);
    q += 4;

    if (bc) {
        dprintf("%s addr=%08x\n",
                (dhcp_msg_type == DHCPDISCOVER) ? "offered" : "ack'ed",
                ntohl(daddr.sin_addr.s_addr));

        if (dhcp_msg_type == DHCPDISCOVER) {
            *q++ = RFC2132_MSG_TYPE;
            *q++ = 1;
            *q++ = DHCPOFFER;
        } else /* DHCPREQUEST */ {
            *q++ = RFC2132_MSG_TYPE;
            *q++ = 1;
            *q++ = DHCPACK;
        }

        if (bootp_filename)
            snprintf((char *)rbp->bp_file, sizeof(rbp->bp_file), "%s",
                     bootp_filename);

        *q++ = RFC2132_SRV_ID;
        *q++ = 4;
        memcpy(q, &saddr.sin_addr, 4);
        q += 4;

        *q++ = RFC1533_NETMASK;
        *q++ = 4;
        *q++ = 0xff;
        *q++ = 0xff;
        *q++ = 0xff;
        *q++ = 0x00;

        if (!slirp_restrict) {
            *q++ = RFC1533_GATEWAY;
            *q++ = 4;
            memcpy(q, &saddr.sin_addr, 4);
            q += 4;

            *q++ = RFC1533_DNS;
            *q++ = 4;
            dns_addr.s_addr = htonl(ntohl(special_addr.s_addr) | CTL_DNS);
            memcpy(q, &dns_addr, 4);
            q += 4;
        }

        *q++ = RFC2132_LEASE_TIME;
        *q++ = 4;
        val = htonl(LEASE_TIME);
        memcpy(q, &val, 4);
        q += 4;

        if (*slirp_hostname) {
            val = strlen(slirp_hostname);
            *q++ = RFC1533_HOSTNAME;
            *q++ = val;
            memcpy(q, slirp_hostname, val);
            q += val;
        }
    } else {
        static const char nak_msg[] = "requested address not available";

        dprintf("nak'ed addr=%08x\n", ntohl(preq_addr->s_addr));

        *q++ = RFC2132_MSG_TYPE;
        *q++ = 1;
        *q++ = DHCPNAK;

        *q++ = RFC2132_MESSAGE;
        *q++ = sizeof(nak_msg) - 1;
        memcpy(q, nak_msg, sizeof(nak_msg) - 1);
        q += sizeof(nak_msg) - 1;
    }
    *q++ = RFC1533_END;

    daddr.sin_addr.s_addr = 0xffffffffu;

    m->m_len = sizeof(struct bootp_t) -
        sizeof(struct ip) - sizeof(struct udphdr);
    udp_output2(NULL, m, &saddr, &daddr, IPTOS_LOWDELAY);
}

void bootp_input(struct mbuf *m)
{
    struct bootp_t *bp = mtod(m, struct bootp_t *);

    if (bp->bp_op == BOOTP_REQUEST) {
        bootp_reply(bp);
    }
}
