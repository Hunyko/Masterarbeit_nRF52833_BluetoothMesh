/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <dk_buttons_and_leds.h>
#include "model_handler.h"
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
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include "board.h"


u_int16_t shots_in_tank = MAX_SHOTS_PER_TANK;

/* Implement output pin handling */
void output_pin_interrupt_reset(void);
void output_pin_interrupt_set(uint8_t pin1_status, uint8_t pin2_status, uint8_t pin3_status);

/* Implement ISR */
void input_pin_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
void pin_isr_feedback(uint32_t *bit_to_pin);

static struct gpio_callback input_pin0_cb;
static struct gpio_callback input_pin1_cb;
static struct gpio_callback input_pin2_cb;

static const struct gpio_dt_spec input_pins [] = {
	GPIO_DT_SPEC_GET_OR(HIT_PIN, gpios, {0}),
	GPIO_DT_SPEC_GET_OR(SHOT_PIN, gpios, {0}),
	GPIO_DT_SPEC_GET_OR(PUMP_PIN, gpios, {0}),
};

static const struct gpio_dt_spec output_pins [] = {
	GPIO_DT_SPEC_GET_OR(INTERRUPT_PIN, gpios, {0}),
	GPIO_DT_SPEC_GET_OR(INTERRUPT_DECODER_MSB, gpios, {0}),
	GPIO_DT_SPEC_GET_OR(INTERRUPT_DECODER_2, gpios, {0}),
	GPIO_DT_SPEC_GET_OR(INTERRUPT_DECODER_LSB, gpios, {0}),
};
/* Button handler implementation. Not needed with the final version. */
struct button {
	bool status;
};

static struct button buttons[] = {
	#if DT_NODE_EXISTS(DT_ALIAS(sw0))
	{.status = false},
	#endif
};

/* Struct holding all local player information. Local player consists of HitTarget and WaterGun.*/
struct bt_mesh_vendor_playerstatus_status local_player_status = {
	.player_id = TEAM_BLUE_PLAYER_ID_3,
	.player_lifecount = 100,
	.tank_left =  0,
	.tanks_since_init = 0,
	.battery_status = {.battery_level_hit_detection = 0, .battery_level_watergun = 0},
	.shots_since_init = 0,
	.device_status.active = DEVICE_DISABLED,
	.device_status.game_mode = GAME_MODE_LEAGUE,
	.game_status.running = GAME_PAUSED,
	.game_status.init_game_time = 0,
	.game_status.init_game_delay = 0,
};

/* Define Hit context */
struct hit_ctx {
	struct bt_mesh_vendor_hit_cli cli;
	struct bt_mesh_vendor_hit_srv srv;
	struct k_work_delayable work;
};
/* Hit Client Handlers*/
static void hit_status(struct bt_mesh_vendor_hit_cli *cli, struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_playerstatus_status *status);

static const struct bt_mesh_vendor_hit_cli_handlers vendor_hit_cli_handlers = {
	.status = hit_status,
};
/* Hit Server Handlers*/
static void hit_set(struct bt_mesh_vendor_hit_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_vendor_hit_set *set,
		    struct bt_mesh_vendor_playerstatus_status *rsp);	

static const struct bt_mesh_vendor_hit_srv_handlers vendor_hit_srv_handlers = {
	.set = hit_set,
};
/* Init Hit context */
static struct hit_ctx hit_ctx[] = {
#if (DEVICE_TO_PROGRAM == WATERGUN)
	{.srv = BT_MESH_VENDOR_HIT_SRV_INIT(&vendor_hit_srv_handlers)},
#elif (DEVICE_TO_PROGRAM == HITDETECTION)
	{.cli = BT_MESH_VENDOR_HIT_CLI_INIT(&vendor_hit_cli_handlers)},
#endif
};

#if (DEVICE_TO_PROGRAM == WATERGUN)
/* Define Heartbeat context */
struct heartbeat_ctx {
	struct bt_mesh_vendor_heartbeat_cli cli;
	struct bt_mesh_vendor_heartbeat_srv srv;
	struct k_work_delayable work;
};
/* Heartbeat Client Handlers */
static void heartbeat_status(struct bt_mesh_vendor_heartbeat_cli *cli, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_vendor_playerstatus_status *rsp);	

