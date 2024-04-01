#include "../include/vendor_test_model_srv.h"
#include <stdlib.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/bluetooth/mesh/msg.h>
#include "../../../../ncs/v2.5.0/nrf/subsys/bluetooth/mesh/model_utils.h"


//NOTE Initializes the buffer and adds the message content.
static void vendor_test_model_encode_status(struct net_buf_simple *buf,
			  const struct bt_mesh_vendor_playerstatus_status *status)
{
	printk("vendor_test_model_encode_status\n");
	bt_mesh_model_msg_init(buf, BT_MESH_VENDOR_TEST_MODEL_STATUS_OP);

	//Some debugging
	printk("Sending status message..\n Player ID: %d, Status: %d\n", status->player_id, status->player_lifecount);

    net_buf_simple_add_u8(buf, status->player_id);
    net_buf_simple_add_u8(buf, status->player_lifecount);
}

//NOTE Defines the message buffer, calls functions to fill it with content and sends the message.
static void vendor_test_model_rsp_status(struct bt_mesh_model *model,
		       struct bt_mesh_msg_ctx *rx_ctx,
		       const struct bt_mesh_vendor_playerstatus_status *status)
{
	printk("vendor_test_model_rsp_status\n");
	BT_MESH_MODEL_BUF_DEFINE(reply, BT_MESH_VENDOR_TEST_MODEL_STATUS_OP, BT_MESH_VENDOR_TEST_MODEL_MSG_STATUS_MAX_LEN);
	vendor_test_model_encode_status(&reply, status);

	(void) bt_mesh_model_send(model, rx_ctx, &reply, NULL, NULL);
}

//TODO
static int vendor_test_model_handle_get(struct bt_mesh_model *model,
                               struct bt_mesh_msg_ctx *ctx,
                               struct net_buf_simple *buf)
{
	printk("vendor_test_model_handle_get\n");
    //struct bt_mesh_vendor_test_model_srv *srv = model->user_data;
	
	//int err;
	//TODO Uncommenting below line will cause the programm to crash with a hard fault.
	//srv->handlers->get(srv, ctx, &player_status);

	//err = dk_set_led(DK_LED1, player_status.player_lifecount);

	vendor_test_model_rsp_status(model, ctx, &local_player_status);

	return 0;
}
//NOTE Sets the on/off status of the LED
static int vendor_test_model_onoff_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		      struct net_buf_simple *buf, bool ack)
{
	printk("vendor_test_model_onoff_set\n");
	if (buf->len != BT_MESH_VENDOR_TEST_MODEL_MSG_SET_MIN_LEN &&
	    buf->len != BT_MESH_VENDOR_TEST_MODEL_MSG_SET_MAX_LEN) {
		printk("buf len out of bounds\n");
		printk("%d\n", buf->len);
		//return -EMSGSIZE;
	}
	printk("Buffer length: %d\n", buf->len);

	struct bt_mesh_vendor_test_model_srv *srv = model->user_data;
	struct bt_mesh_model_transition transition;
	struct bt_mesh_vendor_test_model_set set;

	uint8_t on_off = net_buf_simple_pull_u8(buf);	
	uint8_t tid = net_buf_simple_pull_u8(buf);

	printk("on_off: %d\n", on_off);
	printk("tid: %d\n", tid);
	

	if (on_off > 1) {
		return -EINVAL;
	}

	set.on_off = on_off;
	local_player_status.player_lifecount = on_off;
	printk("player_status.player_id: %d\n", local_player_status.player_id);
	printk("player_status.player_lifecount: %d\n", local_player_status.player_lifecount);

	if (tid_check_and_update(&srv->prev_transaction, tid, ctx) != 0) {
		/* If this is the same transaction, we don't need to send it
		 * to the app, but we still have to respond with a status.
		 */
		srv->handlers->get(srv, NULL, &local_player_status);
		goto respond;
	}

	if (buf->len == 2) {
		model_transition_buf_pull(buf, &transition);
		set.transition = &transition;
	} else if (!atomic_test_bit(&srv->flags, GEN_VENDOR_TEST_MODEL_SRV_NO_DTT)) {
		bt_mesh_dtt_srv_transition_get(srv->model, &transition);
		set.transition = &transition;
	} else {
		set.transition = NULL;
	}

	srv->handlers->set(srv, ctx, &set, &local_player_status);

	(void)vendor_test_model_srv_pub(srv, NULL, &local_player_status);

respond:
	if (ack) {
		vendor_test_model_rsp_status(model, ctx, &local_player_status);
	}

	return 0;
}
//NOTE Handles the set message
static int vendor_test_model_handle_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		      struct net_buf_simple *buf)
{
	printk("vendor_test_model_handle_set\n");
	return vendor_test_model_onoff_set(model, ctx, buf, true);
}
static int vendor_test_model_handle_set_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
			    struct net_buf_simple *buf)
{
	printk("vendor_test_model_handle_set_unack\n");
	return vendor_test_model_onoff_set(model, ctx, buf, false);
}

