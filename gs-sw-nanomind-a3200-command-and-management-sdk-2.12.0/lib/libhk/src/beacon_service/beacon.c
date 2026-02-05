/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <beacon_service/beacon.h>

#include <gs/hk/internal/internal_config.h>
#include <gs/hk/internal/internal_collector_store.h>

#include <gs/util/log.h>
#include <gs/util/clock.h>
#include <gs/util/byteorder.h>

#include <stdlib.h>

#if defined(GS_LOG_CAT_HK)
#define LOG_CATEGORY GS_LOG_CAT_HK
#else
#define LOG_CATEGORY GS_LOG_CAT_DEFAULT
#endif

GS_LOG_GROUP(hkbcn, "hk_beacon", LOG_CATEGORY, LOG_DEFAULT_MASK);
#define LOG_DEFAULT hkbcn

#define GS_HK_BEACON_PACKET_MAX_SIZE 250
#define GS_HK_BEACON_PACKET_MAX_DATA_SIZE ((GS_HK_BEACON_PACKET_MAX_SIZE)-(CSP_BUFFER_PACKET_OVERHEAD))

static gs_error_t noop_initializer(gs_param_table_instance_t* tinst) {
    return GS_OK;
}

static gs_error_t prepare_table_sample_and_create_data_segment_header(
    gs_hk_table_cfg_t* table_cfg, uint32_t requested_time,
    csp_packet_t* bcn_packet, gs_param_table_instance_t* tmp_table) {

    // prepare data to put in beacon
    gs_error_t res = GS_OK;
    uint32_t beacontime = 0;
    if (requested_time > 0) {

        // find sample in store that matches timestamp
        gs_hk_sample_t* sample;
        res = store_search(table_cfg->store, requested_time, &sample);
        if (res == GS_OK) {
            beacontime = sample->timestamp;

            size_t row_count = 0;
            const gs_param_table_row_t* rows =
                    gs_param_table_get_rows(table_cfg->collected_table,
                                            &row_count);

            const gs_param_table_create_t create = {
                .rows = rows,
                .row_count = row_count,
                .memory = sample->data,
                .memory_size = table_cfg->store->sample_size,
                .initializer = noop_initializer,
                .flags = GS_PARAM_TABLE_F_NO_LOCK
            };

            res = gs_param_table_init(tmp_table, &create);
            if (res != GS_OK) {
                log_warning("gs_param_table_init() failed, res=%s",
                            gs_error_string(res));
            }
        } else {
            log_warning("telemetry sample not found, node=%i, id=%i, "
                        "requested_time=%"PRIu32,
                        table_cfg->collected_table_node,
                        table_cfg->collected_table_idx,
                        requested_time);
            res = GS_ERROR_NOT_FOUND;
        }
    } else {
        timestamp_t clock_now;
        gs_clock_get_time(&clock_now);
        beacontime = clock_now.tv_sec;
        log_trace("using current time for beacon, %"PRIu32, beacontime);
    }

    // create data segment header
    if (res == GS_OK) {

        // insert table crc
        uint16_t table_crc = util_hton16(table_cfg->collected_table_checksum);
        memcpy(&(bcn_packet->data[bcn_packet->length]),
               &table_crc, sizeof(table_crc));
        bcn_packet->length += sizeof(table_crc);

        // insert timestamp
        beacontime = util_hton32(beacontime);
        memcpy(&(bcn_packet->data[bcn_packet->length]),
               &beacontime, sizeof(beacontime));
        bcn_packet->length += sizeof(beacontime);

        // insert source - from which node does the following values origin
        uint16_t source = util_hton16(table_cfg->collected_table_node);
        memcpy(&(bcn_packet->data[bcn_packet->length]),
               &source, sizeof(source));
        bcn_packet->length += sizeof(source);
    }

    return res;
}

static csp_packet_t* alloc_csp_packet_and_create_header(
    gs_hk_beacon_cfg_t* bcn_type_cfg, uint16_t satid) {
    log_trace("creating beacon packet, type=%i", bcn_type_cfg->type);
    csp_packet_t* bcn_packet = csp_buffer_get(GS_HK_BEACON_PACKET_MAX_SIZE);
    if (bcn_packet == NULL)
    {
        return NULL;
    }

    /* create packet header */
    bcn_packet->length = 0;
    bcn_packet->data[bcn_packet->length++] = 1; // protocol version
    bcn_packet->data[bcn_packet->length++] = bcn_type_cfg->type;
    bcn_packet->data[bcn_packet->length++] = bcn_type_cfg->version;
    satid = util_hton16(satid);
    memcpy(&(bcn_packet->data[bcn_packet->length]),
           &satid, sizeof(satid));
    bcn_packet->length += sizeof(satid);

    return bcn_packet;
}