static const struct bt_mesh_vendor_heartbeat_cli_handlers vendor_heartbeat_cli_handlers = {
	.status = heartbeat_status,
};
/* Heartbeat Server Handlers */
static void heartbeat_get(struct bt_mesh_vendor_heartbeat_srv *srv, struct bt_mesh_msg_ctx *ctx,
	struct bt_mesh_vendor_playerstatus_status *rsp);	

static const struct bt_mesh_vendor_heartbeat_srv_handlers vendor_heartbeat_srv_handlers = {
	.get = heartbeat_get,
};
/* Init Heartbeat context */
static struct heartbeat_ctx heartbeat_ctx[] = {
	{.srv = BT_MESH_VENDOR_HEARTBEAT_SRV_INIT(&vendor_heartbeat_srv_handlers)},
	{.cli = BT_MESH_VENDOR_HEARTBEAT_CLI_INIT(&vendor_heartbeat_cli_handlers)},
};
#endif

#if (DEVICE_TO_PROGRAM == WATERGUN || DEVICE_TO_PROGRAM == GAMEMASTER)
/* Playerstatus context */
struct playerstatus_ctx {
	struct bt_mesh_vendor_playerstatus_cli cli;
	struct bt_mesh_vendor_playerstatus_srv srv;
	struct k_work_delayable work;
};
/* Playerstatus Client Handlers */
static void playerstatus_status(struct bt_mesh_vendor_playerstatus_cli *srv, struct bt_mesh_msg_ctx *ctx,
	struct bt_mesh_vendor_playerstatus_status *rsp);	

static const struct bt_mesh_vendor_playerstatus_cli_handlers vendor_playerstatus_cli_handlers = {
	.status = playerstatus_status,
};
/* Playerstatus Server Handlers */
static void playerstatus_get(struct bt_mesh_vendor_playerstatus_srv *srv, struct bt_mesh_msg_ctx *ctx,
	struct bt_mesh_vendor_playerstatus_status *rsp);	

static const struct bt_mesh_vendor_playerstatus_srv_handlers vendor_playerstatus_srv_handlers = {
	.get = playerstatus_get,
};
/* Init Playerstatus context */
static struct playerstatus_ctx playerstatus_ctx[] = {
	#if (DEVICE_TO_PROGRAM == WATERGUN)
	{.srv = BT_MESH_VENDOR_PLAYERSTATUS_SRV_INIT(&vendor_playerstatus_srv_handlers)},
	#elif (DEVICE_TO_PROGRAM == GAMEMASTER)
	{.cli = BT_MESH_VENDOR_PLAYERSTATUS_CLI_INIT(&vendor_playerstatus_cli_handlers)},
	#endif
};
#endif

#if (DEVICE_TO_PROGRAM == WATERGUN || DEVICE_TO_PROGRAM == GAMEMASTER)
/* Device context */
struct device_ctx {
	struct bt_mesh_vendor_device_modification_cli cli;
	struct bt_mesh_vendor_device_modification_srv srv;
	struct k_work_delayable work;
};
/* Device Modification Client Handlers */
static void device_modification_status(struct bt_mesh_vendor_device_modification_cli *cli, struct bt_mesh_msg_ctx *ctx,
	struct bt_mesh_vendor_playerstatus_status *rsp);
static const struct bt_mesh_vendor_device_modification_cli_handlers vendor_device_modification_cli_handlers = {
	.status = device_modification_status,
};
/* Device Modification Server Handlers*/
static void device_modification_set(struct bt_mesh_vendor_device_modification_srv *srv, struct bt_mesh_msg_ctx *ctx,
	const struct bt_mesh_vendor_device_modification_set *set,
	struct bt_mesh_vendor_playerstatus_status *rsp);
static const struct bt_mesh_vendor_device_modification_srv_handlers vendor_device_modification_srv_handlers = {
	.set = device_modification_set,
};
/* Init Device context */
static struct device_ctx device_ctx[] = {
	#if (DEVICE_TO_PROGRAM == GAMEMASTER)
	{.cli = BT_MESH_VENDOR_DEVICE_MODIFICATION_CLI_INIT(&vendor_device_modification_cli_handlers)},
	#elif (DEVICE_TO_PROGRAM == WATERGUN)
	{.srv = BT_MESH_VENDOR_DEVICE_MODIFICATION_SRV_INIT(&vendor_device_modification_srv_handlers)},
	#endif
};
#endif

