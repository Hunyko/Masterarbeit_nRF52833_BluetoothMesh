/*
* Vendor server model that implements a hit model.
* Message from the client is sent after the client (hit detection) recognizes a hit. This is triggered outside the bluetooth environment.
* The model then sends an acknowledged SET message to the server (watergun) to update the lifes remaining.
* The message payload consists of the playerid (hit detection and watergun belong to the same player) and the remaining lifes. Both are bit encoded.
*/

#ifndef VENDOR_HIT_SRV_H
#define VENDOR_HIT_SRV_H

#include "vendor_model_common.h"

struct bt_mesh_vendor_hit_srv;

/** @def BT_MESH_VENDOR_HIT_SRV_INIT
 *
 * @brief Init parameters for a @ref bt_mesh_vendor_hit_srv instance.
 *
 * @param[in] _handlers State access handlers to use in the model instance.
 */
#define BT_MESH_VENDOR_HIT_SRV_INIT(_handlers)              \
	{                                                             \
		.handlers = _handlers,                                    \
	}


/** @def BT_MESH_VENDOR_MODEL_HIT_SRV
 *
 * @brief Vendor Hit model composition data entry.
 *
 * @param[in] _srv Pointer to a @ref bt_mesh_vendor_hit_srv instance.
 */
#define BT_MESH_VENDOR_MODEL_HIT_SRV(_srv)                                            	\
	BT_MESH_MODEL_VND_CB(SPYRA_COMPANY_ID,														\
						BT_MESH_MODEL_ID_SPYRA_HIT_SRV,                               	\
			 			_vendor_hit_srv_opcode_list, &(_srv)->pub,                      	\
			 			BT_MESH_MODEL_USER_DATA(struct bt_mesh_vendor_hit_srv, _srv),  	\
			 			&_bt_mesh_vendor_hit_srv_cb)

int vendor_hit_srv_pub(struct bt_mesh_vendor_hit_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_playerstatus_status *status);

/** Generic Vendor Hit Server state access handlers. */
struct bt_mesh_vendor_hit_srv_handlers {
	/** @brief Set the Hit state.
	 *
	 * When a set message is received, the model publishes a status message, with the response
	 * set to @c rsp. When an acknowledged set message is received, the model also sends a
	 * response back to a client.
	 *
	 * @note This handler is mandatory.
	 *
	 * @param[in] srv Server instance to set the state of.
	 * @param[in] ctx Message context for the message that triggered the
	 * change, or NULL if the change is not coming from a message.
	 * @param[in] set Parameters of the state change.
	 * @param[out] rsp Response structure to be filled.
	 */
	void (*const set)(struct bt_mesh_vendor_hit_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_hit_set *set,
			  struct bt_mesh_vendor_playerstatus_status *rsp);
	/** @brief Get the Hit Model state.
	 *
	 * @note This handler is mandatory.
	 *
	 * @param[in] srv Server instance to get the state of.
	 * @param[in] ctx Message context for the message that triggered the
	 * change, or NULL if the change is not coming from a message.
	 * @param[out] rsp Response structure to be filled.
	 */
	void (*const get)(struct bt_mesh_vendor_hit_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  struct bt_mesh_vendor_playerstatus_status *rsp);
};

/**
 * Generic Vendor Hit Server instance. Should primarily be initialized with the
 * @ref BT_MESH_VENDOR_HIT_SRV_INIT macro.
 */
struct bt_mesh_vendor_hit_srv {
	/** Transaction ID tracker. */
	struct bt_mesh_tid_ctx prev_transaction;
	/** Handler function structure. */
	const struct bt_mesh_vendor_hit_srv_handlers *handlers;
	/** Access model pointer. */
	struct bt_mesh_model *model;
	/** Publish parameters. */
	struct bt_mesh_model_pub pub;
	/* Publication buffer */
	struct net_buf_simple pub_buf;
	/* Publication data */
	uint8_t pub_data[BT_MESH_MODEL_BUF_LEN(
		BT_MESH_VENDOR_HIT_STATUS_OP, BT_MESH_VENDOR_HIT_MSG_STATUS_MAX_LEN)];
	/** Internal flag state. */
	atomic_t flags;
};


/** @cond INTERNAL_HIDDEN */
extern const struct bt_mesh_model_op _vendor_hit_srv_opcode_list[];
extern const struct bt_mesh_model_cb _bt_mesh_vendor_hit_srv_cb;
/** @endcond */

#endif


