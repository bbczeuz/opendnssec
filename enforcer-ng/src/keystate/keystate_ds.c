/*
 * Copyright (c) 2014 Stichting NLnet Labs
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

#include <sys/stat.h>

#include "daemon/cmdhandler.h"
#include "daemon/engine.h"
#include "enforcer/enforce_task.h"
#include "shared/file.h"
#include "shared/log.h"
#include "shared/str.h"
#include "daemon/clientpipe.h"
#include "shared/duration.h"
#include "db/key_data.h"
#include "db/zone.h"
#include "db/db_error.h"
#include "db/hsm_key.h"
#include "libhsm.h"
#include "libhsmdns.h"

#include "keystate/keystate_ds.h"

static const char *module_str = "keystate_ds_x_cmd";

/** Retrieve KEY from HSM, should only be called for DNSKEYs
 * @param id, locator of DNSKEY on HSM
 * @param zone, name of zone key belongs to
 * @param algorithm, alg of DNSKEY
 * @param ttl, ttl DS should get. if 0 DNSKEY_TTL is used.
 * @return RR on succes, NULL on error */
static ldns_rr *
get_dnskey(const char *id, const char *zone, int alg, uint32_t ttl)
{
	libhsm_key_t *key;
	hsm_sign_params_t *sign_params;
	ldns_rr *dnskey_rr;
	/* Code to output the DNSKEY record  (stolen from hsmutil) */
	hsm_ctx_t *hsm_ctx = hsm_create_context();
	if (!hsm_ctx) {
		ods_log_error("[%s] Could not connect to HSM", module_str);
		return NULL;
	}
	if (!(key = hsm_find_key_by_id(hsm_ctx, id))) {
		hsm_destroy_context(hsm_ctx);
		return NULL;
	}

	/* Sign params only need to be kept around 
	 * for the hsm_get_dnskey() call. */
	sign_params = hsm_sign_params_new();
	sign_params->owner = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_DNAME, zone);
	sign_params->algorithm = (ldns_algorithm) alg;
	sign_params->flags = LDNS_KEY_ZONE_KEY | LDNS_KEY_SEP_KEY;
		
	/* Get the DNSKEY record */
	dnskey_rr = hsm_get_dnskey(hsm_ctx, key, sign_params);

	libhsm_key_free(key);
	hsm_sign_params_free(sign_params);
	hsm_destroy_context(hsm_ctx);
	
	/* Override the TTL in the dnskey rr */
	if (ttl) ldns_rr_set_ttl(dnskey_rr, ttl);
	
	return dnskey_rr;
}


static int
exec_dnskey_by_id(int sockfd, key_data_t *key, const char* ds_command,
	const char* action)
{
	ldns_rr *dnskey_rr;
	int ttl = 0, status, i;
	const char *locator;
	char *rrstr, *chrptr;
	zone_t* zone;
	struct stat stat_ret;

	assert(key);

	zone = key_data_get_zone(key);
	key_data_cache_hsm_key(key);
	locator = hsm_key_locator(key_data_hsm_key(key));
	if (!locator) return 1;
	/* This fetches the states from the DB, I'm only assuming they get
	 * cleaned up when 'key' is cleaned(?) */
	if (key_data_cache_key_states(key) != DB_OK)
		return 1;

	ttl = key_state_ttl(key_data_cached_dnskey(key));

	dnskey_rr = get_dnskey(locator, zone_name(zone), key_data_algorithm(key), ttl);
	if (!dnskey_rr) return 2;

	rrstr = ldns_rr2str(dnskey_rr);

	/* Replace tab with white-space */
	for (i = 0; rrstr[i]; ++i) {
		if (rrstr[i] == '\t') rrstr[i] = ' ';
	}

	/* We need to strip off trailing comments before we send
	 to any clients that might be listening */
	if ((chrptr = strchr(rrstr, ';'))) {
		chrptr[0] = '\n';
		chrptr[1] = '\0';
	}

	if (!ds_command || ds_command[0] == '\0') {
		ods_log_error_and_printf(sockfd, module_str, 
			"No \"DelegationSigner%sCommand\" "
			"configured.", action);
		status = 1;
	} else if (stat(ds_command, &stat_ret) != 0) {
		ods_log_error_and_printf(sockfd, module_str,
			"Cannot stat file %s: %s", ds_command,
			strerror(errno));
		status = 2;
	} else if (S_ISREG(stat_ret.st_mode) && 
			!(stat_ret.st_mode & S_IXUSR || 
			  stat_ret.st_mode & S_IXGRP || 
			  stat_ret.st_mode & S_IXOTH)) {
		/* Then see if it is a regular file, then if usr, grp or 
		 * all have execute set */
		status = 3;
		ods_log_error_and_printf(sockfd, module_str,
			"File %s is not executable", ds_command);
	} else {
		/* send records to the configured command */
		FILE *fp = popen(ds_command, "w");
		if (fp == NULL) {
			status = 4;
			ods_log_error_and_printf(sockfd, module_str,
				"failed to run command: %s: %s",ds_command,
				strerror(errno));
		} else {
			int bytes_written = fprintf(fp, "%s", rrstr);
			if (bytes_written < 0) {
				status = 5;
				ods_log_error_and_printf(sockfd,  module_str,
					 "[%s] Failed to write to %s: %s", ds_command,
					 strerror(errno));
			} else if (pclose(fp) == -1) {
				status = 6;
				ods_log_error_and_printf(sockfd, module_str,
					"failed to close %s: %s", ds_command,
					strerror(errno));
			} else {
				client_printf(sockfd, "key %sed to %s\n",
					action, ds_command);
				status = 0;
			}
		}
	}
	LDNS_FREE(rrstr);
	ldns_rr_free(dnskey_rr);
	return status;
}