#if (DEVICE_TO_PROGRAM == WATERGUN || DEVICE_TO_PROGRAM == GAMEMASTER)
/* Game context */
struct game_ctx {
	struct bt_mesh_vendor_game_modification_cli cli;
	struct bt_mesh_vendor_game_modification_srv srv;
	struct k_work_delayable work;
};
/* Game Modification Client Handlers*/
static void game_modification_status(struct bt_mesh_vendor_game_modification_cli *cli, struct bt_mesh_msg_ctx *ctx,
	struct bt_mesh_vendor_playerstatus_status *rsp);
static const struct bt_mesh_vendor_game_modification_cli_handlers vendor_game_modification_cli_handlers = {
	.status = game_modification_status,
};
/* Game Modification Server Handlers*/
static void game_modification_set(struct bt_mesh_vendor_game_modification_srv *srv, struct bt_mesh_msg_ctx *ctx,
	const struct bt_mesh_vendor_game_modification_set *set,
	struct bt_mesh_vendor_playerstatus_status *rsp);
static const struct bt_mesh_vendor_game_modification_srv_handlers vendor_game_modification_srv_handlers = {
	.set = game_modification_set,
};	
/* Init Game context */
static struct game_ctx game_ctx[] = {
	#if (DEVICE_TO_PROGRAM == GAMEMASTER)
	{.cli = BT_MESH_VENDOR_GAME_MODIFICATION_CLI_INIT(&vendor_game_modification_cli_handlers)},
	#elif (DEVICE_TO_PROGRAM == WATERGUN)
	{.srv = BT_MESH_VENDOR_GAME_MODIFICATION_SRV_INIT(&vendor_game_modification_srv_handlers)},
	#endif
};
#endif

static void heartbeat_status(struct bt_mesh_vendor_heartbeat_cli *cli, struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_playerstatus_status *status)
{
	// Received Heartbeat Status shall not trigger a ardunio interrupt as there is no indication who sent it.
	//output_pin_interrupt_set(0, 1, 0);
	//printk("Model_Handler: Set Heartbeat Interrupt Configuration for Arduino.\n");
	//output_pin_interrupt_reset();
};

static void hit_status(struct bt_mesh_vendor_hit_cli *cli, struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_playerstatus_status *status)
{
	//printk("Model_Handler: hit_status\n");
}
static void button_handler_cb(uint32_t pressed, uint32_t changed)
{
	uint8_t err;
	printk("button_handler_cb\n");
	bool target_btn_state;
	for (int i = 0; i < ARRAY_SIZE(buttons); ++i) {
		//printk("Button active\n");
		if (i==0){
			//printk("button_set\n");
			if (!(pressed & changed & BIT(i))) {
				continue;
			}
			struct bt_mesh_vendor_game_modification_set game_set = {
				.running = 1,
				.init_game_delay = 30,
				.init_game_time = 300,
			};
			// Implemented without acknowledgement during testing.
			printk("Model_Handler: Set Game Modificiation Interrupt Configuration for Arduino.");
			output_pin_interrupt_set(1, 0, 0);
			err = bt_mesh_vendor_game_modification_cli_set(&game_ctx[0].cli, NULL, &game_set, NULL);
			output_pin_interrupt_reset();
		}
	}
}
static void hit_set(struct bt_mesh_vendor_hit_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_vendor_hit_set *set,
		    struct bt_mesh_vendor_playerstatus_status *rsp)
{
	//printk("Model_Handler: hit_set\n");
	//struct hit_ctx *hit = CONTAINER_OF(srv, struct hit_ctx, srv);
	//int led_idx = hit - &hit_ctx[0];
	//printk("Updating Player status... Player_ID: %d Lifecount: %d\n", set->player_id, set->player_lifecount);

	
	output_pin_interrupt_set(0, 0, 1);
	printk("Model_Handler: Set Hit Interrupt Configuration for Arduino.\n");
	output_pin_interrupt_reset();

