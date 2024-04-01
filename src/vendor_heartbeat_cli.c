#include "../include/vendor_heartbeat_cli.h"
#include <stdlib.h>
#include <dk_buttons_and_leds.h>
#include "../../../../../ncs/v2.5.0/nrf/subsys/bluetooth/mesh/model_utils.h"

/* Decode the data that is sent with the message acknowledgement from the server.*/
static int vendor_heartbeat_decode_status(struct net_buf_simple *buf,
			  struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Heartbeat_Client: vendor_heartbeat_decode_status\n");

	status->player_id = net_buf_simple_pull_u8(buf);
	printk("\tplayer_id: %d\n", status->player_id);

	return 0;
}

/* Callback that handles the status response from the server.*/
static int vendor_heartbeat_status_cb(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf)
{
	//printk("Heartbeat_Client: vendor_heartbeat_status_cb\n");
	printk("Heartbeat_Client: Status Message received\n");
	if (buf->len > BT_MESH_VENDOR_HEARTBEAT_MSG_STATUS_MAX_LEN ) {
		printk("buf->len too long: %d\n", buf->len);
		//return -EMSGSIZE;
	}

	struct bt_mesh_vendor_heartbeat_cli *cli = model->user_data;
	struct bt_mesh_vendor_playerstatus_status status;
	struct bt_mesh_vendor_playerstatus_status *rsp;
	int err;

	err = vendor_heartbeat_decode_status(buf, &status);
	if (err) {
		return err;
	} else {
		//printk("Heartbeat_Client: vendor_heartbeat_status_cb - SUCCESS\n");
	}

	if (bt_mesh_msg_ack_ctx_match(&cli->ack_ctx, BT_MESH_VENDOR_HEARTBEAT_STATUS_OP, ctx->addr,
				      (void **)&rsp)) {
		*rsp = status;
		bt_mesh_msg_ack_ctx_rx(&cli->ack_ctx);
		//printk("Heartbeat_Client: vendor_heartbeat_status_cb - Marked Acknowledged\n");
	}

	if (cli->handlers->status) {
		cli->handlers->status(cli, ctx, &status);
	}

	return 0;
}

/* Define opcodes */
const struct bt_mesh_model_op _vendor_heartbeat_cli_opcode_list[] = {
	{
		BT_MESH_VENDOR_HEARTBEAT_STATUS_OP,
		BT_MESH_LEN_MIN(BT_MESH_VENDOR_HEARTBEAT_MSG_STATUS_MIN_LEN),
		vendor_heartbeat_status_cb,
	},
	BT_MESH_MODEL_OP_END,
};

/* Reset model */
static void bt_mesh_vendor_heartbeat_cli_reset(struct bt_mesh_model *model)
{
	printk("Heartbeat_Client: bt_mesh_vendor_heartbeat_cli_reset\n");
	struct bt_mesh_vendor_heartbeat_cli *cli = model->user_data;

	net_buf_simple_reset(cli->pub.msg);
	bt_mesh_msg_ack_ctx_reset(&cli->ack_ctx);
}

/* Initialize model */
static int bt_mesh_vendor_heartbeat_cli_init(struct bt_mesh_model *model)
{
	printk("Heartbeat_Client: bt_mesh_vendor_heartbeat_cli_init\n");
	struct bt_mesh_vendor_heartbeat_cli *cli = model->user_data;

	cli->model = model;
	cli->pub.msg = &cli->pub_buf;
	net_buf_simple_init_with_data(&cli->pub_buf, cli->pub_data,
				      sizeof(cli->pub_data));
	bt_mesh_msg_ack_ctx_init(&cli->ack_ctx);

	return 0;
}

/* Callbacks for the heartbeat client model */
const struct bt_mesh_model_cb _bt_mesh_vendor_heartbeat_cli_cb = {
	.init = bt_mesh_vendor_heartbeat_cli_init,
	.reset = bt_mesh_vendor_heartbeat_cli_reset,
};
