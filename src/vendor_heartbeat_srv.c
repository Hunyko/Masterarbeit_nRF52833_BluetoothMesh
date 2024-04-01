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
	struct bt_mesh_vendor_heartbeat_srv *srv = model->user_data;

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
	srv->pub.update = vendor_heartbeat_update_handler;
	net_buf_simple_init_with_data(&srv->pub_buf, srv->pub_data, sizeof(srv->pub_data));
	
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