static int
submit_dnskey_by_id(int sockfd, key_data_t *key, engine_type* engine)
{
	const char* ds_submit_command;
	ds_submit_command = engine->config->delegation_signer_submit_command;
	return exec_dnskey_by_id(sockfd, key, ds_submit_command, "submit");
}

static int
retract_dnskey_by_id(int sockfd, key_data_t *key, engine_type* engine)
{
	const char* ds_retract_command;
	ds_retract_command = engine->config->delegation_signer_retract_command;
	return exec_dnskey_by_id(sockfd, key, ds_retract_command, "retract");
}

static int
ds_list_keys(db_connection_t *dbconn, int sockfd,
	key_data_ds_at_parent_t state)
{
	const char *fmth = "%-31s %-13s %-13s %-40s\n";
	const char *fmtl = "%-31s %-13s %-13u %-40s\n";

	key_data_list_t *key_list;
	const key_data_t *key;
	zone_t *zone = NULL;
	hsm_key_t* hsmkey = NULL;
	db_clause_list_t* clause_list;
	db_clause_t* clause;

	if (!(key_list = key_data_list_new(dbconn))
		|| !(clause_list = db_clause_list_new()))
	{
		key_data_list_free(key_list);
		return 10;
	}
	if (!(clause = key_data_role_clause(clause_list, KEY_DATA_ROLE_ZSK))
		|| db_clause_set_type(clause, DB_CLAUSE_NOT_EQUAL)
		|| !(clause = key_data_ds_at_parent_clause(clause_list, state))
		|| db_clause_set_type(clause, DB_CLAUSE_EQUAL))
	{
		key_data_list_free(key_list);
		db_clause_list_free(clause_list);
		return 11;
	}
	if (key_data_list_get_by_clauses(key_list, clause_list)) {
		key_data_list_free(key_list);
		db_clause_list_free(clause_list);
		return 12;
	}
	db_clause_list_free(clause_list);

	client_printf(sockfd, fmth, "Zone:", "Key role:", "Keytag:", "Id:");

	for (key = key_data_list_next(key_list); key;
		key = key_data_list_next(key_list))
	{
		zone = key_data_get_zone(key);
		hsmkey = key_data_get_hsm_key(key);
		client_printf(sockfd, fmtl,
			(zone ? zone_name(zone) : "NOT_FOUND"),
			key_data_role_text(key), key_data_keytag(key),
			(hsmkey ? hsm_key_locator(hsmkey) : "NOT_FOUND")
		);
		zone_free(zone);
		hsm_key_free(hsmkey);
	}
	key_data_list_free(key_list);
	return 0;
}

static int
push_clauses(db_clause_list_t *clause_list, zone_t *zone,
	key_data_ds_at_parent_t state_from, const hsm_key_t* hsmkey, int keytag)
{
	db_clause_t* clause;

	if (!key_data_zone_id_clause(clause_list, zone_id(zone)))
		return 1;
	if (!(clause = key_data_role_clause(clause_list, KEY_DATA_ROLE_ZSK)) ||
			db_clause_set_type(clause, DB_CLAUSE_NOT_EQUAL))
		return 1;
	if (!key_data_ds_at_parent_clause(clause_list, state_from))
		return 1;
	/* filter in id and or keytag conditionally. */
	if (hsmkey) {
		if (hsmkey && !key_data_hsm_key_id_clause(clause_list, hsm_key_id(hsmkey)))
			return 1;
	} else {
		if (keytag < 0 || !key_data_keytag_clause(clause_list, keytag))
			return 1;
	}
	return 0;
}

/* Change DS state, when zonename not given do it for all zones!
 */
