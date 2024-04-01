#include "../include/vendor_device_modification_srv.h"
#include <stdlib.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/bluetooth/mesh/msg.h>
#include "../../../../ncs/v2.5.0/nrf/subsys/bluetooth/mesh/model_utils.h"


//NOTE Initializes the buffer and adds the message content.
static void vendor_device_modification_encode_status(struct net_buf_simple *buf,
			  const struct bt_mesh_vendor_playerstatus_status *status)
{
	printk("vendor_device_modification_encode_status\n");
	bt_mesh_model_msg_init(buf, BT_MESH_VENDOR_DEVICE_MODIFICATION_STATUS_OP);

    net_buf_simple_add_u8(buf, status->device_status.active);
    net_buf_simple_add_u8(buf, status->device_status.game_mode);
}

//NOTE Defines the message buffer, calls functions to fill it with content and sends the message.
static void vendor_device_modification_rsp_status(struct bt_mesh_model *model,
		       struct bt_mesh_msg_ctx *rx_ctx,
		       const struct bt_mesh_vendor_playerstatus_status *status)
{
	printk("vendor_device_modification_rsp_status\n");
	BT_MESH_MODEL_BUF_DEFINE(reply, BT_MESH_VENDOR_DEVICE_MODIFICATION_STATUS_OP, BT_MESH_VENDOR_DEVICE_MODIFICATION_MSG_STATUS_MAX_LEN);
	vendor_device_modification_encode_status(&reply, status);

	(void) bt_mesh_model_send(model, rx_ctx, &reply, NULL, NULL);
}

static int vendor_device_modification_handle_get(struct bt_mesh_model *model,
                               struct bt_mesh_msg_ctx *ctx,
                               struct net_buf_simple *buf)
{
	printk("vendor_device_modification_handle_get\n");

	vendor_device_modification_rsp_status(model, ctx, &local_player_status);

	return 0;
}

//NOTE Makes the device modifications.
static int vendor_device_modification_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		      struct net_buf_simple *buf, bool ack)
{
	printk("vendor_device_modification_onoff_set\n");
	if (buf->len != BT_MESH_VENDOR_DEVICE_MODIFICATION_MSG_SET_MIN_LEN &&
	    buf->len != BT_MESH_VENDOR_DEVICE_MODIFICATION_MSG_SET_MAX_LEN) {
		printk("buf len out of bounds\n");
		printk("%d\n", buf->len);
		//return -EMSGSIZE;
	}

	struct bt_mesh_vendor_device_modification_srv *srv = model->user_data;
	struct bt_mesh_vendor_device_modification_set set;

	uint8_t active = net_buf_simple_pull_u8(buf);	
	uint8_t game_mode = net_buf_simple_pull_u8(buf);

	printk("Active: %d\n", active);
	printk("GameMode: %d\n", game_mode);
	
	set.active = active;
	set.game_mode = game_mode;
	
	srv->handlers->set(srv, ctx, &set, &local_player_status);

	(void)vendor_device_modification_srv_pub(srv, NULL, &local_player_status);

	if (ack) {
		vendor_device_modification_rsp_status(model, ctx, &local_player_status);
	}

	return 0;
}
//NOTE Handles the set message
static int vendor_device_modification_handle_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		      struct net_buf_simple *buf)
{
	printk("vendor_device_modification_handle_set\n");
	return vendor_device_modification_set(model, ctx, buf, true);
}

static int vendor_device_modification_update_handler(struct bt_mesh_model *model)
{
	printk("vendor_device_modification_update_handler\n");

	vendor_device_modification_encode_status(model->pub->msg, &local_player_status);

	return 0;
}

static void vendor_device_modification_srv_reset(struct bt_mesh_model *model)
{
	printk("vendor_device_modification_srv_reset\n");
	net_buf_simple_reset(model->pub->msg);
}


static int vendor_device_modification_srv_init(struct bt_mesh_model *model)
{
	printk("vendor_device_modification_srv_init\n");
	struct bt_mesh_vendor_device_modification_srv *srv = model->user_data;
	
	srv->model = model;
	srv->pub.msg = &srv->pub_buf;
	srv->pub.update = vendor_device_modification_update_handler;

	net_buf_simple_init_with_data(&srv->pub_buf, srv->pub_data,
				      sizeof(srv->pub_data));
	
	return 0;
}

const struct bt_mesh_model_cb _bt_mesh_vendor_device_modification_srv_cb = {
	.init = vendor_device_modification_srv_init,
	.reset = vendor_device_modification_srv_reset,
};

int vendor_device_modification_srv_pub(struct bt_mesh_vendor_device_modification_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_playerstatus_status *status)
{
	printk("vendor_device_modification_srv_pub\n");
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_DEVICE_MODIFICATION_STATUS_OP,
				 BT_MESH_VENDOR_DEVICE_MODIFICATION_MSG_STATUS_MAX_LEN);
	vendor_device_modification_encode_status(&msg, status);
	return bt_mesh_msg_send(srv->model, ctx, &msg);
}

// Opcode list for this vendor model.
const struct bt_mesh_model_op _vendor_device_modification_srv_opcode_list[] = {
	{ BT_MESH_VENDOR_DEVICE_MODIFICATION_GET_OP, BT_MESH_VENDOR_DEVICE_MODIFICATION_MSG_GET_MIN_LEN, vendor_device_modification_handle_get },
	{ BT_MESH_VENDOR_DEVICE_MODIFICATION_SET_OP, BT_MESH_VENDOR_DEVICE_MODIFICATION_MSG_SET_MIN_LEN, vendor_device_modification_handle_set },
    BT_MESH_MODEL_OP_END,
};