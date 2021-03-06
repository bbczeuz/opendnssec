/*
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "daemon/engine.h"
#include "daemon/cmdhandler.h"
#include "shared/log.h"
#include "shared/str.h"
#include "daemon/clientpipe.h"
#include "utils/kc_helper.h"

#include "enforcer/update_conf_cmd.h"

#include <pthread.h>

static const char *module_str = "update_conf_cmd";

static void
usage(int sockfd)
{
	client_printf(sockfd,
		"update conf            Update the configuration from conf.xml and\n"
	    "                       reload the Enforcer.\n"
	);
}

static void
help(int sockfd)
{
    client_printf(sockfd,
        "Update the configuration from conf.xml and reload the Enforcer.\n"
    );
}

static int
handles(const char *cmd, ssize_t n)
{
	return ods_check_command(cmd, n, update_conf_funcblock()->cmdname) ? 1 : 0;
}

static int
run(int sockfd, engine_type* engine, const char *cmd, ssize_t n,
	db_connection_t *dbconn)
{
    char *kasp = NULL;
    char *zonelist = NULL;
    char **repositories = NULL;
    int repository_count = 0;
    int i;

    (void)cmd; (void)n; (void)dbconn;

	ods_log_debug("[%s] %s command", module_str, update_conf_funcblock()->cmdname);

    if (check_conf(engine->config->cfg_filename, &kasp, &zonelist, &repositories, &repository_count, 0)) {
        client_printf_err(sockfd, "Unable to validate '%s' consistency.",
            engine->config->cfg_filename);

        free(kasp);
        free(zonelist);
        if (repositories) {
            for (i = 0; i < repository_count; i++) {
                free(repositories[i]);
            }
            free(repositories);
        }
        return 1;
    }

    free(kasp);
    free(zonelist);
    if (repositories) {
        for (i = 0; i < repository_count; i++) {
            free(repositories[i]);
        }
        free(repositories);
    }

    engine->need_to_reload = 1;
    pthread_cond_signal(&engine->signal_cond);

    return 0;
}

static struct cmd_func_block funcblock = {
	"update conf", &usage, &help, &handles, &run
};

struct cmd_func_block*
update_conf_funcblock(void)
{
	return &funcblock;
}
