#ifndef VENDOR_MODEL_COMMON_H
#define VENDOR_MODEL_COMMON_H

#include <string.h>
#include <zephyr/bluetooth/mesh/msg.h>
#include <zephyr/bluetooth/mesh.h>
#include <bluetooth/mesh/model_types.h>

/* START Test Model Defines
---------------------------------------------------------------- 
*/
// Select if Watergun or HitDetection will be programmed


#define WATERGUN 0
#define HITDETECTION 1
#define GAMEMASTER 2
#define TESTMODEL 3

#define DEVICE_TO_PROGRAM WATERGUN

//Define Test Model (Based on Generic OnOff Cli&Srv)
#define BT_MESH_MODEL_ID_SPYRA_TEST_MODEL_CLI 0x000F 
#define BT_MESH_MODEL_ID_SPYRA_TEST_MODEL_SRV 0x0010 
#define BT_MESH_VENDOR_TEST_MODEL_GET_OP BT_MESH_MODEL_OP_2(0x82, 0x01)
#define BT_MESH_VENDOR_TEST_MODEL_SET_OP BT_MESH_MODEL_OP_2(0x82, 0x02)
#define BT_MESH_VENDOR_TEST_MODEL_SET_UNACK_OP BT_MESH_MODEL_OP_2(0x82, 0x03)
#define BT_MESH_VENDOR_TEST_MODEL_STATUS_OP BT_MESH_MODEL_OP_2(0x82, 0x04)
// Define min. message length
#define BT_MESH_VENDOR_TEST_MODEL_MSG_GET_MIN_LEN 0
#define BT_MESH_VENDOR_TEST_MODEL_MSG_STATUS_MIN_LEN 1
#define BT_MESH_VENDOR_TEST_MODEL_MSG_SET_MIN_LEN 2
// Define max. message length
#define BT_MESH_VENDOR_TEST_MODEL_MSG_GET_MAX_LEN 0
#define BT_MESH_VENDOR_TEST_MODEL_MSG_STATUS_MAX_LEN 4
#define BT_MESH_VENDOR_TEST_MODEL_MSG_SET_MAX_LEN 4

/** Parameters for the Vendor Heartbeat Status message. */
struct bt_mesh_vendor_test_model_status {
	/** The id of the player. */
	uint8_t player_id;
	/** Player status. Alive or Dead. */
	bool player_alive;
};

/** Mandatory parameters for the Generic OnOff Set message. */
struct bt_mesh_vendor_test_model_set {
	/** State to set. */
	bool on_off;
	/** Whether this should reuse the previous transaction identifier. */
	bool reuse_transaction;
	/**
	 * Transition time parameters for the state change, or NULL.
	 *
	 * When sending, setting the transition to NULL makes the receiver use
	 * its default transition time parameters, or 0 if no default transition
	 * time is set.
	 */
	const struct bt_mesh_model_transition *transition;
};

enum {
	/** Don't use DTT if transition time is not present in set message. */
	GEN_VENDOR_TEST_MODEL_SRV_NO_DTT,
};

/* END Test Model Defines
------------------------------------------------
*/

// Define Company ID
#define SPYRA_COMPANY_ID 0x0059 //Currently, this is the nordic company id

// Define Model IDs
#define BT_MESH_MODEL_ID_SPYRA_HEARTBEAT_CLI 0x000F //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_HEARTBEAT_SRV 0x0010 //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_HIT_CLI 0x0011 //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_HIT_SRV 0x0012 //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_PLAYERSTATUS_CLI 0x0013 //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_PLAYERSTATUS_SRV 0x0014 //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_DEVICE_MODIFICATION_CLI 0x0015 //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_DEVICE_MODIFICATION_SRV 0x0016 //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_GAME_MODIFICATION_CLI 0x0017 //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_GAME_MODIFICATION_SRV 0x0018 //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_PLAYER_ID_CLI 0x0019 //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_PLAYER_ID_SRV 0x001A //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_PARTNER_ADDRESS_CLI 0x001B //According to Nordic, this model ID is not yet taken.
#define BT_MESH_MODEL_ID_SPYRA_PARTNER_ADDRESS_SRV 0x001C //According to Nordic, this model ID is not yet taken.

