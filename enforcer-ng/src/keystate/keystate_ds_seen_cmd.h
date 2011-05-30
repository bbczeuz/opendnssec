#ifndef _KEYSTATE_DS_SEEN_CMD_H_
#define _KEYSTATE_DS_SEEN_CMD_H_

#include "daemon/engine.h"

void help_keystate_ds_seen_cmd(int sockfd);

int handled_keystate_ds_seen_cmd(int sockfd, engine_type* engine,
                                 const char *cmd, ssize_t n);

#endif