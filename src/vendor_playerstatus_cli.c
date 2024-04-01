#include "../include/vendor_playerstatus_cli.h"
#include <stdlib.h>
#include <dk_buttons_and_leds.h>
#include "../../../../../ncs/v2.5.0/nrf/subsys/bluetooth/mesh/model_utils.h"


static int vendor_playerstatus_decode_status(struct net_buf_simple *buf,
			  struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Playerstatus_Client: vendor_playerstatus_decode_status\n");

	status->player_id = net_buf_simple_pull_u8(buf);
	printk("\tplayer_id: %d\n", status->player_id);
    status->player_lifecount = net_buf_simple_pull_u8(buf);
	printk("\tplayer_lifecount: %d\n", status->player_lifecount);
	status->tank_left =  net_buf_simple_pull_u8(buf);
	printk("\ttank_left: %d/255\n", status->tank_left);
	status->tanks_since_init = net_buf_simple_pull_u8(buf);

	return 0;
}

static int vendor_playerstatus_status_cb(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf)
{
	//printk("Playerstatus_Client: vendor_playerstatus_status_cb\n");
	printk("Playerstatus_Client: Status Message received\n");
	if (buf->len > BT_MESH_VENDOR_PLAYERSTATUS_MSG_STATUS_MAX_LEN ) {
		printk("buf->len too long: %d\n", buf->len);
		return -EMSGSIZE;
	}

	struct bt_mesh_vendor_playerstatus_cli *cli = model->user_data;
	struct bt_mesh_vendor_playerstatus_status status;
	struct bt_mesh_vendor_playerstatus_status *rsp;
	int err;

	err = vendor_playerstatus_decode_status(buf, &status);
	if (err) {
		return err;
	} else {
		//printk("Playerstatus_Client: vendor_playerstatus_status_cb - SUCCESS\n");
	}

	if (bt_mesh_msg_ack_ctx_match(&cli->ack_ctx, BT_MESH_VENDOR_PLAYERSTATUS_STATUS_OP, ctx->addr,
				      (void **)&rsp)) {
		*rsp = status;
		bt_mesh_msg_ack_ctx_rx(&cli->ack_ctx);
		//printk("Playerstatus_Client: vendor_playerstatus_status_cb - Marked Acknowledged\n");
	}

	if (cli->handlers->status) {
		cli->handlers->status(cli, ctx, &status);
	}

	return 0;
}

const struct bt_mesh_model_op _vendor_playerstatus_cli_opcode_list[] = {
	{
		BT_MESH_VENDOR_PLAYERSTATUS_STATUS_OP,
		BT_MESH_LEN_MIN(BT_MESH_VENDOR_PLAYERSTATUS_MSG_STATUS_MIN_LEN),
		vendor_playerstatus_status_cb,
	},
	BT_MESH_MODEL_OP_END,
};

static void bt_mesh_vendor_playerstatus_cli_reset(struct bt_mesh_model *model)
{
	printk("Playerstatus_Client: bt_mesh_vendor_playerstatus_cli_reset\n");
	struct bt_mesh_vendor_playerstatus_cli *cli = model->user_data;

	net_buf_simple_reset(cli->pub.msg);
	bt_mesh_msg_ack_ctx_reset(&cli->ack_ctx);
}


static int bt_mesh_vendor_playerstatus_cli_init(struct bt_mesh_model *model)
{
	printk("Playerstatus_Client: bt_mesh_vendor_playerstatus_cli_init\n");
	struct bt_mesh_vendor_playerstatus_cli *cli = model->user_data;

	cli->model = model;
	cli->pub.msg = &cli->pub_buf;
	net_buf_simple_init_with_data(&cli->pub_buf, cli->pub_data,
				      sizeof(cli->pub_data));
	bt_mesh_msg_ack_ctx_init(&cli->ack_ctx);

	return 0;
}

const struct bt_mesh_model_cb _bt_mesh_vendor_playerstatus_cli_cb = {
	.init = bt_mesh_vendor_playerstatus_cli_init,
	.reset = bt_mesh_vendor_playerstatus_cli_reset,
};

int bt_mesh_vendor_playerstatus_cli_get(struct bt_mesh_vendor_playerstatus_cli *cli,
			  struct bt_mesh_msg_ctx *ctx,
			  struct bt_mesh_vendor_playerstatus_status *rsp)
{
	//printk("Playerstatus_Client: bt_mesh_vendor_playerstatus_cli_get\n");
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_PLAYERSTATUS_GET_OP,
				 BT_MESH_VENDOR_PLAYERSTATUS_MSG_GET_MAX_LEN);
	bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_PLAYERSTATUS_GET_OP);

	struct bt_mesh_msg_rsp_ctx rsp_ctx = {
		.ack = &cli->ack_ctx,
		.op = BT_MESH_VENDOR_PLAYERSTATUS_STATUS_OP,
		.user_data = rsp,
		.timeout = model_ackd_timeout_get(cli->model, ctx),
	};

	return bt_mesh_msg_send(cli->model, ctx, &msg);
}