	if (set->player_lifecount != local_player_status.player_lifecount) {
		// led_indicate_hit(true, 1);
		local_player_status.player_lifecount = set->player_lifecount;
	}
	// led_indicate_hit(true, 2);
	
}

static void heartbeat_get(struct bt_mesh_vendor_heartbeat_srv *srv, struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_playerstatus_status *rsp)
{
	output_pin_interrupt_set(0, 1, 0);
	printk("Model_Handler: Set Heartbeat Interrupt Configuration for Arduino.\n");
	output_pin_interrupt_reset();
};

static void playerstatus_get(struct bt_mesh_vendor_playerstatus_srv *srv, struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_playerstatus_status *rsp)
{
	output_pin_interrupt_set(0, 1, 1);
	printk("Model_Handler: Set Playerstatus Interrupt Configuration for Arduino.\n");
	output_pin_interrupt_reset();
};

static void playerstatus_status(struct bt_mesh_vendor_playerstatus_cli *srv, struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_playerstatus_status *rsp)
{
	output_pin_interrupt_set(0, 1, 1);
	printk("Model_Handler: Set Playerstatus Interrupt Configuration for Arduino.\n");
	output_pin_interrupt_reset();
};

static void device_modification_status(struct bt_mesh_vendor_device_modification_cli *cli, struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_playerstatus_status *rsp)
{
	//printk("Model_Handler: device_modification_status\n");
};

static void device_modification_set(struct bt_mesh_vendor_device_modification_srv *srv, struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_vendor_device_modification_set *set,
			   struct bt_mesh_vendor_playerstatus_status *rsp)
{
	//printk("Model_Handler: device_modification_set\n");
};

static void game_modification_status(struct bt_mesh_vendor_game_modification_cli *cli, struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_playerstatus_status *rsp)
{
	//printk("Model_Handler: game_modification_status\n");
};

static void game_modification_set(struct bt_mesh_vendor_game_modification_srv *srv, struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_vendor_game_modification_set *set,
			   struct bt_mesh_vendor_playerstatus_status *rsp)
{
	output_pin_interrupt_set(1, 0, 0);
	printk("Model_Handler: Set Game Modification Interrupt Configuration for Arduino.\n");
	output_pin_interrupt_reset();

	local_player_status.game_status.running = set->running;
	local_player_status.game_status.init_game_delay = set->init_game_delay;
	local_player_status.game_status.init_game_time = set->init_game_time;

};

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
	BT_MESH_ELEM(
		1, BT_MESH_MODEL_LIST(
			BT_MESH_MODEL_CFG_SRV,
			BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub)),
			BT_MESH_MODEL_NONE),
