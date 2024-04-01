#include "../include/vendor_game_modification_cli.h"
#include <stdlib.h>
#include <dk_buttons_and_leds.h>
#include "../../../../../ncs/v2.5.0/nrf/subsys/bluetooth/mesh/model_utils.h"

//TODO Add the player information to a central database.

//TODO Decodes the "payload" of the status message returned by the server model.
static int vendor_game_modification_decode_status(struct net_buf_simple *buf,
			  struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("vendor_game_modification_decode_status\n");

	//TODO: Add or check these values with player database.
	uint8_t running = net_buf_simple_pull_u8(buf);
	uint8_t init_game_delay = net_buf_simple_pull_u8(buf);
	uint16_t init_game_time = net_buf_simple_pull_le16(buf);
	
	//status->player_lifecount = net_buf_simple_pull_u8(buf);

	return 0;
}
//NOTE CB handling the status message from the server model.
static int vendor_game_modification_status_cb(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf)
{
	//printk("vendor_game_modification_status_cb\n");
	if (buf->len >= BT_MESH_VENDOR_GAME_MODIFICATION_MSG_STATUS_MAX_LEN ) {
		printk("buf->len too long\n");
		printk("%d\n", buf->len);
		return -EMSGSIZE;
	}

	struct bt_mesh_vendor_game_modification_cli *cli = model->user_data;
	struct bt_mesh_vendor_playerstatus_status status;
	struct bt_mesh_vendor_playerstatus_status *rsp;
	int err;

	err = vendor_game_modification_decode_status(buf, &status);
	if (err) {
		return err;
	} else {
		//printk("vendor_game_modification_status_cb - SUCCESS\n");
	}

	if (bt_mesh_msg_ack_ctx_match(&cli->ack_ctx, BT_MESH_VENDOR_GAME_MODIFICATION_STATUS_OP, ctx->addr,
				      (void **)&rsp)) {
		*rsp = status;
		bt_mesh_msg_ack_ctx_rx(&cli->ack_ctx);
		//printk("vendor_game_modification_status_cb - Marked Acknowledged\n");
	}

	if (cli->handlers->status) {
		cli->handlers->status(cli, ctx, &status);
	}

	return 0;
}

const struct bt_mesh_model_op _vendor_game_modification_cli_opcode_list[] = {
	{
		BT_MESH_VENDOR_GAME_MODIFICATION_STATUS_OP,
		BT_MESH_LEN_MIN(BT_MESH_VENDOR_GAME_MODIFICATION_MSG_STATUS_MIN_LEN),
		vendor_game_modification_status_cb,
	},
	BT_MESH_MODEL_OP_END,
};

static void bt_mesh_vendor_game_modification_cli_reset(struct bt_mesh_model *model)
{
	//printk("bt_mesh_vendor_game_modification_cli_reset\n");
	struct bt_mesh_vendor_game_modification_cli *cli = model->user_data;

	net_buf_simple_reset(cli->pub.msg);
	bt_mesh_msg_ack_ctx_reset(&cli->ack_ctx);
}


static int bt_mesh_vendor_game_modification_cli_init(struct bt_mesh_model *model)
{
	//printk("bt_mesh_vendor_game_modification_cli_init\n");
	struct bt_mesh_vendor_game_modification_cli *cli = model->user_data;

	cli->model = model;
	cli->pub.msg = &cli->pub_buf;
	net_buf_simple_init_with_data(&cli->pub_buf, cli->pub_data,
				      sizeof(cli->pub_data));
	bt_mesh_msg_ack_ctx_init(&cli->ack_ctx);

	return 0;
}

const struct bt_mesh_model_cb _bt_mesh_vendor_game_modification_cli_cb = {
	.init = bt_mesh_vendor_game_modification_cli_init,
	.reset = bt_mesh_vendor_game_modification_cli_reset,
};


int bt_mesh_vendor_game_modification_cli_set(struct bt_mesh_vendor_game_modification_cli *cli,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_game_modification_set *set,
			  struct bt_mesh_vendor_playerstatus_status *rsp)
{
	//printk("bt_mesh_vendor_game_modification_cli_set\n");
	if (!set->reuse_transaction) {
		cli->tid++;
	}
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_GAME_MODIFICATION_SET_OP,
				 BT_MESH_VENDOR_GAME_MODIFICATION_MSG_SET_MAX_LEN);
	bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_GAME_MODIFICATION_SET_OP);

	net_buf_simple_add_u8(&msg, set->running);
	net_buf_simple_add_u8(&msg, set->init_game_delay);
	net_buf_simple_add_le16(&msg, set->init_game_time);
	net_buf_simple_add_u8(&msg, cli->tid);

	struct bt_mesh_msg_rsp_ctx rsp_ctx = {
		.ack = &cli->ack_ctx,
		.op = BT_MESH_VENDOR_GAME_MODIFICATION_STATUS_OP,
		.user_data = rsp,
		.timeout = model_ackd_timeout_get(cli->model, ctx),
	};

	return bt_mesh_msg_ackd_send(cli->model, ctx, &msg, rsp ? &rsp_ctx : NULL);
}

