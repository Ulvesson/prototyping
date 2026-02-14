// fd_rpc.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>

constexpr uint32_t MAX_WORKERS = 4;
typedef struct _FD_RPC {
	uint32_t frame_no;	// Frame number
	uint32_t workers[MAX_WORKERS];
} FD_RPC, * PFD_RPC;