static gs_error_t resolve_parameter_name(gs_hk_beacon_parameter_t* parameter,
                                         gs_hk_table_cfg_t* table) {

    if (parameter->p_row == NULL) {
        size_t row_count = 0;
        const gs_param_table_row_t* rows =
                gs_param_table_get_rows(table->collected_table,
                                        &row_count);
        parameter->p_row = gs_param_row_by_name(parameter->name,
                                                rows, row_count);
    }

    if (parameter->p_row == NULL) {
        return GS_ERROR_NOT_FOUND;
    }

    if (parameter->array_idx_end > 0 &&
        parameter->array_idx_end >= parameter->p_row->array_size) {
        return GS_ERROR_RANGE;
    }

    return GS_OK;
}

static gs_error_t serialize_parameter_value_into_beacon(
    csp_packet_t* bcn_packet,
    gs_hk_table_cfg_t* table,
    gs_hk_beacon_parameter_t* parameter,
    gs_param_table_instance_t* tmp_table) {

    gs_error_t error = resolve_parameter_name(parameter, table);
    if (error != GS_OK) {
        log_warning("could not find parameter in table, beacon cannot be "
                    "serialized, node=%i, id=%i, param=%s, error=%s",
                    table->collected_table_node, table->collected_table_idx,
                    parameter->name, gs_error_string(error));
        return error;
    }

    // fetch value
    uint8_t array_idx = parameter->array_idx_begin;
    do {
        uint8_t item[parameter->p_row->size];
        uint8_t array_offset = parameter->p_row->size * array_idx;
        uint16_t offset = parameter->p_row->addr + array_offset;
        error = gs_param_get(tmp_table,
                             offset,
                             parameter->p_row->type, item,
                             parameter->p_row->size, 0);
        if (error == GS_OK) {
            log_trace("serializing parameter value into beacon packet, "
                      "name=%s, offset=%i, size=%i",
                      parameter->name, offset,
                      parameter->p_row->size);

            unsigned int pos = bcn_packet->length;
            error = gs_param_serialize_item(parameter->p_row,
                                            offset,
                                            item,
                                            F_TO_BIG_ENDIAN | F_PACKED,
                                            bcn_packet->data,
                                            GS_HK_BEACON_PACKET_MAX_DATA_SIZE,
                                            &pos);
            if (error == GS_OK) {
                log_trace("serialization returned GS_OK, pos=%i", pos);
                bcn_packet->length = pos;
            } else {
                log_error("gs_param_serialize_item() failed, node=%i, id=%i, "
                          "param=%s", table->collected_table_node,
                          table->collected_table_idx, parameter->name);
            }
        } else {
            log_warning("gs_param_get failed, cannot serialize,"
                        "node=%i, id=%i, offset=%i, size=%i, param=%s, error=%s",
                        table->collected_table_node, table->collected_table_idx,
                        offset, parameter->p_row->size, parameter->name,
                        gs_error_string(error));
            break;
        }
    } while (++array_idx <= parameter->array_idx_end);
    return error;
}

void gs_hk_beacon_init(void)
{
    gs_log_group_register(LOG_DEFAULT);
}

csp_packet_t* create_bcn_packet(gs_hk_beacon_cfg_t* bcn,
                                uint32_t requested_time, uint16_t satid) {
    if (bcn == NULL) {
        return NULL;
    }

    csp_packet_t* bcn_packet = alloc_csp_packet_and_create_header(bcn, satid);
    if (bcn_packet == NULL) {
        log_error("failed to allocate buffer for beacon packet, type=%i",
                  bcn->type);
        return NULL;
    }

    /* loop through all beacon data entries */
    gs_error_t error = GS_OK;
    if (bcn->data_elements != NULL) {
        gs_hk_beacon_data_element_t* elem = NULL;
        LL_FOREACH(bcn->data_elements, elem) {

            if (!elem->table ||
                !gs_hk_config_is_table_spec_downloaded(elem->table)) {
                error = GS_ERROR_NO_DATA;
                log_debug("No table samples exists, cannot create beacon, "
                          "type=%i", bcn->type);
                break;
            }

            gs_param_table_instance_t* tmp_table;
            bool free_tmp_table = false;
            if (requested_time > 0) {
                tmp_table = gs_param_table_instance_alloc();
                free_tmp_table = true;
            } else {
                if (elem->table != NULL &&
                    elem->table->collected_table != NULL) {
                    tmp_table = elem->table->collected_table;
                } else {
                    error = GS_ERROR_NOT_FOUND;
                    break;
                }
            }

            error = prepare_table_sample_and_create_data_segment_header(
                elem->table, requested_time, bcn_packet, tmp_table);
            if (error != GS_OK) {
                if (free_tmp_table) {
                    free(tmp_table);
                }
                break;
            }

            gs_hk_beacon_parameter_t* parameter = NULL;
            LL_FOREACH(elem->parameters, parameter) {
                error = serialize_parameter_value_into_beacon(bcn_packet,
                                                              elem->table,
                                                              parameter,
                                                              tmp_table);
                if (error != GS_OK) {
                    break;
                }
            }

            if (free_tmp_table) {
                free(tmp_table);
            }

            if (error != GS_OK) {
                break;
            }
        }
    }

    if (error != GS_OK) {
        csp_buffer_free(bcn_packet);
        bcn_packet = NULL;
    }

    return bcn_packet;
}

