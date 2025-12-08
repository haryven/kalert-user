#include <stdio.h>
#include <libkalert/libkalert.h>

static int msg_count;

void parse_notify_message(struct kalert_message *reply)
{
	struct kalert_notify_msg *notify;

	notify = NLMSG_DATA(&reply->nlh);
	if (kalert_notify_valid(notify))
		return;

	switch (notify->type) {
	case KALERT_NOTIFY_GEN:
	case KALERT_NOTIFY_MEM:
	case KALERT_NOTIFY_IO:
	case KALERT_NOTIFY_FS:
	case KALERT_NOTIFY_SCHED:
	case KALERT_NOTIFY_NET:
	case KALERT_NOTIFY_RAS:
	case KALERT_NOTIFY_VIRT:
	case KALERT_NOTIFY_SEC: {
		printf("Received message %d,type = %s,level = %s, event=%s\n",
		       msg_count++, kalert_type_str[notify->type],
		       kalert_level_str[notify->level],
		       kalert_event_name(notify->event));
		break;
	}
	default:
		printf("Received message unknow %d\n", notify->type);
	}
}

int main()
{
	/* See include/uapi/linux/kalert.h
	 * subscribe KALERT_GEN_SOFTLOCKUP,KALERT_MEM_ALLOCFAIL,KALERT_FS_EXT4_ERR separately
	 */
	int ids[] = { KALERT_GEN_SOFTLOCKUP, KALERT_MEM_LEAK,
		      KALERT_FS_EXT4_ERR };
	struct kalert_message reply;
	int rc;
	int sock_fd;

	rc = kalert_open();
	if (rc < 0) {
		printf("failed to open kalert channel\n");
		return -1;
	}

	sock_fd = rc;

	/* subscribe events by type mask */
#if 0
	rc = kalert_subscribe_type(sock_fd, KALERT_NOTIFY_MEM | KALERT_NOTIFY_GEN, KALERT_WARN);

#else
	/* subscribe events by event list */
	rc = kalert_subscribe_event(sock_fd, ids, KALERT_WARN);
#endif
	if (rc < 0) {
		printf("failed to subscribe kernel fault event\n");
	}

	printf("Create subscriber successed\n");

	while (1) {
		rc = kalert_get_reply(sock_fd, &reply, GET_REPLY_BLOCKING, 0);
		if (rc > 0)
			parse_notify_message(&reply);
	}
	return 0;
}
