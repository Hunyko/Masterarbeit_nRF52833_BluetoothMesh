/*
* Vendor server model that implements a playerstatus model.
* Client can ask for the current status of a player by sending an ackned get message.
* Server responds with a playerstatus status message containing the bit encoded player state.
* Server also allows for periodic publishing of the playerstatus state.
*/

#ifndef VENDOR_PLAYERSTATUS_SRV_H
#define VENDOR_PLAYERSTATUS_SRV_H

#include "vendor_model_common.h"

struct bt_mesh_vendor_playerstatus_srv;

/** @def BT_MESH_VENDOR_PLAYERSTATUS_SRV_INIT
 *
 * @brief Init parameters for a @ref bt_mesh_vendor_playerstatus_srv instance.
 *
 * @param[in] _handlers State access handlers to use in the model instance.
 */
#define BT_MESH_VENDOR_PLAYERSTATUS_SRV_INIT(_handlers)              \
	{                                                             \
		.handlers = _handlers,                                    \
	}


/** @def BT_MESH_VENDOR_MODEL_PLAYERSTATUS_SRV
 *
 * @brief Vendor Playerstatus model composition data entry.
 *
 * @param[in] _srv Pointer to a @ref bt_mesh_vendor_playerstatus_srv instance.
 */
#define BT_MESH_VENDOR_MODEL_PLAYERSTATUS_SRV(_srv)                                            	\
	BT_MESH_MODEL_VND_CB(SPYRA_COMPANY_ID,														\
						BT_MESH_MODEL_ID_SPYRA_PLAYERSTATUS_SRV,                               	\
			 			_vendor_playerstatus_srv_opcode_list, &(_srv)->pub,                      	\
			 			BT_MESH_MODEL_USER_DATA(struct bt_mesh_vendor_playerstatus_srv, _srv),  	\
			 			&_bt_mesh_vendor_playerstatus_srv_cb)


int vendor_playerstatus_srv_pub(struct bt_mesh_vendor_playerstatus_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_playerstatus_status *status);


/** Generic Vendor Playerstatus Server state access handlers. */
struct bt_mesh_vendor_playerstatus_srv_handlers {
	/** @brief Get the Playerstatus state.
	 *
	 * @note This handler is mandatory.
	 *
	 * @param[in] srv Server instance to get the state of.
	 * @param[in] ctx Message context for the message that triggered the
	 * change, or NULL if the change is not coming from a message.
	 * @param[out] rsp Response structure to be filled.
	 */
	void (*const get)(struct bt_mesh_vendor_playerstatus_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  struct bt_mesh_vendor_playerstatus_status *rsp);
};

/**
 * Generic Vendor Playerstatus Server instance. Should primarily be initialized with the
 * @ref BT_MESH_PLAYERSTATUS_SRV_INIT macro.
 */
struct bt_mesh_vendor_playerstatus_srv {
	/** Transaction ID tracker. */
	struct bt_mesh_tid_ctx prev_transaction;
	/** Handler function structure. */
	const struct bt_mesh_vendor_playerstatus_srv_handlers *handlers;
	/** Access model pointer. */
	struct bt_mesh_model *model;
	/** Publish parameters. */
	struct bt_mesh_model_pub pub;
	/* Publication buffer */
	struct net_buf_simple pub_buf;
	/* Publication data */
	uint8_t pub_data[BT_MESH_MODEL_BUF_LEN(
		BT_MESH_VENDOR_PLAYERSTATUS_STATUS_OP, BT_MESH_VENDOR_PLAYERSTATUS_MSG_STATUS_MAX_LEN)];
	/** Internal flag state. */
	atomic_t flags;
};


/** @cond INTERNAL_HIDDEN */
extern const struct bt_mesh_model_op _vendor_playerstatus_srv_opcode_list[];
extern const struct bt_mesh_model_cb _bt_mesh_vendor_playerstatus_srv_cb;
/** @endcond */

#endif


