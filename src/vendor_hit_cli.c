#include "../include/vendor_hit_cli.h"
#include <stdlib.h>
#include <dk_buttons_and_leds.h>
#include "../../../../../ncs/v2.5.0/nrf/subsys/bluetooth/mesh/model_utils.h"


/* Decode the data that is sent with the message acknowledgement from the server.*/
static int vendor_hit_decode_status(struct net_buf_simple *buf,
			  struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Hit_Client: vendor_hit_decode_status\n");

	status->player_id = net_buf_simple_pull_u8(buf);
	printk("\tplayer_id: %d\n", status->player_id);
	status->player_lifecount = net_buf_simple_pull_u8(buf);
	printk("\tplayer_lifecount: %d\n", status->player_lifecount);

    return 0;
}

/* Callback that handles the status response from the server.*/
static int vendor_hit_status_cb(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf)
{
	//printk("Hit_Client: vendor_hit_status_cb\n");
	printk("Hit_Client: Status Message received\n");
	if (buf->len > BT_MESH_VENDOR_HIT_MSG_STATUS_MAX_LEN ) {
		printk("buf->len too long: %d\n", buf->len);
		//return -EMSGSIZE;
	}

	struct bt_mesh_vendor_hit_cli *cli = model->user_data;
	struct bt_mesh_vendor_playerstatus_status status;
	struct bt_mesh_vendor_playerstatus_status *rsp;
	int err;

	err = vendor_hit_decode_status(buf, &status);
	if (err) {
		return err;
	} else {
		//printk("Hit_Client: vendor_hit_status_cb - SUCCESS\n");
	}

	if (bt_mesh_msg_ack_ctx_match(&cli->ack_ctx, BT_MESH_VENDOR_HIT_STATUS_OP, ctx->addr,
				      (void **)&rsp)) {
		*rsp = status;
		bt_mesh_msg_ack_ctx_rx(&cli->ack_ctx);
		//printk("Hit_Client: vendor_hit_status_cb - Marked Acknowledged\n");
	}

	if (cli->handlers->status) {
		cli->handlers->status(cli, ctx, &status);
	}

	return 0;
}

/* Define opcodes */
const struct bt_mesh_model_op _vendor_hit_cli_opcode_list[] = {
	{
		BT_MESH_VENDOR_HIT_STATUS_OP,
		BT_MESH_LEN_MIN(BT_MESH_VENDOR_HIT_MSG_STATUS_MIN_LEN),
		vendor_hit_status_cb,
	},
	BT_MESH_MODEL_OP_END,
};

/* Reset model */
static void bt_mesh_vendor_hit_cli_reset(struct bt_mesh_model *model)
{
	printk("Hit_Client: bt_mesh_vendor_hit_cli_reset\n");
	struct bt_mesh_vendor_hit_cli *cli = model->user_data;

	net_buf_simple_reset(cli->pub.msg);
	bt_mesh_msg_ack_ctx_reset(&cli->ack_ctx);
}

/* Initialize model */
static int bt_mesh_vendor_hit_cli_init(struct bt_mesh_model *model)
{
	printk("Hit_Client: bt_mesh_vendor_hit_cli_init\n");
	struct bt_mesh_vendor_hit_cli *cli = model->user_data;

	cli->model = model;
	cli->pub.msg = &cli->pub_buf;
	net_buf_simple_init_with_data(&cli->pub_buf, cli->pub_data,
				      sizeof(cli->pub_data));
	bt_mesh_msg_ack_ctx_init(&cli->ack_ctx);

	return 0;
}

/* Callbacks for the hit client model */
const struct bt_mesh_model_cb _bt_mesh_vendor_hit_cli_cb = {
	.init = bt_mesh_vendor_hit_cli_init,
	.reset = bt_mesh_vendor_hit_cli_reset,
};

/* Send set message to the server model. */
int bt_mesh_vendor_hit_cli_set(struct bt_mesh_vendor_hit_cli *cli,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_hit_set *set,
			  struct bt_mesh_vendor_playerstatus_status *rsp)
{
	printk("Hit_Client: Communicate Hit... Player ID: %d Player Lifecount: %d\n", set->player_id, set->player_lifecount);
    if (!set->reuse_transaction) {
		cli->tid++;
	}

	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_HIT_SET_OP,
				 BT_MESH_VENDOR_HIT_MSG_SET_MAX_LEN);
	bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_HIT_SET_OP);
	//printk("Message OP: %02X\n", BT_MESH_VENDOR_HIT_SET_OP);
	
 	net_buf_simple_add_u8(&msg, set->player_id);
	net_buf_simple_add_u8(&msg, set->player_lifecount);
	net_buf_simple_add_u8(&msg, cli->tid);


	struct bt_mesh_msg_rsp_ctx rsp_ctx = {
		.ack = &cli->ack_ctx,
		.op = BT_MESH_VENDOR_HIT_STATUS_OP,
		.user_data = rsp,
		.timeout = model_ackd_timeout_get(cli->model, ctx),
	};
	//printk("Response payload: %d\n", rsp);
	//printk("Expected response OP: %02X\n", BT_MESH_VENDOR_HIT_STATUS_OP);

	return bt_mesh_msg_ackd_send(cli->model, ctx, &msg, rsp ? &rsp_ctx : NULL);
}