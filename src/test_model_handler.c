/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <dk_buttons_and_leds.h>
#include "test_model_handler.h"
#include "../include/vendor_hit_cli.h"
#include "../include/vendor_hit_srv.h"
#include "../include/vendor_device_modification_cli.h"
#include "../include/vendor_device_modification_srv.h"
#include "../include/vendor_game_modification_cli.h"
#include "../include/vendor_game_modification_srv.h"
#include "../include/vendor_heartbeat_cli.h"
#include "../include/vendor_heartbeat_srv.h"
#include "../include/vendor_playerstatus_cli.h"
#include "../include/vendor_playerstatus_srv.h"
#include "../include/vendor_test_model_cli.h"
#include "../include/vendor_test_model_srv.h"


struct button {
	bool status;
	struct bt_mesh_vendor_test_model_cli client;
};

struct bt_mesh_vendor_playerstatus_status local_player_status = {
	.player_id = TEAM_BLUE_PLAYER_ID_1,
	.player_lifecount = PLAYER_LIFE_5,
	.tank_left =  TANK_LEVEL_0_PERCENT,
	.tanks_since_init = 0,
	.battery_status = {.battery_status_hit_detection = BATTERY_LEVEL_100_PERCENT, .battery_status_watergun = BATTERY_LEVEL_100_PERCENT},
};

#if PROGRAMM_CLIENT
static void test_model_status(struct bt_mesh_vendor_test_model_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_playerstatus_status *status);
		
static const struct bt_mesh_vendor_test_model_cli_handlers vendor_test_model_cli_handlers = {
	.status = test_model_status,
};

static struct button buttons[] = {
#if DT_NODE_EXISTS(DT_ALIAS(sw0))
	{.client = BT_MESH_VENDOR_TEST_MODEL_CLI_INIT(&vendor_test_model_cli_handlers)},
#endif
#if DT_NODE_EXISTS(DT_ALIAS(sw1))
	{.client = BT_MESH_VENDOR_TEST_MODEL_CLI_INIT(&vendor_test_model_cli_handlers)},
#endif
};
#else
static void test_model_get(struct bt_mesh_vendor_test_model_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_vendor_playerstatus_status *rsp);


static void test_model_set(struct bt_mesh_vendor_test_model_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_vendor_test_model_set *set,
		    struct bt_mesh_vendor_playerstatus_status *rsp);	

static const struct bt_mesh_vendor_test_model_srv_handlers vendor_test_model_srv_handlers = {
	.get = test_model_get,
	.set = test_model_set,
};

struct test_model_ctx {
	struct bt_mesh_vendor_test_model_srv srv;
	struct k_work_delayable work;
	struct bt_mesh_vendor_playerstatus_status status;
};

static struct test_model_ctx test_model_ctx[] = {
#if DT_NODE_EXISTS(DT_ALIAS(led1))
	{.srv = BT_MESH_VENDOR_TEST_MODEL_SRV_INIT(&vendor_test_model_srv_handlers)},
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led2))
	{ .srv = BT_MESH_VENDOR_TEST_MODEL_SRV_INIT(&vendor_test_model_srv_handlers) },
#endif
};

#endif



static void led_status(struct test_model_ctx *test_model, struct bt_mesh_vendor_playerstatus_status *status)
{
	printk("led_status\n");
	//status->player_id = heartbeat->status.player_id;
	//status->player_lifecount = heartbeat->status.player_lifecount;
}
#if PROGRAMM_CLIENT
//TODO Check if implemenation is correct or needs to be updated for get message (which I assume right now)
static void test_model_status(struct bt_mesh_vendor_test_model_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_playerstatus_status *status)
{
	printk("test_model_status\n");
	struct button *button =
		CONTAINER_OF(cli, struct button, client);
	int index = button - &buttons[0];

	button->status = status->player_lifecount > 0 ? 1 : 0;
	dk_set_led(index, status->player_lifecount > 0 ? 1 : 0);

	/*
	printk("Button %d: Received response: %s\n", index + 1,
	       status->player_lifecount ? "on" : "off");
	*/
}

static void button_handler_cb(uint32_t pressed, uint32_t changed)
{
	printk("button_handler_cb\n");
	struct bt_mesh_vendor_playerstatus_status status;

	if (!bt_mesh_is_provisioned()) {
		return;
	}

	if (IS_ENABLED(CONFIG_BT_MESH_LOW_POWER) && (pressed & changed & BIT(3))) {
		bt_mesh_proxy_identity_enable();
		return;
	}

	for (int i = 0; i < ARRAY_SIZE(buttons); ++i) {
		printk("Button active\n");
		if (!(pressed & changed & BIT(i))) {
			continue;
		}

		int err;

		if (i==1){
			printk("button_set\n");
			struct bt_mesh_vendor_test_model_set set = {
				.on_off = !buttons[i].status,
			};

			/* As we can't know how many nodes are in a group, it doesn't
			* make sense to send acknowledged messages to group addresses -
			* we won't be able to make use of the responses anyway. This also
			* applies in LPN mode, since we can't expect to receive a response
			* in appropriate time.
			*/
			if (bt_mesh_model_pub_is_unicast(buttons[i].client.model) &&
				!IS_ENABLED(CONFIG_BT_MESH_LOW_POWER)) {
				err = bt_mesh_vendor_test_model_cli_set(&buttons[i].client, NULL, &set, NULL);
			} else {
				err = bt_mesh_vendor_test_model_cli_set_unack(&buttons[i].client,
								NULL, &set);
				if (!err) {
					/* There'll be no response status for the
					* unacked message. Set the state immediately.
					*/
					buttons[i].status = set.on_off;
					dk_set_led(i, set.on_off);
				}
			}
			if (err) {
				printk("OnOff %d set failed: %d\n", i + 1, err);
			}
		} else {	
			printk("button_get\n");			
			err = bt_mesh_vendor_test_model_cli_get(&buttons[i].client, NULL, &status);

			if (err) {
				printk("Test_model %d get failed: %d\n", i + 1, err);
			} else {
				printk("bt_mesh_msg_ackd_send\n");
			}
		}
	}
}
#else
static void test_model_get(struct bt_mesh_vendor_test_model_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_vendor_playerstatus_status *rsp)
{
	printk("test_model_get\n");
	struct test_model_ctx *test_model = CONTAINER_OF(srv, struct test_model_ctx, srv);

	led_status(test_model, rsp);
}

