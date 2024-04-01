#include "../include/vendor_test_model_cli.h"
#include <stdlib.h>
#include <dk_buttons_and_leds.h>
#include "../../../../../ncs/v2.5.0/nrf/subsys/bluetooth/mesh/model_utils.h"


//NOTE Decodes the "payload" of the status message returned by the server model.
static int vendor_test_model_decode_status(struct net_buf_simple *buf,
			  struct bt_mesh_vendor_playerstatus_status *status)
{
	printk("vendor_test_model_decode_status\n");
	//Some testing to check for the buffer length before extracting the data.
	/*
	uint8_t buf_tmp;
	while(buf->len > 0){
		buf_tmp = net_buf_simple_pull_u8(buf);
		printk("%d\n", buf_tmp);
	*/
	printk("Status message received..\n");

	status->player_id = net_buf_simple_pull_u8(buf);
	printk("CLI - status->player_id: %d\n", status->player_id);

	status->player_lifecount = net_buf_simple_pull_u8(buf);
	printk("CLI - status->player_lifecount: %d\n", status->player_lifecount);

	//dk_set_led(0, status->player_lifecount);

	return 0;
}
//NOTE CB handling the status message from the server model.
static int vendor_test_model_status_cb(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf)
{
	printk("vendor_test_model_status_cb\n");
	if (buf->len >= BT_MESH_VENDOR_TEST_MODEL_MSG_STATUS_MAX_LEN ) {
		printk("buf->len too long\n");
		printk("%d\n", buf->len);
		return -EMSGSIZE;
	}

	struct bt_mesh_vendor_test_model_cli *cli = model->user_data;
	struct bt_mesh_vendor_playerstatus_status status;
	struct bt_mesh_vendor_playerstatus_status *rsp;
	int err;

	err = vendor_test_model_decode_status(buf, &status);
	if (err) {
		return err;
	} else {
		printk("vendor_test_model_status_cb - SUCCESS\n");
	}

	if (bt_mesh_msg_ack_ctx_match(&cli->ack_ctx, BT_MESH_VENDOR_TEST_MODEL_STATUS_OP, ctx->addr,
				      (void **)&rsp)) {
		*rsp = status;
		bt_mesh_msg_ack_ctx_rx(&cli->ack_ctx);
		printk("vendor_test_model_status_cb - Marked Acknowledged\n");
	}

	if (cli->handlers->status) {
		cli->handlers->status(cli, ctx, &status);
	}

	return 0;
}

const struct bt_mesh_model_op _vendor_test_model_cli_opcode_list[] = {
	{
		BT_MESH_VENDOR_TEST_MODEL_STATUS_OP,
		BT_MESH_LEN_MIN(BT_MESH_VENDOR_TEST_MODEL_MSG_STATUS_MIN_LEN),
		vendor_test_model_status_cb,
	},
	BT_MESH_MODEL_OP_END,
};

static void bt_mesh_vendor_test_model_cli_reset(struct bt_mesh_model *model)
{
	printk("bt_mesh_vendor_test_model_cli_reset\n");
	struct bt_mesh_vendor_test_model_cli *cli = model->user_data;

	net_buf_simple_reset(cli->pub.msg);
	bt_mesh_msg_ack_ctx_reset(&cli->ack_ctx);
}


static int bt_mesh_vendor_test_model_cli_init(struct bt_mesh_model *model)
{
	printk("bt_mesh_vendor_test_model_cli_init\n");
	struct bt_mesh_vendor_test_model_cli *cli = model->user_data;

	cli->model = model;
	cli->pub.msg = &cli->pub_buf;
	net_buf_simple_init_with_data(&cli->pub_buf, cli->pub_data,
				      sizeof(cli->pub_data));
	bt_mesh_msg_ack_ctx_init(&cli->ack_ctx);

	return 0;
}

const struct bt_mesh_model_cb _bt_mesh_vendor_test_model_cli_cb = {
	.init = bt_mesh_vendor_test_model_cli_init,
	.reset = bt_mesh_vendor_test_model_cli_reset,
};

int bt_mesh_vendor_test_model_cli_get(struct bt_mesh_vendor_test_model_cli *cli,
			  struct bt_mesh_msg_ctx *ctx,
			  struct bt_mesh_vendor_playerstatus_status *rsp)
{
	printk("bt_mesh_vendor_test_model_cli_get\n");
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_TEST_MODEL_GET_OP,
				 BT_MESH_VENDOR_TEST_MODEL_MSG_GET_MAX_LEN);
	bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_TEST_MODEL_GET_OP);

	struct bt_mesh_msg_rsp_ctx rsp_ctx = {
		.ack = &cli->ack_ctx,
		.op = BT_MESH_VENDOR_TEST_MODEL_STATUS_OP,
		.user_data = rsp,
		.timeout = model_ackd_timeout_get(cli->model, ctx),
	};

	//TODO Sending it as ackd message does not work as intended and always leads to a timeout...
	// The message is sent and responded to only after timing out first.
	// Currently this is "fixed" by sending non ackd message. Ackd probably is not necessary hear
	// as get always expects a status response.
	return bt_mesh_msg_send(cli->model, ctx, &msg);
	//return bt_mesh_msg_ackd_send(cli->model, ctx, &msg, rsp ? &rsp_ctx : NULL);
}

int bt_mesh_vendor_test_model_cli_set(struct bt_mesh_vendor_test_model_cli *cli,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_test_model_set *set,
			  struct bt_mesh_vendor_playerstatus_status *rsp)
{
	printk("bt_mesh_vendor_test_model_cli_set\n");
	if (!set->reuse_transaction) {
		cli->tid++;
	}
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_TEST_MODEL_SET_OP,
				 BT_MESH_VENDOR_TEST_MODEL_MSG_SET_MAX_LEN);
	bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_TEST_MODEL_SET_OP);

	net_buf_simple_add_u8(&msg, set->on_off);
	printk("Set status: %d\n", set->on_off);
	net_buf_simple_add_u8(&msg, cli->tid);
	if (set->transition) {
		model_transition_buf_add(&msg, set->transition);
	}

	struct bt_mesh_msg_rsp_ctx rsp_ctx = {
		.ack = &cli->ack_ctx,
		.op = BT_MESH_VENDOR_TEST_MODEL_STATUS_OP,
		.user_data = rsp,
		.timeout = model_ackd_timeout_get(cli->model, ctx),
	};

	return bt_mesh_msg_ackd_send(cli->model, ctx, &msg, rsp ? &rsp_ctx : NULL);
}

int bt_mesh_vendor_test_model_cli_set_unack(struct bt_mesh_vendor_test_model_cli *cli,
				struct bt_mesh_msg_ctx *ctx,
				const struct bt_mesh_vendor_test_model_set *set)
{
	printk("bt_mesh_vendor_test_model_cli_set_unack\n");
	if (!set->reuse_transaction) {
		cli->tid++;
	}
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_TEST_MODEL_SET_UNACK_OP,
				 BT_MESH_VENDOR_TEST_MODEL_MSG_SET_MAX_LEN);
	bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_TEST_MODEL_SET_UNACK_OP);

	net_buf_simple_add_u8(&msg, set->on_off);
	printk("Set status: %d\n", set->on_off);
	net_buf_simple_add_u8(&msg, cli->tid);
	if (set->transition) {
		model_transition_buf_add(&msg, set->transition);
	}

	return bt_mesh_msg_send(cli->model, ctx, &msg);
}