// Model message Opcodes
// HEARTBEAT
//#define BT_MESH_VENDOR_HEARTBEAT_GET_OP BT_MESH_MODEL_OP_3(0x12, SPYRA_COMPANY_ID)
//#define BT_MESH_VENDOR_HEARTBEAT_STATUS_OP BT_MESH_MODEL_OP_3(0x13, SPYRA_COMPANY_ID)
#define BT_MESH_VENDOR_HEARTBEAT_GET_OP BT_MESH_MODEL_OP_2(0x82, 0x05)
#define BT_MESH_VENDOR_HEARTBEAT_STATUS_OP BT_MESH_MODEL_OP_2(0x82, 0x08)

// HIT
//#define BT_MESH_VENDOR_HIT_GET_OP BT_MESH_MODEL_OP_3(0x21, SPYRA_COMPANY_ID)
//#define BT_MESH_VENDOR_HIT_SET_OP BT_MESH_MODEL_OP_3(0x14, SPYRA_COMPANY_ID)
//#define BT_MESH_VENDOR_HIT_STATUS_OP BT_MESH_MODEL_OP_3(0x15, SPYRA_COMPANY_ID)
#define BT_MESH_VENDOR_HIT_GET_OP BT_MESH_MODEL_OP_2(0x82, 0x01)
#define BT_MESH_VENDOR_HIT_SET_OP BT_MESH_MODEL_OP_2(0x82, 0x02)
#define BT_MESH_VENDOR_HIT_STATUS_OP BT_MESH_MODEL_OP_2(0x82, 0x04)

// PLAYERSTATUS
#define BT_MESH_VENDOR_PLAYERSTATUS_GET_OP BT_MESH_MODEL_OP_2(0x82, 0x23)
#define BT_MESH_VENDOR_PLAYERSTATUS_STATUS_OP BT_MESH_MODEL_OP_2(0x82, 0x24)
//#define BT_MESH_VENDOR_PLAYERSTATUS_GET_OP BT_MESH_MODEL_OP_3(0x16, SPYRA_COMPANY_ID)
//#define BT_MESH_VENDOR_PLAYERSTATUS_STATUS_OP BT_MESH_MODEL_OP_3(0x17, SPYRA_COMPANY_ID)

// DEVICE MODIFICATION
#define BT_MESH_VENDOR_DEVICE_MODIFICATION_GET_OP BT_MESH_MODEL_OP_2(0x82, 0x91)
#define BT_MESH_VENDOR_DEVICE_MODIFICATION_SET_OP BT_MESH_MODEL_OP_2(0x82, 0x92)
#define BT_MESH_VENDOR_DEVICE_MODIFICATION_STATUS_OP BT_MESH_MODEL_OP_2(0x82, 0x94)
//#define BT_MESH_VENDOR_DEVICE_MODIFICATION_GET_OP BT_MESH_MODEL_OP_3(0x18, SPYRA_COMPANY_ID)
//#define BT_MESH_VENDOR_DEVICE_MODIFICATION_SET_OP BT_MESH_MODEL_OP_3(0x19, SPYRA_COMPANY_ID)
//#define BT_MESH_VENDOR_DEVICE_MODIFICATION_STATUS_OP BT_MESH_MODEL_OP_3(0x1A, SPYRA_COMPANY_ID)

// GAME MODIFICATION
#define BT_MESH_VENDOR_GAME_MODIFICATION_SET_OP BT_MESH_MODEL_OP_2(0x82, 96)
#define BT_MESH_VENDOR_GAME_MODIFICATION_STATUS_OP BT_MESH_MODEL_OP_2(0x82, 98)
//#define BT_MESH_VENDOR_GAME_MODIFICATION_SET_OP BT_MESH_MODEL_OP_3(0x1B, SPYRA_COMPANY_ID)
//#define BT_MESH_VENDOR_GAME_MODIFICATION_STATUS_OP BT_MESH_MODEL_OP_3(0x1C, SPYRA_COMPANY_ID)

// PLAYER ID
//#define BT_MESH_VENDOR_PLAYER_ID_SET_OP BT_MESH_MODEL_OP_3(0x1D, SPYRA_COMPANY_ID)
//#define BT_MESH_VENDOR_PLAYER_ID_STATUS_OP BT_MESH_MODEL_OP_3(0x1E, SPYRA_COMPANY_ID)

// PARTNERADDRESS
//#define BT_MESH_VENDOR_PARTNER_ADDRESS_SET_OP BT_MESH_MODEL_OP_3(0x1F, SPYRA_COMPANY_ID)
//#define BT_MESH_VENDOR_PARTNER_ADDRESS_STATUS_OP BT_MESH_MODEL_OP_3(0x20, SPYRA_COMPANY_ID)