int
change_keys_from_to(db_connection_t *dbconn, int sockfd,
	const char *zonename, const hsm_key_t* hsmkey, int keytag,
	key_data_ds_at_parent_t state_from, key_data_ds_at_parent_t state_to,
	engine_type *engine)
{
	key_data_list_t *key_list = NULL;
	key_data_t *key;
	zone_t *zone = NULL;
	int status = 0, key_match = 0, key_mod = 0;
	db_clause_list_t* clause_list = NULL;
	db_clause_t* clause = NULL;

	if (zonename) {
		if (!(key_list = key_data_list_new(dbconn)) ||
			!(clause_list = db_clause_list_new()) ||
			!(zone = zone_new_get_by_name(dbconn, zonename)) ||
			push_clauses(clause_list, zone, state_from, hsmkey, keytag) ||
			key_data_list_get_by_clauses(key_list, clause_list))
		{
			key_data_list_free(key_list);
			db_clause_list_free(clause_list);
			zone_free(zone);
			ods_log_error("[%s] Error fetching from database", module_str);
			return 10;
		}
		db_clause_list_free(clause_list);
	} else {
		/* Select all KSKs */
		if (!(clause_list = db_clause_list_new()) ||
			!key_data_ds_at_parent_clause(clause_list, state_from) ||
			!(clause = key_data_role_clause(clause_list, KEY_DATA_ROLE_ZSK)) ||
			db_clause_set_type(clause, DB_CLAUSE_NOT_EQUAL) != DB_OK ||
			!(key_list = key_data_list_new_get_by_clauses(dbconn, clause_list)))
		{
			key_data_list_free(key_list);
			db_clause_list_free(clause_list);
			ods_log_error("[%s] Error fetching from database", module_str);
			return 14;
		}
		db_clause_list_free(clause_list);
	}
	while ((key = key_data_list_get_next(key_list))) {
		key_match++;
		/* if from is submit also exec dsSubmit command? */
		if (state_from == KEY_DATA_DS_AT_PARENT_SUBMIT &&
			state_to == KEY_DATA_DS_AT_PARENT_SUBMITTED)
		{
			(void)submit_dnskey_by_id(sockfd, key, engine);
		} else if (state_from == KEY_DATA_DS_AT_PARENT_RETRACT &&
			state_to == KEY_DATA_DS_AT_PARENT_RETRACTED)
		{
			(void)retract_dnskey_by_id(sockfd, key, engine);
		}
		if (key_data_set_ds_at_parent(key, state_to) ||
			key_data_update(key))
		{
			key_data_free(key);
			ods_log_error("[%s] Error writing to database", module_str);
			client_printf(sockfd, "[%s] Error writing to database", module_str);
			status = 12;
			break;
		}
		key_mod++;
		key_data_free(key);
	}
	key_data_list_free(key_list);

	client_printf(sockfd, "%d KSK matches found.\n", key_match);
	if (!key_match) status = 11;
	client_printf(sockfd, "%d KSKs changed.\n", key_mod);

	zone_free(zone);
	return status;
}

static int
get_args(const char *cmd, ssize_t n, const char **zone,
	const char **cka_id, int *keytag, char *buf)
{

	#define NARGV 16
	const char *argv[NARGV], *tag;
	int argc;
	(void)n;

	*keytag = -1;
	*zone = NULL;
	*cka_id = NULL;
	tag = NULL;
	
	strncpy(buf, cmd, ODS_SE_MAXLINE);
	argc = ods_str_explode(buf, NARGV, argv);
	buf[sizeof(buf)-1] = '\0';
	if (argc > NARGV) {
		ods_log_warning("[%s] too many arguments for %s command",
			module_str, cmd);
		return 1;
	}
	
	(void)ods_find_arg_and_param(&argc, argv, "zone", "z", zone);
	(void)ods_find_arg_and_param(&argc, argv, "cka_id", "k", cka_id);
	(void)ods_find_arg_and_param(&argc, argv, "keytag", "x", &tag);

	if (tag) {
		*keytag = atoi(tag);
		if (*keytag < 0 || *keytag >= 65536) {
			ods_log_warning("[%s] value \"%s\" for --keytag is invalid",
				module_str, *keytag);
			return 1;
		}
	}
	return 0;
}

int
run_ds_cmd(int sockfd, const char *cmd, ssize_t n,
	db_connection_t *dbconn, key_data_ds_at_parent_t state_from,
	key_data_ds_at_parent_t state_to, engine_type *engine)
{
	const char *zone, *cka_id;
	int keytag;
	hsm_key_t* hsmkey = NULL;
	int ret;
	char buf[ODS_SE_MAXLINE];

	if (get_args(cmd, n, &zone, &cka_id, &keytag, buf)) {
		client_printf(sockfd, "Error parsing arguments\n", keytag);
		return -1;
	}
	
	if (!zone && !cka_id && keytag == -1) {
		return ds_list_keys(dbconn, sockfd, state_from);
	}
	if (!(zone && ((cka_id && keytag == -1) || (!cka_id && keytag != -1))))
	{
		ods_log_warning("[%s] expected --zone and either --cka_id or "
			"--keytag option", module_str);
		client_printf(sockfd, "expected --zone and either --cka_id or "
			"--keytag option.\n");
		return -1;
	}
	
	if (cka_id) {
		if (!(hsmkey = hsm_key_new_get_by_locator(dbconn, cka_id))) {
			client_printf_err(sockfd, "CKA_ID %s can not be found!\n", cka_id);
		}
	}

	ret = change_keys_from_to(dbconn, sockfd, zone, hsmkey, keytag,
		state_from, state_to, engine);
	hsm_key_free(hsmkey);
	return ret;
}
