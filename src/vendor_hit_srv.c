#include "../include/vendor_hit_srv.h"
#include <stdlib.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/bluetooth/mesh/msg.h>
#include "../../../../ncs/v2.5.0/nrf/subsys/bluetooth/mesh/model_utils.h"

/* Encode the data that is sent with the message acknowledgement to the client. */
static void vendor_hit_encode_status(struct net_buf_simple *buf,
			  const struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Hit_Server: vendor_hit_encode_status\n");
	bt_mesh_model_msg_init(buf, BT_MESH_VENDOR_HIT_STATUS_OP);

	//Some debugging
	// TODO: Remove this
	printk("Hit_Server: Sending status message..\n\tplayer_id: %d, Lifecount: %d\n", status->player_id, status->player_lifecount);

    net_buf_simple_add_u8(buf, status->player_id);
    net_buf_simple_add_u8(buf, status->player_lifecount);
}

/* Handle the response status message to the client. */
static void vendor_hit_rsp_status(struct bt_mesh_model *model,
		       struct bt_mesh_msg_ctx *rx_ctx,
		       const struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Hit_Server: vendor_hit_rsp_status\n");
	BT_MESH_MODEL_BUF_DEFINE(reply, BT_MESH_VENDOR_HIT_STATUS_OP, BT_MESH_VENDOR_HIT_MSG_STATUS_MAX_LEN);
	vendor_hit_encode_status(&reply, status);

	(void) bt_mesh_model_send(model, rx_ctx, &reply, NULL, NULL);
}

/* Handles the get request from the client. */
static int vendor_hit_handle_get(struct bt_mesh_model *model,
                               struct bt_mesh_msg_ctx *ctx,
                               struct net_buf_simple *buf)
{
	//printk("Hit_Server: vendor_hit_handle_get\n");
    //struct bt_mesh_vendor_test_model_srv *srv = model->user_data;
	
	//TODO Uncommenting below line will cause the programm to crash with a hard fault.
	//srv->handlers->get(srv, ctx, &player_status);

	vendor_hit_rsp_status(model, ctx, &local_player_status);

	return 0;
}

/* Get the data from the set message and transfer it to the application. */
static int vendor_hit_update_lifecount_self(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		      struct net_buf_simple *buf, bool ack)
{
	//printk("Hit_Server: vendor_hit_update_lifecount_self\n");
	if (buf->len != BT_MESH_VENDOR_HIT_MSG_SET_MIN_LEN &&
	    buf->len != BT_MESH_VENDOR_HIT_MSG_SET_MAX_LEN) {
		printk("buf->len too long: %d\n", buf->len);
		//return -EMSGSIZE;
	}

	struct bt_mesh_vendor_hit_srv *srv = model->user_data;
	struct bt_mesh_vendor_hit_set set;

	uint8_t player_id = net_buf_simple_pull_u8(buf);	
	uint8_t player_lifecount = net_buf_simple_pull_u8(buf);	
	uint8_t tid = net_buf_simple_pull_u8(buf);

	//printk("Player_id: %d ", player_id);	
	//printk("Lifecount: %d\n", player_lifecount);	
	//printk("tid: %d\n", tid);

	set.player_lifecount = player_lifecount;
	set.player_id = player_id;

	if (tid_check_and_update(&srv->prev_transaction, tid, ctx) != 0) {
		// If this is the same transaction, we don't need to send it
		// to the app, but we still have to respond with a status.
		//
		srv->handlers->get(srv, NULL, &local_player_status);
		goto respond;
	}
    
	srv->handlers->set(srv, ctx, &set, &local_player_status);
	// TODO Check if below code is necessary.
	//(void)vendor_hit_srv_pub(srv, NULL, &local_player_status);
	respond:
		if (ack) {
			vendor_hit_rsp_status(model, ctx, &local_player_status);
		}

	return 0;
}
/* Handle the set message from the client*/
static int vendor_hit_handle_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		      struct net_buf_simple *buf)
{
	//printk("Hit_Server: vendor_hit_handle_set\n");
	return vendor_hit_update_lifecount_self(model, ctx, buf, true);
}

/* Reset the model */
static void vendor_hit_srv_reset(struct bt_mesh_model *model)
{
	printk("Hit_Server: vendor_hit_srv_reset\n");
	net_buf_simple_reset(model->pub->msg);
}

/* Initialize the model */
static int vendor_hit_srv_init(struct bt_mesh_model *model)
{
	printk("Hit_Server: vendor_hit_srv_init\n");
	//int err;
	struct bt_mesh_vendor_hit_srv *srv = model->user_data;
	
	srv->model = model;
	srv->pub.msg = &srv->pub_buf;
	net_buf_simple_init_with_data(&srv->pub_buf, srv->pub_data,
				      sizeof(srv->pub_data));
	return 0;
}

/* Model callbacks */
const struct bt_mesh_model_cb _bt_mesh_vendor_hit_srv_cb = {
	.init = vendor_hit_srv_init,
	.reset = vendor_hit_srv_reset,
};

/* Publish status message */
int vendor_hit_srv_pub(struct bt_mesh_vendor_hit_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Hit_Server: vendor_hit_srv_pub\n");
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_HIT_STATUS_OP,
				 BT_MESH_VENDOR_HIT_MSG_STATUS_MAX_LEN);
	vendor_hit_encode_status(&msg, status);
	return bt_mesh_msg_send(srv->model, ctx, &msg);
}

/* Opcode list */
const struct bt_mesh_model_op _vendor_hit_srv_opcode_list[] = {
	{ BT_MESH_VENDOR_HIT_GET_OP, BT_MESH_VENDOR_HIT_MSG_GET_MIN_LEN, vendor_hit_handle_get },
	{ BT_MESH_VENDOR_HIT_SET_OP, BT_MESH_VENDOR_HIT_MSG_SET_MIN_LEN, vendor_hit_handle_set },
    BT_MESH_MODEL_OP_END,
};