#if (DEVICE_TO_PROGRAM == WATERGUN)
	BT_MESH_ELEM(
		2, BT_MESH_MODEL_LIST(
			BT_MESH_VENDOR_MODEL_HEARTBEAT_SRV(&heartbeat_ctx[0].srv)),
			BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(
		3, BT_MESH_MODEL_LIST(
			BT_MESH_VENDOR_MODEL_HIT_SRV(&hit_ctx[0].srv)),
			BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(
		4, BT_MESH_MODEL_LIST(
			BT_MESH_VENDOR_MODEL_PLAYERSTATUS_SRV(&playerstatus_ctx[0].srv)),
			BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(
		5, BT_MESH_MODEL_LIST(
			BT_MESH_VENDOR_MODEL_DEVICE_MODIFICATION_SRV(&device_ctx[0].srv)),
			BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(
		6, BT_MESH_MODEL_LIST(
			BT_MESH_VENDOR_MODEL_GAME_MODIFICATION_SRV(&game_ctx[0].srv)),
			BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(
		7, BT_MESH_MODEL_LIST(
	 		BT_MESH_VENDOR_MODEL_HEARTBEAT_CLI(&heartbeat_ctx[1].cli)),
			BT_MESH_MODEL_NONE),
#elif (DEVICE_TO_PROGRAM == HITDETECTION)
	//TODO: Hearbeat and Playerstatus in only used during development testing.
	BT_MESH_ELEM(
		2, BT_MESH_MODEL_LIST(
			BT_MESH_VENDOR_MODEL_HIT_CLI(&hit_ctx[0].cli)),
			BT_MESH_MODEL_NONE),
#elif (DEVICE_TO_PROGRAM == GAMEMASTER)
	BT_MESH_ELEM(
		2, BT_MESH_MODEL_LIST(
			BT_MESH_VENDOR_MODEL_PLAYERSTATUS_CLI(&playerstatus_ctx[0].cli)),
			BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(
		3, BT_MESH_MODEL_LIST(
			BT_MESH_VENDOR_MODEL_DEVICE_MODIFICATION_CLI(&device_ctx[0].cli)),
			BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(
		4, BT_MESH_MODEL_LIST(
			BT_MESH_VENDOR_MODEL_GAME_MODIFICATION_CLI(&game_ctx[0].cli)),
			BT_MESH_MODEL_NONE),
#endif
};

static const struct bt_mesh_comp comp = {
	.cid = CONFIG_BT_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

//NOTE This function is called in main.c
const struct bt_mesh_comp *model_handler_init(void)
{
	printk("Model_Handler: model_handler_init\n");
	k_work_init_delayable(&attention_blink_work, attention_blink);
	
	int err;
	for(int i = 0; i < ARRAY_SIZE(input_pins); i++)
	{
		if (!gpio_is_ready_dt(&input_pins[i])) {
			printk("Error: input pin %s is not ready\n",
				input_pins[i].port->name);
			return 0;
		} else {
			err = gpio_pin_configure_dt(&input_pins[i], GPIO_INPUT);
			if (err != 0) {
				printk("Error %d: failed to configure pin %d\n", err, i);
			} else {
				printk("Pin %d configured\n", i);
			}
		}
	}
	/* Setup of interrupt pin 0. Connected to hit signal from arduino.*/
	//TODO Add logic to only configure the pins necessary for the device selected as DEVICE_TO_PROGRAM.
	err = gpio_pin_interrupt_configure_dt(&input_pins[0], GPIO_INT_EDGE_TO_ACTIVE);
	if (err) {printk("inpu_pins[0] - interrupt_configure: %d\n", err);};
	gpio_init_callback(&input_pin0_cb, input_pin_isr, BIT(input_pins[0].pin));
	printk("input_pins[0].pin: %lu\n", BIT(input_pins[0].pin));
	gpio_add_callback(input_pins[0].port, &input_pin0_cb);

	/* Setup of interrupt pin 1. Connected to shot signal from arduino.*/
	err = gpio_pin_interrupt_configure_dt(&input_pins[1], GPIO_INT_EDGE_TO_ACTIVE);
	if (err) {printk("inpu_pins[1] - interrupt_configure: %d\n", err);};
	gpio_init_callback(&input_pin1_cb, input_pin_isr, BIT(input_pins[1].pin));
	printk("input_pins[1].pin: %lu\n", BIT(input_pins[1].pin));
	gpio_add_callback(input_pins[1].port, &input_pin1_cb);

	/* Setup of interrupt pin 2. Connected to pump signal from arduino.*/
	err = gpio_pin_interrupt_configure_dt(&input_pins[2], GPIO_INT_EDGE_TO_ACTIVE);
	if (err) {printk("inpu_pins[2] - interrupt_configure: %d\n", err);};
	gpio_init_callback(&input_pin2_cb, input_pin_isr, BIT(input_pins[2].pin));
	printk("input_pins[2].pin: %lu\n", BIT(input_pins[2].pin));
	gpio_add_callback(input_pins[2].port, &input_pin2_cb);	
	
	for(int i = 0; i < ARRAY_SIZE(output_pins); i++)
	{
		if (!gpio_is_ready_dt(&output_pins[i])) {
			printk("Error: output pin %s is not ready\n",
				output_pins[i].port->name);
			return 0;
		} else {
			err = gpio_pin_configure_dt(&output_pins[i], GPIO_OUTPUT);
			if (err != 0) {
				printk("Error %d: failed to configure pin %d\n", err, i);
			} else {
				printk("Pin %d configured\n", i);
			}
		}
	}
	
	#if DEVICE_TO_PROGRAM == GAMEMASTER
	//NOTE Initialisierung der Buttons sowie der entsprechenden Callbacks.
	//Only needed during testing.
	static struct button_handler button_handler = {
		.cb = button_handler_cb,
	};
	dk_button_handler_add(&button_handler);
	#endif

	return &comp;
}

void pin_isr_feedback(uint32_t *bit_to_pin)
{
	switch (*bit_to_pin)
	{
		case 1:
			printk("Hit Interrupt\n");
			break;
		case 2:
			printk("Shot Interrupt\n");
			printk("\tTank Level: %d/100\tShots fired: %d\n", local_player_status.tank_left, local_player_status.shots_since_init );
			break;
		case 3:
			printk("Pump Interrupt\n");
			printk("\tTank Level %d/100\n", local_player_status.tank_left);
			break;
		default:
			printk("Unknown Interrupt\n");
			break;
	}	
};

/* Main ISR for hit/shot/pump input signal from Arduino */
void input_pin_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // printk("input_pin_isr - ");

	/* Evaluating the pin mask */
	uint32_t bit_to_pin = 0;
	for (int i = 0; i < 32; i++) {
		if ((pins >> i) == 1) {
			bit_to_pin = i;
			break;
		}
	}
	/* Based on pin responsible for the interrupt, the according action is performed. */
	switch (bit_to_pin)
	{
		case 1:
			// Hit
			#if (DEVICE_TO_PROGRAM == HITDETECTION)
			if (local_player_status.player_lifecount > 0)
			{
				local_player_status.player_lifecount -= 1;
				// printk("Player Lifecount: %d\n", local_player_status.player_lifecount);
				struct bt_mesh_vendor_hit_set set = {
					.player_id = local_player_status.player_id,
					.player_lifecount = local_player_status.player_lifecount,
				};
				//TODO
				// Get rid of the button structure and add the client functionality to the input_pin structure.
				// These structure has to include all the client/server functionality of all the models used.
					int err;
					output_pin_interrupt_set(0, 0, 1);
					// Implemented without acknowledgement during testing.
					err = bt_mesh_vendor_hit_cli_set(&hit_ctx[0].cli, NULL, &set, NULL);
					//if (err) {
					//	 printk("Hit Client Set failed: %d\n", err);
					//}			
					output_pin_interrupt_reset();	
			} else {
				// printk("Player Dead\n");
				// NOTE
				// Deactivate Shot Button
				// Deactivate Pump Button
			}    
			#endif
			break;
		case 2:
			// Shot
			if (local_player_status.tank_left > TANK_LEVEL_REDUCTION_PER_SHOT)
			{
				local_player_status.shots_since_init += 1;
				local_player_status.tank_left -= TANK_LEVEL_REDUCTION_PER_SHOT;
				pin_isr_feedback(&bit_to_pin);
				// printk("Shots in Tank: %d\n", shots_in_tank);
			} else {
				// printk("Tank Empty\n");
			}
			break;
		case 3:
			// Pump
			if (local_player_status.tank_left < 100 - TANK_LEVEL_REDUCTION_PER_SHOT)
			{
				local_player_status.tank_left += TANK_LEVEL_REDUCTION_PER_SHOT;
				pin_isr_feedback(&bit_to_pin);
				// printk("Shots in Tank: %d\n", shots_in_tank);
			} else {
				// printk("Tank Empty\n");
			}
			break;
		default:
			// Unknown pin
			break;
	}
	
}

void output_pin_interrupt_reset(void)
{
	gpio_pin_set_dt(&output_pins[0], 0);
	gpio_pin_set_dt(&output_pins[1], 0);
	gpio_pin_set_dt(&output_pins[2], 0);
	gpio_pin_set_dt(&output_pins[3], 0);
}

void output_pin_interrupt_set(uint8_t pin1_status, uint8_t pin2_status, uint8_t pin3_status)
{
	gpio_pin_set_dt(&output_pins[1], pin1_status);
	gpio_pin_set_dt(&output_pins[2], pin2_status);
	gpio_pin_set_dt(&output_pins[3], pin3_status);
	gpio_pin_set_dt(&output_pins[0], 1);
}