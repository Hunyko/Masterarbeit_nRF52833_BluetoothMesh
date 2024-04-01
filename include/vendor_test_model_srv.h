/*
* Vendor model that implements a simple heartbeat function which is published by a server model in regular intervalls.  
*/

#ifndef VENDOR_TEST_MODEL_SRV_H
#define VENDOR_TEST_MODEL_SRV_H

#include "vendor_model_common.h"

struct bt_mesh_vendor_test_model_srv;

/** @def BT_MESH_VENDOR_TEST_MODEL_SRV_INIT
 *
 * @brief Init parameters for a @ref bt_mesh_vendor_test_model_srv instance.
 *
 * @param[in] _handlers State access handlers to use in the model instance.
 */
#define BT_MESH_VENDOR_TEST_MODEL_SRV_INIT(_handlers)              \
	{                                                             \
		.handlers = _handlers,                                    \
	}


/** @def BT_MESH_VENDOR_MODEL_TEST_MODEL_SRV
 *
 * @brief Vendor Test model composition data entry.
 *
 * @param[in] _srv Pointer to a @ref bt_mesh_vendor_test_model_srv instance.
 */
#if USE_STANDARD_MODEL
#define BT_MESH_VENDOR_MODEL_TEST_MODEL_SRV(_srv)                                            	\
	BT_MESH_MODEL_CB(	BT_MESH_MODEL_ID_GEN_ONOFF_SRV,                               	\
			 			_vendor_test_model_srv_opcode_list, &(_srv)->pub,                      	\
			 			BT_MESH_MODEL_USER_DATA(struct bt_mesh_vendor_test_model_srv, _srv),  	\
			 			&_bt_mesh_vendor_test_model_srv_cb)
#else
#define BT_MESH_VENDOR_MODEL_TEST_MODEL_SRV(_srv)                                            	\
	BT_MESH_MODEL_VND_CB(SPYRA_COMPANY_ID,														\
						BT_MESH_MODEL_ID_SPYRA_TEST_MODEL_SRV,                               	\
			 			_vendor_test_model_srv_opcode_list, &(_srv)->pub,                      	\
			 			BT_MESH_MODEL_USER_DATA(struct bt_mesh_vendor_test_model_srv, _srv),  	\
			 			&_bt_mesh_vendor_test_model_srv_cb)
#endif

int vendor_test_model_srv_pub(struct bt_mesh_vendor_test_model_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_playerstatus_status *status);

//NOTE Taken from OnOff Example
/** Generic Vendor TestModel Server state access handlers. */
struct bt_mesh_vendor_test_model_srv_handlers {
	/** @brief Set the TestModel state.
	 *
	 * When a set message is received, the model publishes a status message, with the response
	 * set to @c rsp. When an acknowledged set message is received, the model also sends a
	 * response back to a client. If a state change is non-instantaneous, for example when
	 * @ref bt_mesh_model_transition_time returns a nonzero value, the application is
	 * responsible for publishing a value of the OnOff state at the end of the transition.
	 *
	 * @note This handler is mandatory.
	 *
	 * @param[in] srv Server instance to set the state of.
	 * @param[in] ctx Message context for the message that triggered the
	 * change, or NULL if the change is not coming from a message.
	 * @param[in] set Parameters of the state change.
	 * @param[out] rsp Response structure to be filled.
	 */
	void (*const set)(struct bt_mesh_vendor_test_model_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_test_model_set *set,
			  struct bt_mesh_vendor_playerstatus_status *rsp);
	/** @brief Get the TestModel state.
	 *
	 * @note This handler is mandatory.
	 *
	 * @param[in] srv Server instance to get the state of.
	 * @param[in] ctx Message context for the message that triggered the
	 * change, or NULL if the change is not coming from a message.
	 * @param[out] rsp Response structure to be filled.
	 */
	void (*const get)(struct bt_mesh_vendor_test_model_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  struct bt_mesh_vendor_playerstatus_status *rsp);
};

//NOTE Taken from OnOff Example
/**
 * Generic Vendor TestModel Server instance. Should primarily be initialized with the
 * @ref BT_MESH_TEST_MODEL_SRV_INIT macro.
 */
struct bt_mesh_vendor_test_model_srv {
	/** Transaction ID tracker. */
	struct bt_mesh_tid_ctx prev_transaction;
	/** Handler function structure. */
	const struct bt_mesh_vendor_test_model_srv_handlers *handlers;
	/** Access model pointer. */
	struct bt_mesh_model *model;
	/** Publish parameters. */
	struct bt_mesh_model_pub pub;
	/* Publication buffer */
	struct net_buf_simple pub_buf;
	/* Publication data */
	uint8_t pub_data[BT_MESH_MODEL_BUF_LEN(
		BT_MESH_VENDOR_TEST_MODEL_STATUS_OP, BT_MESH_VENDOR_TEST_MODEL_MSG_STATUS_MAX_LEN)];
	/** Internal flag state. */
	atomic_t flags;
};


/** @cond INTERNAL_HIDDEN */
extern const struct bt_mesh_model_op _vendor_test_model_srv_opcode_list[];
extern const struct bt_mesh_model_cb _bt_mesh_vendor_test_model_srv_cb;
/** @endcond */

#endif


