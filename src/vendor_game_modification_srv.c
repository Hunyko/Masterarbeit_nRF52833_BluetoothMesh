#include "../include/vendor_game_modification_srv.h"
#include <stdlib.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/bluetooth/mesh/msg.h>
#include "../../../../ncs/v2.5.0/nrf/subsys/bluetooth/mesh/model_utils.h"


//TODO Add the player information to a central database.


//NOTE Initializes the buffer and adds the message content.
static void vendor_game_modification_encode_status(struct net_buf_simple *buf,
			  const struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("vendor_game_modification_encode_status\n");
	bt_mesh_model_msg_init(buf, BT_MESH_VENDOR_GAME_MODIFICATION_STATUS_OP);

    net_buf_simple_add_u8(buf, status->game_status.running);
    net_buf_simple_add_u8(buf, status->game_status.init_game_delay);
	net_buf_simple_add_le16(buf, status->game_status.init_game_time);
}

//NOTE Defines the message buffer, calls functions to fill it with content and sends the message.
static void vendor_game_modification_rsp_status(struct bt_mesh_model *model,
		       struct bt_mesh_msg_ctx *rx_ctx,
		       const struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("vendor_game_modification_rsp_status\n");
	BT_MESH_MODEL_BUF_DEFINE(reply, BT_MESH_VENDOR_GAME_MODIFICATION_STATUS_OP, BT_MESH_VENDOR_GAME_MODIFICATION_MSG_STATUS_MAX_LEN);
	vendor_game_modification_encode_status(&reply, status);

	(void) bt_mesh_model_send(model, rx_ctx, &reply, NULL, NULL);
}

//TODO
static int vendor_game_modification_handle_get(struct bt_mesh_model *model,
                               struct bt_mesh_msg_ctx *ctx,
                               struct net_buf_simple *buf)
{
	//printk("vendor_game_modification_handle_get\n");
    //struct bt_mesh_vendor_game_modification_srv *srv = model->user_data;
	
	//int err;
	//TODO Uncommenting below line will cause the programm to crash with a hard fault.
	//srv->handlers->get(srv, ctx, &player_status);

	vendor_game_modification_rsp_status(model, ctx, &local_player_status);

	return 0;
}
//NOTE Makes the game modifications
static int vendor_game_modification_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		      struct net_buf_simple *buf, bool ack)
{
	//printk("vendor_game_modification_set\n");
	if (buf->len != BT_MESH_VENDOR_GAME_MODIFICATION_MSG_SET_MIN_LEN &&
	    buf->len != BT_MESH_VENDOR_GAME_MODIFICATION_MSG_SET_MAX_LEN) {
		printk("buf len out of bounds\n");
		printk("%d\n", buf->len);
		//return -EMSGSIZE;
	}

	struct bt_mesh_vendor_game_modification_srv *srv = model->user_data;
	struct bt_mesh_model_transition transition;
	struct bt_mesh_vendor_game_modification_set set;

	uint8_t running = net_buf_simple_pull_u8(buf);	
	uint8_t init_game_delay = net_buf_simple_pull_u8(buf);
	uint16_t init_game_time = net_buf_simple_pull_le16(buf);
	uint8_t tid = net_buf_simple_pull_u8(buf);

	printk("Running: %d\n", running);
	printk("Init_Game_Delay: %d\n", init_game_delay);
	printk("Init_Game_Time: %d\n", init_game_time);
	printk("tid: %d\n", tid);
	
	set.running = running;
	set.init_game_delay = init_game_delay;
	set.init_game_time = init_game_time;

	srv->handlers->set(srv, ctx, &set, &local_player_status);

	//(void)vendor_game_modification_srv_pub(srv, NULL, &local_player_status);

respond:
	if (ack) {
		vendor_game_modification_rsp_status(model, ctx, &local_player_status);
	}

	return 0;
}
//NOTE Handles the set message
static int vendor_game_modification_handle_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		      struct net_buf_simple *buf)
{
	//printk("vendor_game_modification_handle_set\n");
	return vendor_game_modification_set(model, ctx, buf, true);
}
static int vendor_game_modification_handle_set_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
			    struct net_buf_simple *buf)
{
	//printk("vendor_game_modification_handle_set_unack\n");
	return vendor_game_modification_set(model, ctx, buf, false);
}

static int vendor_game_modification_update_handler(struct bt_mesh_model *model)
{
	//printk("vendor_game_modification_update_handler\n");
	//int err;
	//struct bt_mesh_vendor_game_modification_srv *srv = model->user_data;


	//TODO Uncommenting below line will cause the programm to crash with a hard fault.
	//srv->handlers->get(srv, NULL, &game_modification_player_status);
	vendor_game_modification_encode_status(model->pub->msg, &local_player_status);

	return 0;
}

static void vendor_game_modification_srv_reset(struct bt_mesh_model *model)
{
	//printk("vendor_game_modification_srv_reset\n");
	net_buf_simple_reset(model->pub->msg);
}


static int vendor_game_modification_srv_init(struct bt_mesh_model *model)
{
	//printk("vendor_game_modification_srv_init\n");
	struct bt_mesh_vendor_game_modification_srv *srv = model->user_data;
	
	srv->model = model;
	srv->pub.msg = &srv->pub_buf;
	srv->pub.update = vendor_game_modification_update_handler;
	//TODO Model does not automatically start broadcasting after adding a application key. Why?
	//srv->pub.period = 2;
	//Publishing to the broadcast address (all nodes)
	//srv->pub.addr = 0xFFFF;
	//Message will not be relayed
	//srv->pub.ttl = 0;
	//srv->pub.retransmit = 0;
	net_buf_simple_init_with_data(&srv->pub_buf, srv->pub_data,
				      sizeof(srv->pub_data));
	
	return 0;
}

const struct bt_mesh_model_cb _bt_mesh_vendor_game_modification_srv_cb = {
	.init = vendor_game_modification_srv_init,
	.reset = vendor_game_modification_srv_reset,
};

int vendor_game_modification_srv_pub(struct bt_mesh_vendor_game_modification_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("vendor_game_modification_srv_pub\n");
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_GAME_MODIFICATION_STATUS_OP,
				 BT_MESH_VENDOR_GAME_MODIFICATION_MSG_STATUS_MAX_LEN);
	vendor_game_modification_encode_status(&msg, status);
	return bt_mesh_msg_send(srv->model, ctx, &msg);
}

// Opcode list for this vendor model.
const struct bt_mesh_model_op _vendor_game_modification_srv_opcode_list[] = {
	{ BT_MESH_VENDOR_GAME_MODIFICATION_SET_OP, BT_MESH_VENDOR_GAME_MODIFICATION_MSG_SET_MIN_LEN, vendor_game_modification_handle_set },
    BT_MESH_MODEL_OP_END,
};