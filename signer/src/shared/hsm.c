/*
 * Copyright (c) 2009 NLNet Labs. All rights reserved.
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

/**
 * Hardware Security Module support.
 *
 */

#include "daemon/engine.h"
#include "shared/hsm.h"
#include "shared/log.h"

static const char* hsm_str = "hsm";


/**
 * Open HSM.
 *
 */
int
lhsm_open(hsm_repository_t* rlist)
{
    int result = hsm_open2(rlist, hsm_check_pin);
    if (result != HSM_OK) {
        char* error =  hsm_get_error(NULL);
        if (error != NULL) {
            ods_log_error("[%s] %s", hsm_str, error);
            free(error);
        } else {
            ods_log_crit("[%s] error opening libhsm (errno %i)", hsm_str,
                result);
        }
        /* exit? */
    } else {
        ods_log_info("[%s] libhsm connection opened succesfully", hsm_str);
    }
    return result;
}


/**
 * Reopen HSM.
 *
 */
int
lhsm_reopen(hsm_repository_t* rlist)
{
    if (hsm_check_context(NULL) != HSM_OK) {
        ods_log_warning("[%s] idle libhsm connection, trying to reopen",
            hsm_str);
        hsm_close();
        return lhsm_open(rlist);
    }
    return HSM_OK;
}


/**
 * Clear key cache.
 *
 */
static void
lhsm_clear_key_cache(key_type* key)
{
    if (!key) {
        return;
    }
    if (key->dnskey) {
        /* DNSKEY still exists in zone */
        key->dnskey = NULL;
    }
    if (key->hsmkey) {
        libhsm_key_free(key->hsmkey);
        key->hsmkey = NULL;
    }
    if (key->params) {
        hsm_sign_params_free(key->params);
        key->params = NULL;
    }
    return;
}


/**
 * Check the HSM connection, reload engine if necessary.
 *
 */
void
lhsm_check_connection(void* engine)
{
    engine_type* e = (engine_type*) engine;
    if (hsm_check_context(NULL) != HSM_OK) {
        ods_log_warning("[%s] idle libhsm connection, trying to reopen",
            hsm_str);
        engine_stop_drudgers(e);
        hsm_close();
        (void)lhsm_open(e->config->repositories);
        engine_start_drudgers((engine_type*) engine);
    } else {
        ods_log_debug("[%s] libhsm connection ok", hsm_str);
    }
    return;
}


/**
 * Get key from one of the HSMs.
 *
 */
ods_status
lhsm_get_key(hsm_ctx_t* ctx, ldns_rdf* owner, key_type* key_id)
{
    char *error = NULL;
    int retries = 0;

    if (!owner || !key_id) {
        ods_log_error("[%s] unable to get key: missing required elements",
            hsm_str);
        return ODS_STATUS_ASSERT_ERR;
    }

llibhsm_key_start:

    /* set parameters */
    if (!key_id->params) {
        key_id->params = hsm_sign_params_new();
        if (key_id->params) {
            key_id->params->owner = ldns_rdf_clone(owner);
            key_id->params->algorithm = key_id->algorithm;
            key_id->params->flags = key_id->flags;
        } else {
            /* could not create params */
            error = hsm_get_error(ctx);
            if (error) {
                ods_log_error("[%s] %s", hsm_str, error);
                free((void*)error);
            } else if (!retries) {
                lhsm_clear_key_cache(key_id);
                retries++;
                goto llibhsm_key_start;
           }
            ods_log_error("[%s] unable to get key: create params for key %s "
                "failed", hsm_str, key_id->locator?key_id->locator:"(null)");
            return ODS_STATUS_ERR;
        }
    }
    /* lookup key */
    if (!key_id->hsmkey) {
        key_id->hsmkey = hsm_find_key_by_id(ctx, key_id->locator);
    }
    if (!key_id->hsmkey) {
        error = hsm_get_error(ctx);
        if (error) {
            ods_log_error("[%s] %s", hsm_str, error);
            free((void*)error);
        } else if (!retries) {
            lhsm_clear_key_cache(key_id);
            retries++;
            goto llibhsm_key_start;
        }
        /* could not find key */
        ods_log_error("[%s] unable to get key: key %s not found", hsm_str,
            key_id->locator?key_id->locator:"(null)");
        return ODS_STATUS_ERR;
    }
    /* get dnskey */
    if (!key_id->dnskey) {
        key_id->dnskey = hsm_get_dnskey(ctx, key_id->hsmkey, key_id->params);
    }
    if (!key_id->dnskey) {
        error = hsm_get_error(ctx);
        if (error) {
            ods_log_error("[%s] %s", hsm_str, error);
            free((void*)error);
        } else if (!retries) {
            lhsm_clear_key_cache(key_id);
            retries++;
            goto llibhsm_key_start;
        }
        ods_log_error("[%s] unable to get key: hsm failed to create dnskey",
            hsm_str);
        return ODS_STATUS_ERR;
    }
    key_id->params->keytag = ldns_calc_keytag(key_id->dnskey);
    return ODS_STATUS_OK;
}


/**
 * Get RRSIG from one of the HSMs, given a RRset and a key.
 *
 */
ldns_rr*
lhsm_sign(hsm_ctx_t* ctx, ldns_rr_list* rrset, key_type* key_id,
    ldns_rdf* owner, time_t inception, time_t expiration)
{
    char* error = NULL;
    ldns_rr* result = NULL;
    hsm_sign_params_t* params = NULL;

    if (!owner || !key_id || !rrset || !inception || !expiration) {
        ods_log_error("[%s] unable to sign: missing required elements",
            hsm_str);
        return NULL;
    }
    ods_log_assert(key_id->dnskey);
    ods_log_assert(key_id->hsmkey);
    ods_log_assert(key_id->params);
    /* adjust parameters */
    params = hsm_sign_params_new();
    params->owner = ldns_rdf_clone(key_id->params->owner);
    params->algorithm = key_id->algorithm;
    params->flags = key_id->flags;
    params->inception = inception;
    params->expiration = expiration;
    params->keytag = key_id->params->keytag;
    ods_log_deeebug("[%s] sign RRset[%i] with key %s tag %u", hsm_str,
        ldns_rr_get_type(ldns_rr_list_rr(rrset, 0)),
        key_id->locator?key_id->locator:"(null)", params->keytag);
    result = hsm_sign_rrset(ctx, rrset, key_id->hsmkey, params);
    hsm_sign_params_free(params);
    if (!result) {
        error = hsm_get_error(ctx);
        if (error) {
            ods_log_error("[%s] %s", hsm_str, error);
            free((void*)error);
        }
        ods_log_crit("[%s] error signing rrset with libhsm", hsm_str);
    }
    return result;
}
