#include "../include/vendor_playerstatus_srv.h"
#include <stdlib.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/bluetooth/mesh/msg.h>
#include "../../../../ncs/v2.5.0/nrf/subsys/bluetooth/mesh/model_utils.h"


static void vendor_playerstatus_encode_status(struct net_buf_simple *buf,
			  const struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Playerstatus_Server: vendor_playerstatus_encode_status\n");
	bt_mesh_model_msg_init(buf, BT_MESH_VENDOR_PLAYERSTATUS_STATUS_OP);
	printk("Playerstatus_Sever: Sending status message...\n");
    net_buf_simple_add_u8(buf, status->player_id);
	printk("\tplayer_id: %d\n", status->player_id);
    net_buf_simple_add_u8(buf, status->player_lifecount);
	printk("\tplayer_lifecount: %d\n", status->player_lifecount);
	net_buf_simple_add_u8(buf, status->tank_left);
	printk("\ttank_left: %d/255\n", status->tank_left);
	net_buf_simple_add_u8(buf, status->tanks_since_init);
}

static void vendor_playerstatus_rsp_status(struct bt_mesh_model *model,
		       struct bt_mesh_msg_ctx *rx_ctx,
		       const struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Playerstatus_Server: vendor_playerstatus_rsp_status\n");
	BT_MESH_MODEL_BUF_DEFINE(reply, BT_MESH_VENDOR_PLAYERSTATUS_STATUS_OP, BT_MESH_VENDOR_PLAYERSTATUS_MSG_STATUS_MAX_LEN);
	vendor_playerstatus_encode_status(&reply, status);

	(void) bt_mesh_model_send(model, rx_ctx, &reply, NULL, NULL);
}

static int vendor_playerstatus_handle_get(struct bt_mesh_model *model,
                               struct bt_mesh_msg_ctx *ctx,
                               struct net_buf_simple *buf)
{

	vendor_playerstatus_rsp_status(model, ctx, &local_player_status);

	return 0;
}

static int vendor_playerstatus_update_handler(struct bt_mesh_model *model)
{
	//printk("Playerstatus_Server: vendor_playerstatus_update_handler\n");
	//int err;
	struct bt_mesh_vendor_playerstatus_srv *srv = model->user_data;

	srv->handlers->get(srv, NULL, NULL);

	vendor_playerstatus_encode_status(model->pub->msg, &local_player_status);

	return 0;
}

static void vendor_playerstatus_srv_reset(struct bt_mesh_model *model)
{
	printk("Playerstatus_Server: vendor_playerstatus_srv_reset\n");
	net_buf_simple_reset(model->pub->msg);
}


static int vendor_playerstatus_srv_init(struct bt_mesh_model *model)
{
	printk("Playerstatus_Server: vendor_playerstatus_srv_init\n");
	//int err;
	struct bt_mesh_vendor_playerstatus_srv *srv = model->user_data;
	
	srv->model = model;
	srv->pub.msg = &srv->pub_buf;
	srv->pub.update = vendor_playerstatus_update_handler;
	net_buf_simple_init_with_data(&srv->pub_buf, srv->pub_data,
				      sizeof(srv->pub_data));
	
	return 0;
}

const struct bt_mesh_model_cb _bt_mesh_vendor_playerstatus_srv_cb = {
	.init = vendor_playerstatus_srv_init,
	.reset = vendor_playerstatus_srv_reset,
};

int vendor_playerstatus_srv_pub(struct bt_mesh_vendor_playerstatus_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Playerstatus_Server: vendor_playerstatus_srv_pub\n");
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_PLAYERSTATUS_STATUS_OP,
				 BT_MESH_VENDOR_PLAYERSTATUS_MSG_STATUS_MAX_LEN);
	vendor_playerstatus_encode_status(&msg, status);
	return bt_mesh_msg_send(srv->model, ctx, &msg);
}

// Opcode list for this vendor model.
const struct bt_mesh_model_op _vendor_playerstatus_srv_opcode_list[] = {
	{ BT_MESH_VENDOR_PLAYERSTATUS_GET_OP, BT_MESH_VENDOR_PLAYERSTATUS_MSG_GET_MIN_LEN, vendor_playerstatus_handle_get },
    BT_MESH_MODEL_OP_END,
};