static int vendor_test_model_update_handler(struct bt_mesh_model *model)
{
	printk("vendor_test_model_update_handler\n");
	//int err;
	//struct bt_mesh_vendor_test_model_srv *srv = model->user_data;


	//TODO Uncommenting below line will cause the programm to crash with a hard fault.
	//srv->handlers->get(srv, NULL, &test_model_player_status);
	vendor_test_model_encode_status(model->pub->msg, &local_player_status);

	//err = dk_set_led(DK_LED1, test_model_player_status.player_lifecount);
	printk("Player ID: %d, Status: %d\n", local_player_status.player_id, local_player_status.player_lifecount);

	return 0;
}

static void vendor_test_model_srv_reset(struct bt_mesh_model *model)
{
	printk("vendor_test_model_srv_reset\n");
	net_buf_simple_reset(model->pub->msg);
}


static int vendor_test_model_srv_init(struct bt_mesh_model *model)
{
	printk("vendor_test_model_srv_init\n");
	int err;
	struct bt_mesh_vendor_test_model_srv *srv = model->user_data;
	
	srv->model = model;
	srv->pub.msg = &srv->pub_buf;
	srv->pub.update = vendor_test_model_update_handler;
	//TODO Model does not automatically start broadcasting after adding a application key. Why?
	//srv->pub.period = 2;
	//Publishing to the broadcast address (all nodes)
	//srv->pub.addr = 0xFFFF;
	//Message will not be relayed
	//srv->pub.ttl = 0;
	//srv->pub.retransmit = 0;
	net_buf_simple_init_with_data(&srv->pub_buf, srv->pub_data,
				      sizeof(srv->pub_data));


	/* NOTE
	* Initializes the relay function of the node.
	* TODO:
	* Check if the current values are optimal.
	*/
	enum bt_mesh_feat_state feat_state = BT_MESH_FEATURE_ENABLED;
	uint8_t xmit = BT_MESH_TRANSMIT(2, 10);
	err = bt_mesh_relay_set(feat_state, xmit);
	printk("bt_mesh_relay_set status code: %d", err);
	
	return 0;
}

const struct bt_mesh_model_cb _bt_mesh_vendor_test_model_srv_cb = {
	.init = vendor_test_model_srv_init,
	.reset = vendor_test_model_srv_reset,
};

int vendor_test_model_srv_pub(struct bt_mesh_vendor_test_model_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_playerstatus_status *status)
{
	printk("vendor_test_model_srv_pub\n");
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_TEST_MODEL_STATUS_OP,
				 BT_MESH_VENDOR_TEST_MODEL_MSG_STATUS_MAX_LEN);
	vendor_test_model_encode_status(&msg, status);
	return bt_mesh_msg_send(srv->model, ctx, &msg);
}

// Opcode list for this vendor model.
const struct bt_mesh_model_op _vendor_test_model_srv_opcode_list[] = {
	{ BT_MESH_VENDOR_TEST_MODEL_GET_OP, BT_MESH_VENDOR_TEST_MODEL_MSG_GET_MIN_LEN, vendor_test_model_handle_get },
	{ BT_MESH_VENDOR_TEST_MODEL_SET_OP, BT_MESH_VENDOR_TEST_MODEL_MSG_SET_MIN_LEN, vendor_test_model_handle_set },
	{ BT_MESH_VENDOR_TEST_MODEL_SET_UNACK_OP, BT_MESH_VENDOR_TEST_MODEL_MSG_SET_MIN_LEN, vendor_test_model_handle_set_unack },
    BT_MESH_MODEL_OP_END,
};