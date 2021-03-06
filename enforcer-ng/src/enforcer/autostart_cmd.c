/*
 * Copyright (c) 2011 Surfnet 
 * Copyright (c) 2011 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2011 OpenDNSSEC AB (svb)
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

#include "config.h"

#include "daemon/engine.h"
#include "enforcer/enforce_task.h"
#include "policy/policy_resalt_task.h"
#include "shared/status.h"
#include "shared/log.h"
#include "hsmkey/hsm_key_factory.h"

#include "enforcer/autostart_cmd.h"

static const char *module_str = "autostart_cmd";

void
autostart(engine_type* engine)
{
	ods_status status;

	ods_log_debug("[%s] autostart", module_str);

	schedule_purge(engine->taskq); /* Remove old tasks in queue */

	hsm_key_factory_schedule_generate_all(engine, 0);
	status = schedule_task(engine->taskq, policy_resalt_task(engine));
	if (status != ODS_STATUS_OK)
		ods_log_crit("[%s] failed to create resalt task", module_str);
	
	status = schedule_task(engine->taskq, enforce_task(engine, 1));
    if (status != ODS_STATUS_OK)
        ods_fatal_exit("[%s] failed to create enforce task", module_str);
}