//Define minimum message length in bytes
#define BT_MESH_VENDOR_HEARTBEAT_MSG_GET_MIN_LEN 0
#define BT_MESH_VENDOR_HEARTBEAT_MSG_STATUS_MIN_LEN 1
#define BT_MESH_VENDOR_HIT_MSG_GET_MIN_LEN 0
#define BT_MESH_VENDOR_HIT_MSG_SET_MIN_LEN 2
#define BT_MESH_VENDOR_HIT_MSG_STATUS_MIN_LEN 2
#define BT_MESH_VENDOR_PLAYERSTATUS_MSG_GET_MIN_LEN 0
#define BT_MESH_VENDOR_PLAYERSTATUS_MSG_STATUS_MIN_LEN 0
#define BT_MESH_VENDOR_DEVICE_MODIFICATION_MSG_GET_MIN_LEN 0
#define BT_MESH_VENDOR_DEVICE_MODIFICATION_MSG_SET_MIN_LEN 3
#define BT_MESH_VENDOR_DEVICE_MODIFICATION_MSG_STATUS_MIN_LEN 3
#define BT_MESH_VENDOR_GAME_MODIFICATION_MSG_SET_MIN_LEN 3
#define BT_MESH_VENDOR_GAME_MODIFICATION_MSG_STATUS_MIN_LEN 3
// #define BT_MESH_VENDOR_PLAYER_ID_MSG_SET_MIN_LEN 1
// #define BT_MESH_VENDOR_PLAYER_ID_MSG_STATUS_MIN_LEN 1
// #define BT_MESH_VENDOR_PARTNER_ADDRESS_MSG_SET_MIN_LEN 3
// #define BT_MESH_VENDOR_PARTNER_ADDRESS_MSG_STATUS_MIN_LEN 3

// Define maximum message length in bytes
#define BT_MESH_VENDOR_HEARTBEAT_MSG_GET_MAX_LEN 0
#define BT_MESH_VENDOR_HEARTBEAT_MSG_STATUS_MAX_LEN 2
#define BT_MESH_VENDOR_HIT_MSG_GET_MAX_LEN 0
#define BT_MESH_VENDOR_HIT_MSG_SET_MAX_LEN 3
#define BT_MESH_VENDOR_HIT_MSG_STATUS_MAX_LEN 3
#define BT_MESH_VENDOR_PLAYERSTATUS_MSG_GET_MAX_LEN 2
#define BT_MESH_VENDOR_PLAYERSTATUS_MSG_STATUS_MAX_LEN 4
#define BT_MESH_VENDOR_DEVICE_MODIFICATION_MSG_GET_MAX_LEN 0
#define BT_MESH_VENDOR_DEVICE_MODIFICATION_MSG_SET_MAX_LEN 3
#define BT_MESH_VENDOR_DEVICE_MODIFICATION_MSG_STATUS_MAX_LEN 3
#define BT_MESH_VENDOR_GAME_MODIFICATION_MSG_SET_MAX_LEN 5
#define BT_MESH_VENDOR_GAME_MODIFICATION_MSG_STATUS_MAX_LEN 5
// #define BT_MESH_VENDOR_PLAYER_ID_MSG_SET_MAX_LEN 1
// #define BT_MESH_VENDOR_PLAYER_ID_MSG_STATUS_MAX_LEN 1
// #define BT_MESH_VENDOR_PARTNER_ADDRESS_MSG_SET_MAX_LEN 3
// #define BT_MESH_VENDOR_PARTNER_ADDRESS_MSG_STATUS_MAX_LEN 3


//Define human readable PlayerIDs
#define TEAM_BLUE_PLAYER_ID_1 0b00000000
#define TEAM_BLUE_PLAYER_ID_2 0b00000001
#define TEAM_BLUE_PLAYER_ID_3 0b00000010
#define TEAM_BLUE_PLAYER_ID_4 0b00000011
#define TEAM_BLUE_PLAYER_ID_5 0b00000100
#define TEAM_BLUE_PLAYER_ID_6 0b00000101
#define TEAM_BLUE_PLAYER_ID_7 0b00000110
#define TEAM_BLUE_PLAYER_ID_8 0b00000111
#define TEAM_RED_PLAYER_ID_1 0b10000000
#define TEAM_RED_PLAYER_ID_2 0b10000001
#define TEAM_RED_PLAYER_ID_3 0b10000010
#define TEAM_RED_PLAYER_ID_4 0b10000011
#define TEAM_RED_PLAYER_ID_5 0b10000100
#define TEAM_RED_PLAYER_ID_6 0b10000101
#define TEAM_RED_PLAYER_ID_7 0b10000110
#define TEAM_RED_PLAYER_ID_8 0b10000111

