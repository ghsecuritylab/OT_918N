
#include <string.h>
#include <net/if.h>
#include <linux/veth.h>

#include "utils.h"
#include "ip_common.h"

static void usage(void)
{
	printf("Usage: ip link <options> type veth "
	       "[peer <options>]\nTo get <options> type "
	       "'ip link add help'\n");
}

static int veth_parse_opt(struct link_util *lu, int argc, char **argv,
			  struct nlmsghdr *hdr)
{
	char *name, *type, *link, *dev;
	int err, len;
	struct rtattr * data;

	if (strcmp(argv[0], "peer") != 0) {
		usage();
		return -1;
	}

	data = NLMSG_TAIL(hdr);
	addattr_l(hdr, 1024, VETH_INFO_PEER, NULL, 0);

	hdr->nlmsg_len += sizeof(struct ifinfomsg);

	err = iplink_parse(argc - 1, argv + 1, (struct iplink_req *)hdr,
			   &name, &type, &link, &dev);
	if (err < 0)
		return err;

	if (name) {
		len = strlen(name) + 1;
		if (len > IFNAMSIZ)
			invarg("\"name\" too long\n", *argv);
		addattr_l(hdr, 1024, IFLA_IFNAME, name, len);
	}

	data->rta_len = (void *)NLMSG_TAIL(hdr) - (void *)data;
	return argc - 1 - err;
}

struct link_util veth_link_util = {
	.id = "veth",
	.parse_opt = veth_parse_opt,
};
