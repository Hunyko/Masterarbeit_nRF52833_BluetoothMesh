/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file
 * @brief Model handler
 */

#ifndef TEST_MODEL_HANDLER_H__
#define TEST_MODEL_HANDLER_H__

#include <zephyr/bluetooth/mesh.h>

#ifdef __cplusplus
extern "C" {
#endif

const struct bt_mesh_comp *test_model_handler_init(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_MODEL_HANDLER_H__ */