static void test_model_set(struct bt_mesh_vendor_test_model_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_vendor_test_model_set *set,
		    struct bt_mesh_vendor_playerstatus_status *rsp)
{
	printk("test_model_set\n");
	struct test_model_ctx *test_model = CONTAINER_OF(srv, struct test_model_ctx, srv);
	int led_idx = test_model - &test_model_ctx[0];

	printk("set_status: %d\n", set->on_off);

	if (set->on_off == test_model->status.player_lifecount) {
		goto respond;
	}

	test_model->status.player_lifecount = set->on_off;
	if (!bt_mesh_model_transition_time(set->transition)) {
		dk_set_led(led_idx, set->on_off);
		goto respond;
	}

	test_model->status.player_lifecount = set->on_off;

respond:
	if (rsp) {
		led_status(test_model, rsp);
	}
}

static void test_model_work(struct k_work *work)
{
	printk("test_model_work\n");
	struct test_model_ctx *test_model = CONTAINER_OF(work, struct test_model_ctx, work.work);

	struct bt_mesh_vendor_playerstatus_status hb_status;

	led_status(test_model, &hb_status);
	vendor_test_model_srv_pub(&test_model->srv, NULL, &hb_status);
}
#endif
/* Set up a repeating delayed work to blink the DK's LEDs when attention is
 * requested.
 */
static struct k_work_delayable attention_blink_work;
static bool attention;

static void attention_blink(struct k_work *work)
{
	static int idx;
	const uint8_t pattern[] = {
#if DT_NODE_EXISTS(DT_ALIAS(led0))
		BIT(0),
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led1))
		BIT(1),
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led2))
		BIT(2),
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led3))
		BIT(3),
#endif
	};

	if (attention) {
		dk_set_leds(pattern[idx++ % ARRAY_SIZE(pattern)]);
		k_work_reschedule(&attention_blink_work, K_MSEC(30));
	} else {
		dk_set_leds(DK_NO_LEDS_MSK);
	}
}

static void attention_on(struct bt_mesh_model *mod)
{
	attention = true;
	k_work_reschedule(&attention_blink_work, K_NO_WAIT);
}

static void attention_off(struct bt_mesh_model *mod)
{
	/* Will stop rescheduling blink timer */
	attention = false;
}

static const struct bt_mesh_health_srv_cb health_srv_cb = {
	.attn_on = attention_on,
	.attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
	.cb = &health_srv_cb,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);


//NOTE Struct which holds each Node Element and its available models.
static struct bt_mesh_elem elements[] = {
#if DT_NODE_EXISTS(DT_ALIAS(led0))
	BT_MESH_ELEM(
		1, BT_MESH_MODEL_LIST(
			BT_MESH_MODEL_CFG_SRV,
			BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub)),
			//BT_MESH_MODEL_ONOFF_SRV(&led_ctx[0].srv),
		BT_MESH_MODEL_NONE),
#endif
#if !PROGRAMM_CLIENT
#if DT_NODE_EXISTS(DT_ALIAS(led1))
	BT_MESH_ELEM(
		2, BT_MESH_MODEL_LIST(BT_MESH_VENDOR_MODEL_TEST_MODEL_SRV(&test_model_ctx[0].srv)),
		BT_MESH_MODEL_NONE),
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led2))
	BT_MESH_ELEM(
		2, BT_MESH_MODEL_LIST(BT_MESH_VENDOR_MODEL_TEST_MODEL_SRV(&test_model_ctx[1].srv)),
		BT_MESH_MODEL_NONE),
#endif
#else
#if DT_NODE_EXISTS(DT_ALIAS(sw0))
	BT_MESH_ELEM(
		2, BT_MESH_MODEL_LIST(BT_MESH_VENDOR_MODEL_TEST_MODEL_CLI(&buttons[0].client)),
		BT_MESH_MODEL_NONE),
#endif
#if DT_NODE_EXISTS(DT_ALIAS(sw1))
	BT_MESH_ELEM(
		3, BT_MESH_MODEL_LIST(BT_MESH_VENDOR_MODEL_TEST_MODEL_CLI(&buttons[1].client)),
		BT_MESH_MODEL_NONE),
#endif
#endif
};

static const struct bt_mesh_comp comp = {
	.cid = CONFIG_BT_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

//NOTE This function is called in main.c
const struct bt_mesh_comp *test_model_handler_init(void)
{
	printk("test_model_handler_init\n");
	k_work_init_delayable(&attention_blink_work, attention_blink);

	#if PROGRAMM_CLIENT
		//NOTE Initialisierung der Buttons sowie der entsprechenden Callbacks.
		static struct button_handler button_handler = {
			.cb = button_handler_cb,
		};
		dk_button_handler_add(&button_handler);
	#else
		for (int i = 0; i < ARRAY_SIZE(test_model_ctx); ++i) {
		k_work_init_delayable(&test_model_ctx[i].work, test_model_work);
		printk("k_work_init_delayable(&test_model_ctx[%d].work, test_model_work)\n", i);
		}
	#endif

	
	return &comp;
}
