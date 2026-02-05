/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gs/util/gosh/command.h>
#include <gs/util/clock.h>
#include <gs/hk/config.h>
#include <gs/hk/internal/internal_config.h>
#include <gs/hk/param/host/hkcfg.h>
#include <gs/param/host_table.h>
#include <gs/uthash/utlist.h>

static gs_error_t noop_initializer(gs_param_table_instance_t* tinst) {
    return GS_OK;
}

static void print_beacons_brief(gs_command_context_t *ctx) {

    gs_hk_config_handle_t cfg_hdl;
    gs_error_t res = gs_hk_config_open(&cfg_hdl);
    if (res != GS_OK) {
        return;
    }

    gs_hk_beacon_cfg_t* beacons = gs_hk_config_get_beacons(cfg_hdl);
    gs_hk_beacon_cfg_t* elem = NULL;
    char all[128];
    memset(all, 0, 128);
    uint32_t cnt = 0;
    LL_FOREACH(beacons, elem) {
        char buf[5];
        sprintf(buf, "%i,", elem->type);
        strncat(all, buf, 5);
        cnt++;
    }

    char root_path[GS_HK_HKCFG_DATA_PATH_LENGTH+1];
    GS_HK_HKCFG_DATA_PATH_GET(root_path, sizeof(root_path));
    char path[50];
    snprintf(path, sizeof(path), "%s/%s", root_path, GS_HK_BCN_TYPES_PATH_PREFIX);

    char buf[8];
    sprintf(buf, "%u", (unsigned int)gs_hk_config_count_beacon_files_in_data_path(path));

    gs_command_set_output(ctx, "Beacon configuration", "Path", path);
    gs_command_set_output(ctx, "Beacon configuration", "Files in path", buf);
    gs_command_set_output(ctx, "Beacon configuration", "Beacons loaded", all);

    gs_hk_config_close(cfg_hdl);
}

static gs_error_t print_table(gs_command_context_t *ctx,
                              bool verbose,
                              gs_hk_table_cfg_t* table) {

    char buf[32];
    gs_error_t res = GS_OK;
    sprintf(buf, "%i", table->collected_table_node);
    gs_command_set_output(ctx, "General", "Node", buf);
    sprintf(buf, "%i", table->collected_table_idx);
    gs_command_set_output(ctx, "General", "Id", buf);
    sprintf(buf, "%i", table->collected_table_checksum);
    gs_command_set_output(ctx, "General", "Checksum", buf);
    sprintf(buf, "%i", table->collect_interval);
    gs_command_set_output(ctx, "General", "Samplerate", buf);
    sprintf(buf, "%i", table->rparam_download_timeout);
    gs_command_set_output(ctx, "General", "Download timeout", buf);
    sprintf(buf, "%i", table->ttl);
    gs_command_set_output(ctx, "General", "TTL", buf);
    sprintf(buf, "%i", table->used_in_beacons_count);
    gs_command_set_output(ctx, "General", "Used in # beacons", buf);
    sprintf(buf, "%"PRIu32, table->store_max_samples);
    gs_command_set_output(ctx, "Store", "Max samples", buf);
    if (table->store != NULL) {
        sprintf(buf, "%u", (unsigned int)table->store->sample_size);
        gs_command_set_output(ctx, "Store", "Sample size", buf);
        if (table->store->head != NULL) {
            char buf2[32];
            gs_clock_to_iso8601_string2(table->store->head->timestamp,
                                        buf2, sizeof(buf2));
            gs_command_set_output(ctx, "Store", "Last sample time", buf2);
            gs_clock_to_iso8601_string2(table->last_persist, buf2, sizeof(buf2));
            gs_command_set_output(ctx, "Store", "Last persist time", buf2);

            store_lock(table->store);
            gs_hk_sample_t* e = NULL;
            uint32_t samples_in_use = 0;
            DL_FOREACH(table->store->head, e) {
                if (e->data != NULL) {
                    ++samples_in_use;
                }
            }
            store_unlock(table->store);
            sprintf(buf, "%"PRIu32, samples_in_use);
            gs_command_set_output(ctx, "Store", "Samples in use", buf);

            if (verbose) {
                fprintf(ctx->out, "Latest sample:\r\n");
                GS_PARAM_TINST_VAR(tinst);
                size_t row_count = 0;
                const gs_param_table_row_t* rows =
                        gs_param_table_get_rows(table->collected_table,
                                                &row_count);
                const gs_param_table_create_t create = {
                    .rows = rows,
                    .row_count = row_count,
                    .memory = table->store->head->data,
                    .memory_size = table->store->sample_size,
                    .initializer = noop_initializer,
                    .flags = GS_PARAM_TABLE_F_NO_LOCK
                };
                res = gs_param_table_init(tinst, &create);
                if (res == GS_OK) {
                    res = gs_param_list_to_stream(tinst, true, 0, ctx->out);
                }
            }
        }
    }
    return res;
}

