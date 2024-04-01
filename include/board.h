#ifndef BOARD_H
#define BOARD_H


#include "vendor_model_common.h"

/*
    NODE_NODE_ADDR_SELFADDR - Address of the node
    NODE_ADDR_TARGET - Address of the target node (for publication etc.)
    MOD_VND_ID - Model ID 

*/

#if PROGRAMM_CLIENT
#if !defined(NODE_ADDR_SELF)
#define NODE_ADDR_SELF 0x0b0c
#endif
#define NODE_ADDR_TARGET 0x0b0d
#define MOD_VND_ID BT_MESH_MODEL_ID_SPYRA_HIT_CLI

#define GROUP_ADDR 0xc000
#define PUBLISHER_ADDR  0x000f
#else
#if !defined(NODE_ADDR_SELF)
#define NODE_ADDR_SELF 0x0b0d
#endif
#define NODE_ADDR_TARGET 0x0b0c
#define MOD_VND_ID BT_MESH_MODEL_ID_SPYRA_HIT_SRV

#define GROUP_ADDR 0xc000
#define PUBLISHER_ADDR  0x000f
#endif

#define HIT_PIN DT_ALIAS(input0)
#define SHOT_PIN DT_ALIAS(input1)
#define PUMP_PIN DT_ALIAS(input2)

#define INTERRUPT_PIN DT_ALIAS(output0)
#define INTERRUPT_DECODER_MSB DT_ALIAS(output1)
#define INTERRUPT_DECODER_2 DT_ALIAS(output2)
#define INTERRUPT_DECODER_LSB DT_ALIAS(output3)

#endif

static inline int board_init(uint16_t *addr)
{
	*addr = NODE_ADDR_SELF;
	return 0;
}