#
# Copyright (c) 2019 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app
	VERSION 0.1
	DESCRIPTION "A bluetooth mesh integration for a hightech water gun arena."
	LANGUAGES C)


include_directories(
	${ZEPHYR_NRF_MODULE_DIR}/samples/bluetooth/mesh/common
)

# NORDIC SDK APP START
target_sources(app PRIVATE
	src/main.c
	src/model_handler.c
	#src/vendor_test_model_cli.c
	#src/vendor_test_model_srv.c
	src/vendor_hit_cli.c
	src/vendor_hit_srv.c
	src/vendor_heartbeat_srv.c
	src/vendor_heartbeat_cli.c
	src/vendor_playerstatus_cli.c
	src/vendor_playerstatus_srv.c
	src/vendor_device_modification_cli.c
	src/vendor_device_modification_srv.c
	src/vendor_game_modification_cli.c
	src/vendor_game_modification_srv.c
	)
target_include_directories(app PRIVATE
	include
	${ZEPHYR_NRF_MODULE_DIR}/samples/bluetooth/mesh/common
)

# Preinitialization related to DFU over SMP for nRF52 series
if(CONFIG_SOC_SERIES_NRF52X)
  target_sources_ifdef(CONFIG_MCUMGR_TRANSPORT_BT app PRIVATE
    ${ZEPHYR_NRF_MODULE_DIR}/samples/bluetooth/mesh/common/smp_bt.c)
endif()

# NORDIC SDK APP END
