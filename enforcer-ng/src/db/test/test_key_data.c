/*
 * Copyright (c) 2014 Jerry Lundström <lundstrom.jerry@gmail.com>
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

#include "CUnit/Basic.h"

#include "../db_configuration.h"
#include "../db_connection.h"
#include "../key_data.h"

#include <string.h>

static db_configuration_list_t* configuration_list = NULL;
static db_configuration_t* configuration = NULL;
static db_connection_t* connection = NULL;

static key_data_t* object = NULL;
static key_data_list_t* object_list = NULL;
static db_value_t id = DB_VALUE_EMPTY;
static db_clause_list_t* clause_list = NULL;

static int db_sqlite = 0;
static int db_couchdb = 0;
static int db_mysql = 0;

#if defined(ENFORCER_DATABASE_SQLITE3)
int test_key_data_init_suite_sqlite(void) {
    if (configuration_list) {
        return 1;
    }
    if (configuration) {
        return 1;
    }
    if (connection) {
        return 1;
    }

    /*
     * Setup the configuration for the connection
     */
    if (!(configuration_list = db_configuration_list_new())) {
        return 1;
    }
    if (!(configuration = db_configuration_new())
        || db_configuration_set_name(configuration, "backend")
        || db_configuration_set_value(configuration, "sqlite")
        || db_configuration_list_add(configuration_list, configuration))
    {
        db_configuration_free(configuration);
        configuration = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = db_configuration_new())
        || db_configuration_set_name(configuration, "file")
        || db_configuration_set_value(configuration, "test.db")
        || db_configuration_list_add(configuration_list, configuration))
    {
        db_configuration_free(configuration);
        configuration = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;

    /*
     * Connect to the database
     */
    if (!(connection = db_connection_new())
        || db_connection_set_configuration_list(connection, configuration_list))
    {
        db_connection_free(connection);
        connection = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration_list = NULL;

    if (db_connection_setup(connection)
        || db_connection_connect(connection))
    {
        db_connection_free(connection);
        connection = NULL;
        return 1;
    }

    db_sqlite = 1;
    db_couchdb = 0;
    db_mysql = 0;

    return 0;
}
#endif

#if defined(ENFORCER_DATABASE_COUCHDB)
int test_key_data_init_suite_couchdb(void) {
    if (configuration_list) {
        return 1;
    }
    if (configuration) {
        return 1;
    }
    if (connection) {
        return 1;
    }

    /*
     * Setup the configuration for the connection
     */
    if (!(configuration_list = db_configuration_list_new())) {
        return 1;
    }
    if (!(configuration = db_configuration_new())
        || db_configuration_set_name(configuration, "backend")
        || db_configuration_set_value(configuration, "couchdb")
        || db_configuration_list_add(configuration_list, configuration))
    {
        db_configuration_free(configuration);
        configuration = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = db_configuration_new())
        || db_configuration_set_name(configuration, "url")
        || db_configuration_set_value(configuration, "http://127.0.0.1:5984/opendnssec")
        || db_configuration_list_add(configuration_list, configuration))
    {
        db_configuration_free(configuration);
        configuration = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;

    /*
     * Connect to the database
     */
    if (!(connection = db_connection_new())
        || db_connection_set_configuration_list(connection, configuration_list))
    {
        db_connection_free(connection);
        connection = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration_list = NULL;

    if (db_connection_setup(connection)
        || db_connection_connect(connection))
    {
        db_connection_free(connection);
        connection = NULL;
        return 1;
    }

    db_sqlite = 0;
    db_couchdb = 1;
    db_mysql = 0;

    return 0;
}
#endif

#if defined(ENFORCER_DATABASE_MYSQL)
int test_key_data_init_suite_mysql(void) {
    if (configuration_list) {
        return 1;
    }
    if (configuration) {
        return 1;
    }
    if (connection) {
        return 1;
    }

    /*
     * Setup the configuration for the connection
     */
    if (!(configuration_list = db_configuration_list_new())) {
        return 1;
    }
    if (!(configuration = db_configuration_new())
        || db_configuration_set_name(configuration, "backend")
        || db_configuration_set_value(configuration, "mysql")
        || db_configuration_list_add(configuration_list, configuration))
    {
        db_configuration_free(configuration);
        configuration = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = db_configuration_new())
        || db_configuration_set_name(configuration, "host")
        || db_configuration_set_value(configuration, ENFORCER_DB_HOST)
        || db_configuration_list_add(configuration_list, configuration))
    {
        db_configuration_free(configuration);
        configuration = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = db_configuration_new())
        || db_configuration_set_name(configuration, "port")
        || db_configuration_set_value(configuration, ENFORCER_DB_PORT_TEXT)
        || db_configuration_list_add(configuration_list, configuration))
    {
        db_configuration_free(configuration);
        configuration = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = db_configuration_new())
        || db_configuration_set_name(configuration, "user")
        || db_configuration_set_value(configuration, ENFORCER_DB_USERNAME)
        || db_configuration_list_add(configuration_list, configuration))
    {
        db_configuration_free(configuration);
        configuration = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = db_configuration_new())
        || db_configuration_set_name(configuration, "pass")
        || db_configuration_set_value(configuration, ENFORCER_DB_PASSWORD)
        || db_configuration_list_add(configuration_list, configuration))
    {
        db_configuration_free(configuration);
        configuration = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = db_configuration_new())
        || db_configuration_set_name(configuration, "db")
        || db_configuration_set_value(configuration, ENFORCER_DB_DATABASE)
        || db_configuration_list_add(configuration_list, configuration))
    {
        db_configuration_free(configuration);
        configuration = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;

    /*
     * Connect to the database
     */
    if (!(connection = db_connection_new())
        || db_connection_set_configuration_list(connection, configuration_list))
    {
        db_connection_free(connection);
        connection = NULL;
        db_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration_list = NULL;

    if (db_connection_setup(connection)
        || db_connection_connect(connection))
    {
        db_connection_free(connection);
        connection = NULL;
        return 1;
    }

    db_sqlite = 0;
    db_couchdb = 0;
    db_mysql = 1;

    return 0;
}
#endif

static int test_key_data_clean_suite(void) {
    db_connection_free(connection);
    connection = NULL;
    db_configuration_free(configuration);
    configuration = NULL;
    db_configuration_list_free(configuration_list);
    configuration_list = NULL;
    db_value_reset(&id);
    db_clause_list_free(clause_list);
    clause_list = NULL;
    return 0;
}

static void test_key_data_new(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((object = key_data_new(connection)));
    CU_ASSERT_PTR_NOT_NULL_FATAL((object_list = key_data_list_new(connection)));
}

static void test_key_data_set(void) {
    db_value_t zone_id = DB_VALUE_EMPTY;
    db_value_t hsm_key_id = DB_VALUE_EMPTY;
    if (db_sqlite) {
        CU_ASSERT(!db_value_from_int32(&zone_id, 1));
    }
    if (db_couchdb) {
        CU_ASSERT(!db_value_from_int32(&zone_id, 1));
    }
    if (db_mysql) {
        CU_ASSERT(!db_value_from_uint64(&zone_id, 1));
    }
    if (db_sqlite) {
        CU_ASSERT(!db_value_from_int32(&hsm_key_id, 1));
    }
    if (db_couchdb) {
        CU_ASSERT(!db_value_from_int32(&hsm_key_id, 1));
    }
    if (db_mysql) {
        CU_ASSERT(!db_value_from_uint64(&hsm_key_id, 1));
    }
    CU_ASSERT(!key_data_set_zone_id(object, &zone_id));
    CU_ASSERT(!key_data_set_hsm_key_id(object, &hsm_key_id));
    CU_ASSERT(!key_data_set_algorithm(object, 1));
    CU_ASSERT(!key_data_set_inception(object, 1));
    CU_ASSERT(!key_data_set_role(object, KEY_DATA_ROLE_KSK));
    CU_ASSERT(!key_data_set_role_text(object, "KSK"));
    CU_ASSERT(!key_data_set_role(object, KEY_DATA_ROLE_ZSK));
    CU_ASSERT(!key_data_set_role_text(object, "ZSK"));
    CU_ASSERT(!key_data_set_role(object, KEY_DATA_ROLE_CSK));
    CU_ASSERT(!key_data_set_role_text(object, "CSK"));
    CU_ASSERT(!key_data_set_introducing(object, 1));
    CU_ASSERT(!key_data_set_should_revoke(object, 1));
    CU_ASSERT(!key_data_set_standby(object, 1));
    CU_ASSERT(!key_data_set_active_zsk(object, 1));
    CU_ASSERT(!key_data_set_publish(object, 1));
    CU_ASSERT(!key_data_set_active_ksk(object, 1));
    CU_ASSERT(!key_data_set_ds_at_parent(object, KEY_DATA_DS_AT_PARENT_UNSUBMITTED));
    CU_ASSERT(!key_data_set_ds_at_parent_text(object, "unsubmitted"));
    CU_ASSERT(!key_data_set_ds_at_parent(object, KEY_DATA_DS_AT_PARENT_SUBMIT));
    CU_ASSERT(!key_data_set_ds_at_parent_text(object, "submit"));
    CU_ASSERT(!key_data_set_ds_at_parent(object, KEY_DATA_DS_AT_PARENT_SUBMITTED));
    CU_ASSERT(!key_data_set_ds_at_parent_text(object, "submitted"));
    CU_ASSERT(!key_data_set_ds_at_parent(object, KEY_DATA_DS_AT_PARENT_SEEN));
    CU_ASSERT(!key_data_set_ds_at_parent_text(object, "seen"));
    CU_ASSERT(!key_data_set_ds_at_parent(object, KEY_DATA_DS_AT_PARENT_RETRACT));
    CU_ASSERT(!key_data_set_ds_at_parent_text(object, "retract"));
    CU_ASSERT(!key_data_set_ds_at_parent(object, KEY_DATA_DS_AT_PARENT_RETRACTED));
    CU_ASSERT(!key_data_set_ds_at_parent_text(object, "retracted"));
    CU_ASSERT(!key_data_set_keytag(object, 1));
    CU_ASSERT(!key_data_set_minimize(object, 1));
    db_value_reset(&zone_id);
    db_value_reset(&hsm_key_id);
}

static void test_key_data_get(void) {
    int ret;
    db_value_t zone_id = DB_VALUE_EMPTY;
    db_value_t hsm_key_id = DB_VALUE_EMPTY;
    if (db_sqlite) {
        CU_ASSERT(!db_value_from_int32(&zone_id, 1));
    }
    if (db_couchdb) {
        CU_ASSERT(!db_value_from_int32(&zone_id, 1));
    }
    if (db_mysql) {
        CU_ASSERT(!db_value_from_uint64(&zone_id, 1));
    }
    if (db_sqlite) {
        CU_ASSERT(!db_value_from_int32(&hsm_key_id, 1));
    }
    if (db_couchdb) {
        CU_ASSERT(!db_value_from_int32(&hsm_key_id, 1));
    }
    if (db_mysql) {
        CU_ASSERT(!db_value_from_uint64(&hsm_key_id, 1));
    }
    CU_ASSERT(!db_value_cmp(key_data_zone_id(object), &zone_id, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT(!db_value_cmp(key_data_hsm_key_id(object), &hsm_key_id, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT(key_data_algorithm(object) == 1);
    CU_ASSERT(key_data_inception(object) == 1);
    CU_ASSERT(key_data_role(object) == KEY_DATA_ROLE_CSK);
    CU_ASSERT_PTR_NOT_NULL_FATAL(key_data_role_text(object));
    CU_ASSERT(!strcmp(key_data_role_text(object), "CSK"));
    CU_ASSERT(key_data_introducing(object) == 1);
    CU_ASSERT(key_data_should_revoke(object) == 1);
    CU_ASSERT(key_data_standby(object) == 1);
    CU_ASSERT(key_data_active_zsk(object) == 1);
    CU_ASSERT(key_data_publish(object) == 1);
    CU_ASSERT(key_data_active_ksk(object) == 1);
    CU_ASSERT(key_data_ds_at_parent(object) == KEY_DATA_DS_AT_PARENT_RETRACTED);
    CU_ASSERT_PTR_NOT_NULL_FATAL(key_data_ds_at_parent_text(object));
    CU_ASSERT(!strcmp(key_data_ds_at_parent_text(object), "retracted"));
    CU_ASSERT(key_data_keytag(object) == 1);
    CU_ASSERT(key_data_minimize(object) == 1);
    db_value_reset(&zone_id);
    db_value_reset(&hsm_key_id);
}

static void test_key_data_create(void) {
    CU_ASSERT_FATAL(!key_data_create(object));
}

static void test_key_data_clauses(void) {
    key_data_list_t* new_list;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_zone_id_clause(clause_list, key_data_zone_id(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_hsm_key_id_clause(clause_list, key_data_hsm_key_id(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_algorithm_clause(clause_list, key_data_algorithm(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_inception_clause(clause_list, key_data_inception(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_role_clause(clause_list, key_data_role(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_introducing_clause(clause_list, key_data_introducing(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_should_revoke_clause(clause_list, key_data_should_revoke(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_standby_clause(clause_list, key_data_standby(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_active_zsk_clause(clause_list, key_data_active_zsk(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_publish_clause(clause_list, key_data_publish(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_active_ksk_clause(clause_list, key_data_active_ksk(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_ds_at_parent_clause(clause_list, key_data_ds_at_parent(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_keytag_clause(clause_list, key_data_keytag(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_minimize_clause(clause_list, key_data_minimize(object)));
    CU_ASSERT(!key_data_list_get_by_clauses(object_list, clause_list));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(object_list));
    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get_by_clauses(connection, clause_list)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
    db_clause_list_free(clause_list);
    clause_list = NULL;
}

static void test_key_data_count(void) {
    size_t count;

    CU_ASSERT(!key_data_count(object, NULL, &count));
    CU_ASSERT(count == 1);

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_zone_id_clause(clause_list, key_data_zone_id(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_hsm_key_id_clause(clause_list, key_data_hsm_key_id(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_algorithm_clause(clause_list, key_data_algorithm(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_inception_clause(clause_list, key_data_inception(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_role_clause(clause_list, key_data_role(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_introducing_clause(clause_list, key_data_introducing(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_should_revoke_clause(clause_list, key_data_should_revoke(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_standby_clause(clause_list, key_data_standby(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_active_zsk_clause(clause_list, key_data_active_zsk(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_publish_clause(clause_list, key_data_publish(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_active_ksk_clause(clause_list, key_data_active_ksk(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_ds_at_parent_clause(clause_list, key_data_ds_at_parent(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_keytag_clause(clause_list, key_data_keytag(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = db_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL(key_data_minimize_clause(clause_list, key_data_minimize(object)));
    CU_ASSERT(!key_data_count(object, clause_list, &count));
    CU_ASSERT(count == 1);
    db_clause_list_free(clause_list);
    clause_list = NULL;
}

static void test_key_data_list(void) {
    const key_data_t* item;
    key_data_t* item2;
    key_data_list_t* new_list;

    CU_ASSERT_FATAL(!key_data_list_get(object_list));
    CU_ASSERT_PTR_NOT_NULL_FATAL((item = key_data_list_next(object_list)));
    CU_ASSERT_FATAL(!db_value_copy(&id, key_data_id(item)));
    CU_ASSERT_PTR_NOT_NULL_FATAL((item = key_data_list_begin(object_list)));

    CU_ASSERT_FATAL(!key_data_list_get(object_list));
    CU_ASSERT_PTR_NOT_NULL_FATAL((item2 = key_data_list_get_next(object_list)));
    key_data_free(item2);
    CU_PASS("key_data_free");
    CU_ASSERT_PTR_NOT_NULL_FATAL((item2 = key_data_list_get_begin(object_list)));
    key_data_free(item2);
    CU_PASS("key_data_free");

    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new_get(connection)));
    CU_ASSERT_PTR_NOT_NULL(key_data_list_next(new_list));
    key_data_list_free(new_list);
}

static void test_key_data_list_store(void) {
    key_data_t* item;
    key_data_list_t* new_list;

    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new(connection)));
    CU_ASSERT_FATAL(!key_data_list_object_store(new_list));
    CU_ASSERT_FATAL(!key_data_list_get(new_list));

    CU_ASSERT_PTR_NOT_NULL_FATAL(key_data_list_next(new_list));
    CU_ASSERT_PTR_NOT_NULL_FATAL(key_data_list_begin(new_list));

    CU_ASSERT_PTR_NOT_NULL_FATAL((item = key_data_list_get_begin(new_list)));
    key_data_free(item);
    CU_PASS("key_data_free");

    key_data_list_free(new_list);
}

static void test_key_data_list_associated(void) {
    key_data_t* item;
    key_data_list_t* new_list;

    CU_ASSERT_PTR_NOT_NULL((new_list = key_data_list_new(connection)));
    CU_ASSERT_FATAL(!key_data_list_associated_fetch(new_list));
    CU_ASSERT_FATAL(!key_data_list_get(new_list));

    CU_ASSERT_PTR_NOT_NULL_FATAL(key_data_list_next(new_list));
    CU_ASSERT_PTR_NOT_NULL_FATAL(key_data_list_begin(new_list));

    CU_ASSERT_PTR_NOT_NULL_FATAL((item = key_data_list_get_begin(new_list)));
    key_data_free(item);
    CU_PASS("key_data_free");

    key_data_list_free(new_list);
}

static void test_key_data_read(void) {
    key_data_t* item;

    CU_ASSERT_FATAL(!key_data_get_by_id(object, &id));
    CU_ASSERT_PTR_NOT_NULL((item = key_data_new_get_by_id(connection, &id)));
    key_data_free(item);
}

static void test_key_data_verify(void) {
    int ret;
    db_value_t zone_id = DB_VALUE_EMPTY;
    db_value_t hsm_key_id = DB_VALUE_EMPTY;
    if (db_sqlite) {
        CU_ASSERT(!db_value_from_int32(&zone_id, 1));
    }
    if (db_couchdb) {
        CU_ASSERT(!db_value_from_int32(&zone_id, 1));
    }
    if (db_mysql) {
        CU_ASSERT(!db_value_from_uint64(&zone_id, 1));
    }
    if (db_sqlite) {
        CU_ASSERT(!db_value_from_int32(&hsm_key_id, 1));
    }
    if (db_couchdb) {
        CU_ASSERT(!db_value_from_int32(&hsm_key_id, 1));
    }
    if (db_mysql) {
        CU_ASSERT(!db_value_from_uint64(&hsm_key_id, 1));
    }
    CU_ASSERT(!db_value_cmp(key_data_zone_id(object), &zone_id, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT(!db_value_cmp(key_data_hsm_key_id(object), &hsm_key_id, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT(key_data_algorithm(object) == 1);
    CU_ASSERT(key_data_inception(object) == 1);
    CU_ASSERT(key_data_role(object) == KEY_DATA_ROLE_CSK);
    CU_ASSERT_PTR_NOT_NULL_FATAL(key_data_role_text(object));
    CU_ASSERT(!strcmp(key_data_role_text(object), "CSK"));
    CU_ASSERT(key_data_introducing(object) == 1);
    CU_ASSERT(key_data_should_revoke(object) == 1);
    CU_ASSERT(key_data_standby(object) == 1);
    CU_ASSERT(key_data_active_zsk(object) == 1);
    CU_ASSERT(key_data_publish(object) == 1);
    CU_ASSERT(key_data_active_ksk(object) == 1);
    CU_ASSERT(key_data_ds_at_parent(object) == KEY_DATA_DS_AT_PARENT_RETRACTED);
    CU_ASSERT_PTR_NOT_NULL_FATAL(key_data_ds_at_parent_text(object));
    CU_ASSERT(!strcmp(key_data_ds_at_parent_text(object), "retracted"));
    CU_ASSERT(key_data_keytag(object) == 1);
    CU_ASSERT(key_data_minimize(object) == 1);
    db_value_reset(&zone_id);
    db_value_reset(&hsm_key_id);
}

static void test_key_data_change(void) {
    db_value_t zone_id = DB_VALUE_EMPTY;
    db_value_t hsm_key_id = DB_VALUE_EMPTY;
    if (db_sqlite) {
        CU_ASSERT(!db_value_from_int32(&zone_id, 1));
    }
    if (db_couchdb) {
        CU_ASSERT(!db_value_from_int32(&zone_id, 1));
    }
    if (db_mysql) {
        CU_ASSERT(!db_value_from_uint64(&zone_id, 1));
    }
    if (db_sqlite) {
        CU_ASSERT(!db_value_from_int32(&hsm_key_id, 1));
    }
    if (db_couchdb) {
        CU_ASSERT(!db_value_from_int32(&hsm_key_id, 1));
    }
    if (db_mysql) {
        CU_ASSERT(!db_value_from_uint64(&hsm_key_id, 1));
    }
    CU_ASSERT(!key_data_set_zone_id(object, &zone_id));
    CU_ASSERT(!key_data_set_hsm_key_id(object, &hsm_key_id));
    CU_ASSERT(!key_data_set_algorithm(object, 2));
    CU_ASSERT(!key_data_set_inception(object, 2));
    CU_ASSERT(!key_data_set_role(object, KEY_DATA_ROLE_KSK));
    CU_ASSERT(!key_data_set_role_text(object, "KSK"));
    CU_ASSERT(!key_data_set_introducing(object, 2));
    CU_ASSERT(!key_data_set_should_revoke(object, 2));
    CU_ASSERT(!key_data_set_standby(object, 2));
    CU_ASSERT(!key_data_set_active_zsk(object, 2));
    CU_ASSERT(!key_data_set_publish(object, 2));
    CU_ASSERT(!key_data_set_active_ksk(object, 2));
    CU_ASSERT(!key_data_set_ds_at_parent(object, KEY_DATA_DS_AT_PARENT_UNSUBMITTED));
    CU_ASSERT(!key_data_set_ds_at_parent_text(object, "unsubmitted"));
    CU_ASSERT(!key_data_set_keytag(object, 2));
    CU_ASSERT(!key_data_set_minimize(object, 2));
    db_value_reset(&zone_id);
    db_value_reset(&hsm_key_id);
}

static void test_key_data_update(void) {
    CU_ASSERT_FATAL(!key_data_update(object));
}

static void test_key_data_read2(void) {
    CU_ASSERT_FATAL(!key_data_get_by_id(object, &id));
}

static void test_key_data_verify2(void) {
    int ret;
    db_value_t zone_id = DB_VALUE_EMPTY;
    db_value_t hsm_key_id = DB_VALUE_EMPTY;
    if (db_sqlite) {
        CU_ASSERT(!db_value_from_int32(&zone_id, 1));
    }
    if (db_couchdb) {
        CU_ASSERT(!db_value_from_int32(&zone_id, 1));
    }
    if (db_mysql) {
        CU_ASSERT(!db_value_from_uint64(&zone_id, 1));
    }
    if (db_sqlite) {
        CU_ASSERT(!db_value_from_int32(&hsm_key_id, 1));
    }
    if (db_couchdb) {
        CU_ASSERT(!db_value_from_int32(&hsm_key_id, 1));
    }
    if (db_mysql) {
        CU_ASSERT(!db_value_from_uint64(&hsm_key_id, 1));
    }
    CU_ASSERT(!db_value_cmp(key_data_zone_id(object), &zone_id, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT(!db_value_cmp(key_data_hsm_key_id(object), &hsm_key_id, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT(key_data_algorithm(object) == 2);
    CU_ASSERT(key_data_inception(object) == 2);
    CU_ASSERT(key_data_role(object) == KEY_DATA_ROLE_KSK);
    CU_ASSERT_PTR_NOT_NULL_FATAL(key_data_role_text(object));
    CU_ASSERT(!strcmp(key_data_role_text(object), "KSK"));
    CU_ASSERT(key_data_introducing(object) == 2);
    CU_ASSERT(key_data_should_revoke(object) == 2);
    CU_ASSERT(key_data_standby(object) == 2);
    CU_ASSERT(key_data_active_zsk(object) == 2);
    CU_ASSERT(key_data_publish(object) == 2);
    CU_ASSERT(key_data_active_ksk(object) == 2);
    CU_ASSERT(key_data_ds_at_parent(object) == KEY_DATA_DS_AT_PARENT_UNSUBMITTED);
    CU_ASSERT_PTR_NOT_NULL_FATAL(key_data_ds_at_parent_text(object));
    CU_ASSERT(!strcmp(key_data_ds_at_parent_text(object), "unsubmitted"));
    CU_ASSERT(key_data_keytag(object) == 2);
    CU_ASSERT(key_data_minimize(object) == 2);
    db_value_reset(&zone_id);
    db_value_reset(&hsm_key_id);
}

static void test_key_data_cmp(void) {
    key_data_t* local_object;

    CU_ASSERT_PTR_NOT_NULL_FATAL((local_object = key_data_new(connection)));
    CU_ASSERT(key_data_cmp(object, local_object));
}

static void test_key_data_delete(void) {
    CU_ASSERT_FATAL(!key_data_delete(object));
}

static void test_key_data_list2(void) {
    CU_ASSERT_FATAL(!key_data_list_get(object_list));
    CU_ASSERT_PTR_NULL(key_data_list_next(object_list));
}

static void test_key_data_end(void) {
    if (object) {
        key_data_free(object);
        CU_PASS("key_data_free");
    }
    if (object_list) {
        key_data_list_free(object_list);
        CU_PASS("key_data_list_free");
    }
}

static int test_key_data_add_tests(CU_pSuite pSuite) {
    if (!CU_add_test(pSuite, "new object", test_key_data_new)
        || !CU_add_test(pSuite, "set fields", test_key_data_set)
        || !CU_add_test(pSuite, "get fields", test_key_data_get)
        || !CU_add_test(pSuite, "create object", test_key_data_create)
        || !CU_add_test(pSuite, "object clauses", test_key_data_clauses)
        || !CU_add_test(pSuite, "object count", test_key_data_count)
        || !CU_add_test(pSuite, "list objects", test_key_data_list)
        || !CU_add_test(pSuite, "list objects (store)", test_key_data_list_store)
        || !CU_add_test(pSuite, "list objects (associated)", test_key_data_list_associated)
        || !CU_add_test(pSuite, "read object by id", test_key_data_read)
        || !CU_add_test(pSuite, "verify fields", test_key_data_verify)
        || !CU_add_test(pSuite, "change object", test_key_data_change)
        || !CU_add_test(pSuite, "update object", test_key_data_update)
        || !CU_add_test(pSuite, "reread object by id", test_key_data_read2)
        || !CU_add_test(pSuite, "verify fields after update", test_key_data_verify2)
        || !CU_add_test(pSuite, "compare objects", test_key_data_cmp)
        || !CU_add_test(pSuite, "delete object", test_key_data_delete)
        || !CU_add_test(pSuite, "list objects to verify delete", test_key_data_list2)
        || !CU_add_test(pSuite, "end test", test_key_data_end))
    {
        return CU_get_error();
    }
    return 0;
}

int test_key_data_add_suite(void) {
    CU_pSuite pSuite = NULL;
    int ret;

#if defined(ENFORCER_DATABASE_SQLITE3)
    pSuite = CU_add_suite("Test of key data (SQLite)", test_key_data_init_suite_sqlite, test_key_data_clean_suite);
    if (!pSuite) {
        return CU_get_error();
    }
    ret = test_key_data_add_tests(pSuite);
    if (ret) {
        return ret;
    }
#endif
#if defined(ENFORCER_DATABASE_COUCHDB)
    pSuite = CU_add_suite("Test of key data (CouchDB)", test_key_data_init_suite_couchdb, test_key_data_clean_suite);
    if (!pSuite) {
        return CU_get_error();
    }
    ret = test_key_data_add_tests(pSuite);
    if (ret) {
        return ret;
    }
#endif
#if defined(ENFORCER_DATABASE_MYSQL)
    pSuite = CU_add_suite("Test of key data (MySQL)", test_key_data_init_suite_mysql, test_key_data_clean_suite);
    if (!pSuite) {
        return CU_get_error();
    }
    ret = test_key_data_add_tests(pSuite);
    if (ret) {
        return ret;
    }
#endif
    return 0;
}
