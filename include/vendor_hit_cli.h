/*
* Vendor client model that implements a hit model.
* Message from the client is sent after the client (hit detection) recognizes a hit. This is triggered outside the bluetooth environment.
* The model then sends an acknowledged SET message to the server (watergun) to update the lifes remaining.
* The message payload consists of the playerid (hit detection and watergun belong to the same player) and the remaining lifes. Both are bit encoded.
*/

#ifndef VENDOR_HIT_CLI_H
#define VENDOR_HIT_CLI_H

#include "vendor_model_common.h"

struct bt_mesh_vendor_hit_cli;

/** @def BT_MESH_VENDOR_HIT_CLI_INIT
 *
 * @brief Initialization parameters for a @ref bt_mesh_vendor_hit_cli instance.
 *
 * @param[in] _status_handler Optional status message handler.
 */
#define BT_MESH_VENDOR_HIT_CLI_INIT(_handlers)	\
	{                                                   \
		.handlers = _handlers,                          \
	}

/** @def BT_MESH_MODEL_VENDOR_HIT_CLI
 *
 * @brief Vendor HIT Client model composition data entry.
 *
 * @param[in] _cli Pointer to a @ref bt_mesh_vendor_hit_cli instance.
 */
#define BT_MESH_VENDOR_MODEL_HIT_CLI(_cli)                               				\
	BT_MESH_MODEL_VND_CB(SPYRA_COMPANY_ID,													\
						BT_MESH_MODEL_ID_SPYRA_HIT_CLI,                   			\
			 			_vendor_hit_cli_opcode_list, &(_cli)->pub,                  	\
			 			BT_MESH_MODEL_USER_DATA(struct bt_mesh_vendor_hit_cli, _cli),	\
			 			&_bt_mesh_vendor_hit_cli_cb)


/** Generic Vendor Hit Client state access handlers. */
struct bt_mesh_vendor_hit_cli_handlers {
	/** @brief Get the Hit state.
	 *
	 * @note This handler is mandatory.
	 *
	 * @param[in] cli Client instance to get the state of.
	 * @param[in] ctx Message context for the message that triggered the
	 * change, or NULL if the change is not coming from a message.
	 * @param[out] rsp Response structure to be filled.
	 */
	void (*const status)(struct bt_mesh_vendor_hit_cli *cli,
			  struct bt_mesh_msg_ctx *ctx,
			  struct bt_mesh_vendor_playerstatus_status *rsp);
};
                                         
/**
 * Vendor Hit Client structure.
 *
 * Should be initialized with the @ref BT_MESH_VENDOR_HIT_CLI_INIT macro.
 */
struct bt_mesh_vendor_hit_cli {
	/** Current Transaction ID. */
	uint8_t tid;
	/** Response context for tracking acknowledged messages. */
	struct bt_mesh_msg_ack_ctx ack_ctx;
	/** Publish parameters. */
	struct bt_mesh_model_pub pub;
	/* Publication buffer */
	struct net_buf_simple pub_buf;
	/* Publication data */
	uint8_t pub_data[BT_MESH_MODEL_BUF_LEN(BT_MESH_VENDOR_HIT_SET_OP,
					       BT_MESH_VENDOR_HIT_MSG_SET_MAX_LEN)];
	/** Access model pointer. */
	struct bt_mesh_model *model;
	/** Handler function structure. */
	const struct bt_mesh_vendor_hit_cli_handlers *handlers;
};

/** @brief Set the Hit state in the srv.
 *
 * This call is blocking if the @p rsp buffer is non-NULL. Otherwise, this
 * function will return, and the response will be passed to the
 * @ref bt_mesh_vendor_heartbeat_cli::status_handler callback.
 *
 * @param[in] cli Client model to send on.
 * @param[in] ctx Message context, or NULL to use the configured publish
 * parameters.
 * @param[in] set New hit parameters to set. @p set::transition can either
 * point to a transition structure, or be left to NULL to use the default
 * transition parameters on the server.
 * @param[out] rsp Status response buffer, or NULL to keep from blocking.
 *
 * @retval 0 Successfully sent the message and populated the @p rsp buffer.
 * @retval -EALREADY A blocking request is already in progress.
 * @retval -EADDRNOTAVAIL A message context was not provided and publishing is
 * not configured.
 * @retval -EAGAIN The device has not been provisioned.
 * @retval -ETIMEDOUT The request timed out without a response.
 */
int bt_mesh_vendor_hit_cli_set(struct bt_mesh_vendor_hit_cli *cli,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_hit_set *set,
			  struct bt_mesh_vendor_playerstatus_status *rsp);

/** @cond INTERNAL_HIDDEN */
extern const struct bt_mesh_model_op _vendor_hit_cli_opcode_list[];
extern const struct bt_mesh_model_cb _bt_mesh_vendor_hit_cli_cb;
/** @endcond */


#endif