static gs_error_t print_beacon_type_info(gs_command_context_t *ctx, bool verbose) {
    uint8_t type = atoi(ctx->argv[1]);
    gs_hk_config_handle_t cfg_hdl;
    gs_hk_beacon_cfg_t *beacon;
    gs_error_t res = gs_hk_config_open(&cfg_hdl);
    if (res != GS_OK) {
        return res;
    }
    res = gs_hk_config_get_beacon(cfg_hdl, type, &beacon);
    if (res != GS_OK) {
        gs_hk_config_close(cfg_hdl);
        return res;
    }

    char buf[32];
    sprintf(buf, "%i", beacon->type);
    gs_command_set_output(ctx, "General", "Type", buf);
    sprintf(buf, "%i", beacon->version);
    gs_command_set_output(ctx, "General", "Version", buf);
    gs_command_set_output(ctx, "Autobeacon", "Policy",
                          gs_hk_beacon_policy_to_string(beacon->policy));


    gs_hk_beacon_data_element_t* elem = NULL;
    LL_FOREACH(beacon->data_elements, elem) {
        gs_hk_beacon_parameter_t* p = NULL;
        char buf2[64];
        if (verbose) {
            LL_FOREACH(elem->parameters, p) {
                sprintf(buf, "Table %i, %i",
                        elem->table->collected_table_node,
                        elem->table->collected_table_idx);
                sprintf(buf2, "array_index: %i-%i, ptr: %p",
                        p->array_idx_begin, p->array_idx_end, p->p_row);
                gs_command_set_output(ctx, buf, p->name, buf2);
            }
        } else {
            uint32_t cnt;
            LL_COUNT(elem->parameters, p, cnt);
            sprintf(buf, "Node=%i, Id=%i, Parameters=%"PRIu32,
                    elem->table->collected_table_node,
                    elem->table->collected_table_idx,
                    cnt);
            gs_command_set_output(ctx, "Tables", "Table", buf);
        }
    }

    gs_hk_config_close(cfg_hdl);
    return GS_OK;
}

gs_error_t cmd_handler_hk_srv_beacon_show(gs_command_context_t *ctx)
{
    if (!gs_hk_config_is_initialized()) {
        return GS_ERROR_STATE;
    }

    if (ctx->argc == 1) {
        print_beacons_brief(ctx);
    } else {
        return print_beacon_type_info(ctx, (ctx->argc == 3));
    }
    return GS_OK;
}

gs_error_t cmd_handler_hk_srv_table_show(gs_command_context_t *ctx)
{
    if (!gs_hk_config_is_initialized()) {
        return GS_ERROR_STATE;
    }

    gs_hk_config_handle_t cfg_hdl;
    gs_error_t res = gs_hk_config_open(&cfg_hdl);
    if (res != GS_OK) {
        return res;
    }

    char buf[64];
    if (ctx->argc == 1) {
        gs_hk_table_cfg_t* tables = gs_hk_config_get_tables(cfg_hdl);
        if (tables != NULL) {
            gs_hk_table_cfg_t* table = NULL;
            LL_FOREACH(tables, table) {
                sprintf(buf, "Node: %i, Id: %i, Samplerate: %i, "
                        "Latest sample: %"PRIu32, table->collected_table_node,
                        table->collected_table_idx, table->collect_interval,
                        table->last_collection_ts);
                gs_command_set_output(ctx, "Tables", NULL, buf);
            }
        } else {
            res = GS_ERROR_NOT_FOUND;
        }
    } else if (ctx->argc >= 3) {
        gs_hk_table_cfg_t* table;
        res = gs_hk_config_get_table(cfg_hdl, atoi(ctx->argv[1]),
                                     atoi(ctx->argv[2]), &table);
        if (res == GS_OK) {
            res = print_table(ctx, (ctx->argc == 4), table);
        }
    } else {
        res = GS_ERROR_ARG;
    }

    gs_hk_config_close(cfg_hdl);
    return res;
}

gs_error_t cmd_handler_hk_srv_reload(gs_command_context_t* ctx) {
    if (!gs_hk_config_is_initialized()) {
        return GS_ERROR_STATE;
    }

    return gs_hk_config_reload();
}

gs_error_t cmd_handler_hk_srv_options_show(gs_command_context_t* ctx) {
    if (!gs_hk_config_is_initialized()) {
        return GS_ERROR_STATE;
    }

    return gs_param_list_to_stream(gs_param_table_by_name(GS_HK_HKCFG_TABLE_NAME), true, 0, ctx->out);
}

gs_error_t cmd_handler_hk_srv_beacon_policy(gs_command_context_t* ctx) {
    if (!gs_hk_config_is_initialized()) {
        return GS_ERROR_STATE;
    }

    uint8_t type = atoi(ctx->argv[1]);
    gs_hk_beacon_policy_t policy = gs_hk_beacon_policy_from_string(ctx->argv[2]);

    gs_hk_config_handle_t cfg_hdl;
    gs_hk_beacon_cfg_t *beacon;
    gs_error_t res = gs_hk_config_open(&cfg_hdl);
    if (res != GS_OK) {
        return res;
    }

    res = gs_hk_config_get_beacon(cfg_hdl, type, &beacon);
    if (res != GS_OK) {
        gs_hk_config_close(cfg_hdl);
        return res;
    }

    fprintf(ctx->out, "Setting autobeacon policy, type=%i, policy=%s\r\n",
            type, gs_hk_beacon_policy_to_string(policy));
    beacon->policy = policy;
    gs_hk_config_close(cfg_hdl);
    return GS_OK;
}

gs_error_t cmd_handler_hk_srv_beacon_samplerate(gs_command_context_t* ctx) {
    if (!gs_hk_config_is_initialized()) {
        return GS_ERROR_STATE;
    }

    uint8_t type = atoi(ctx->argv[1]);
    const char* priority = ctx->argv[2];

    gs_hk_config_handle_t cfg_hdl;
    gs_error_t res = gs_hk_config_open(&cfg_hdl);
    if (res != GS_OK) {
        return res;
    }

    res = gs_hk_config_update_beacon_priority(cfg_hdl, type, priority);
    gs_hk_config_close(cfg_hdl);
    return res;
}