//Define player life enum
enum player_life_enum {
    PLAYER_LIFE_0,
    PLAYER_LIFE_1,
    PLAYER_LIFE_2,
    PLAYER_LIFE_3,
    PLAYER_LIFE_4,
    PLAYER_LIFE_5,
};

//Define player life enum
enum game_status_enum {
    GAME_PAUSED,
    GAME_RUNNING,
};

//Define player life enum
enum device_status_enum {
    DEVICE_DISABLED,
    DEVICE_ENABLED,
};

//Define player life enum
enum game_mode_enum {
    GAME_MODE_LEAGUE,
    GAME_MODE_BURST,
    GAME_MODE_AUTOMATIC
};

//Define max. number of shots per tank
#define MAX_SHOTS_PER_TANK 24

//Define tank level reduction per shot
#define TANK_LEVEL_REDUCTION_PER_SHOT 4

struct bt_mesh_vendor_hit_set {
    /* Player ID */
    uint8_t player_id;
	/** State to set. */
	uint8_t player_lifecount;
	/** Whether this should reuse the previous transaction identifier. */
    //NOTE We omitt this parameter for the time being.
	bool reuse_transaction;
};

struct bt_mesh_vendor_device_modification_set {
    /* Device status which contains the device state (active/deactive) as well as the selected game mode.*/
    bool active;
    uint8_t game_mode;
	/** Whether this should reuse the previous transaction identifier. */
    //NOTE We omitt this parameter for the time being.
	bool reuse_transaction;
};

struct bt_mesh_vendor_game_modification_set {
    /* Status of the game*/
    bool running;
    /* Initial game time in seconds. This is used to set the timer to automatically end the game*/
    uint16_t init_game_time;
    /* Initial game delay in seconds. The device waits for this time befor starting the game for the duration of init_game_time*/
    uint8_t init_game_delay;
	/** Whether this should reuse the previous transaction identifier. */
    //NOTE We omitt this parameter for the time being.
	bool reuse_transaction;
};

struct battery_status {
    /* Battery level of the Hit Detection given in percent with a 1% resolution. (0%-100%)*/
    unsigned int battery_level_hit_detection : 4;
    /* Battery level of the watergun given in percent with a 1% resolution (0%-100%)*/
    unsigned int  battery_level_watergun : 4;
};

struct device_status {
    /* Device status which contains the device state (active/deactive) as well as the selected game mode.*/
    bool active;
    uint8_t game_mode;
};

struct game_status {
    /* Status of the game*/
    bool running;
    /* Initial game time in seconds. This is used to set the timer to automatically end the game*/
    uint16_t init_game_time;
    /* Initial game delay in seconds. The device waits for this time befor starting the game for the duration of init_game_time*/
    uint8_t init_game_delay;
};

/* Stores the state of the local player.*/
struct bt_mesh_vendor_playerstatus_status {
    /* Player ID, bit encoded. Use defined macro, e.g. TEAM_BLUE_PLAYER_ID_1*/
    uint8_t player_id;
    /* Player remaining lifes. Use defined macro, e.g. PLAYER_LIFE_0*/
    uint8_t player_lifecount;
    /* Player tank left. This value calculated by the number of shots left. With 24 shots per tank, each shot adds 4*/
    uint8_t tank_left;
    /* Player water tanks used since game start. This value is updated by the main MCU.*/
    uint8_t tanks_since_init;
    /* Player battery status. This value is updated by the main MCU.*/
    struct battery_status battery_status;
    /* Device status which contains the device state (active/deactive) as well as the selected game mode.*/
    struct device_status device_status;
    /* INTERNAL ONLY - Shots since game start*/
    uint16_t shots_since_init;
    /* INTERNAL ONLY - State of the game*/
    struct game_status game_status;
};

extern struct bt_mesh_vendor_playerstatus_status local_player_status;
extern enum tank_levels tank_levels;
extern enum battery_levels battery_levels;
extern enum player_life_enum player_life_enum;

#endif