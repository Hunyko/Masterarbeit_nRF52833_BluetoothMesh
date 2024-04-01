#include "../include/vendor_heartbeat_srv.h"
#include <stdlib.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/bluetooth/mesh/msg.h>
#include "../../../../ncs/v2.5.0/nrf/subsys/bluetooth/mesh/model_utils.h"


/* Encode the data that is sent with the message acknowledgement to the client. */
static void vendor_heartbeat_encode_status(struct net_buf_simple *buf,
			  const struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Heartbeat_Server: vendor_heartbeat_encode_status\n");
	bt_mesh_model_msg_init(buf, BT_MESH_VENDOR_HEARTBEAT_STATUS_OP);

	//Some debugging
	// TODO: Remove this
	printk("Heartbeat_Server: Sending status message..\n\tplayer_id: %d\n", status->player_id);

    net_buf_simple_add_u8(buf, status->player_id);
}

/* Handle the response status message to the client. */
static void vendor_heartbeat_rsp_status(struct bt_mesh_model *model,
		       struct bt_mesh_msg_ctx *rx_ctx,
		       const struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Heartbeat_Server: vendor_heartbeat_rsp_status\n");
	BT_MESH_MODEL_BUF_DEFINE(reply, BT_MESH_VENDOR_HEARTBEAT_STATUS_OP, BT_MESH_VENDOR_HEARTBEAT_MSG_STATUS_MAX_LEN);
	vendor_heartbeat_encode_status(&reply, status);

	(void) bt_mesh_model_send(model, rx_ctx, &reply, NULL, NULL);
}

/* Handle the regular status update broadcast */
static int vendor_heartbeat_update_handler(struct bt_mesh_model *model)
{
	//printk("Heartbeat_Server: vendor_heartbeat_update_handler\n");
	//int err;
	struct bt_mesh_vendor_heartbeat_srv *srv = model->user_data;


	//TODO Uncommenting below line will cause the programm to crash with a hard fault.
	srv->handlers->get(srv, NULL, NULL);
	vendor_heartbeat_encode_status(model->pub->msg, &local_player_status);

	return 0;
}

/* Reset the server model */
static void vendor_heartbeat_srv_reset(struct bt_mesh_model *model)
{
	printk("Heartbeat_Server: vendor_heartbeat_srv_reset\n");
	net_buf_simple_reset(model->pub->msg);
}

/* Initialize the server model */
static int vendor_heartbeat_srv_init(struct bt_mesh_model *model)
{
	printk("Heartbeat_Server: vendor_heartbeat_srv_init\n");
	//int err;
	struct bt_mesh_vendor_heartbeat_srv *srv = model->user_data;
	
	srv->model = model;
	srv->pub.msg = &srv->pub_buf;
	//TODO Check  publication settings.
	// Model does not automatically start broadcasting after adding an application key.
	srv->pub.update = vendor_heartbeat_update_handler;
	// srv->pub.period = 2;
	// //Publish to all relay nodes
	// srv->pub.addr = 0xFFFE;
	// //Message is not to be relayed&retransmitted
	// srv->pub.ttl = 0;
	// srv->pub.retransmit = 0;
	net_buf_simple_init_with_data(&srv->pub_buf, srv->pub_data, sizeof(srv->pub_data));
	
	/*
	TODO
	Check how to init the periodic publishing*/
	/*
	model->pub->msg = &srv->pub_buf;				  
	model->pub->update = vendor_heartbeat_update_handler;
	model->pub->period = 2;
	model->pub->addr = 0xFFFE;
	model->pub->ttl = 0;
	model->pub->retransmit = 0;
	*/
	
	return 0;
}

/* Model callbacks */
const struct bt_mesh_model_cb _bt_mesh_vendor_heartbeat_srv_cb = {
	.init = vendor_heartbeat_srv_init,
	.reset = vendor_heartbeat_srv_reset,
};

/* Send status message to the client */
int vendor_heartbeat_srv_pub(struct bt_mesh_vendor_heartbeat_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Heartbeat_Server: vendor_heartbeat_srv_pub\n");
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_HEARTBEAT_STATUS_OP,
				 BT_MESH_VENDOR_HEARTBEAT_MSG_STATUS_MAX_LEN);
	vendor_heartbeat_encode_status(&msg, status);
	return bt_mesh_msg_send(srv->model, ctx, &msg);
}


// TODO: Code below is not needed.
#if 0
/* Opcode list */
const struct bt_mesh_model_op _vendor_heartbeat_srv_opcode_list[] = {
	{ BT_MESH_VENDOR_HEARTBEAT_GET_OP, BT_MESH_VENDOR_HEARTBEAT_MSG_GET_MIN_LEN, vendor_heartbeat_handle_get },
    BT_MESH_MODEL_OP_END,
};


/* Handle the response status message to the client. */
static int vendor_heartbeat_handle_get(struct bt_mesh_model *model,
                               struct bt_mesh_msg_ctx *ctx,
                               struct net_buf_simple *buf)
{
	printk("vendor_heartbeat_handle_get\n");
    //struct bt_mesh_vendor_heartbeat_srv *srv = model->user_data;
	
	//TODO Uncommenting below line will cause the programm to crash with a hard fault.
	//srv->handlers->get(srv, ctx, &player_status);

	vendor_heartbeat_rsp_status(model, ctx, &local_player_status);

	return 0;
}
#endif