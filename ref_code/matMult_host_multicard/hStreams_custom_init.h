/*
 * Copyright 2014-2016 Intel Corporation.
 *
 * This file is subject to the Intel Sample Source Code License. A copy
 * of the Intel Sample Source Code License is included.
 */

#ifndef CUST_INIT_H
#define CUST_INIT_H

#include <hStreams_source.h>

// Caveats:
// - First N physical domains may have more logical domains in them
//      (N = num_log_dom % num_phys_dom)
// - Last logical domain in a physical domain will may be wider by
//      num_threads_in_phys_dom % num_log_dom_in_that_phys_dom
// - First M places in a logical domain may have more threads in them
//      (M = width_of_log_dom % num_places_in_that_log_dom)
HSTR_RESULT
hStreams_app_init_selected_domains(
    uint32_t num_phys_domains,
    HSTR_PHYS_DOM *phys_domains_of_interest,
    uint32_t num_log_domains,
    uint32_t *places_per_log_domain,
    uint32_t log_streams_per_place)
{
    HSTR_RESULT ret = hStreams_Init();
    if (ret != HSTR_RESULT_SUCCESS) {
        return ret;
    }

    uint32_t num_places = 0;
    for (uint32_t i = 0; i < num_log_domains; ++i) {
        num_places += *(places_per_log_domain + i);
    }

    uint32_t ldom_low = num_log_domains / num_phys_domains;
    uint32_t ldom_high = (num_phys_domains % num_log_domains) ? ldom_low + 1 : ldom_low;
    uint32_t last_high_pdom_idx = num_log_domains % num_phys_domains;
    uint32_t global_place_idx = 0;
    uint32_t global_ldom_idx = 0;

    // Check that all the requested physical domains exist before creating anything
    for (uint32_t pdom_idx = 0; pdom_idx < num_phys_domains; ++pdom_idx) {
        HSTR_PHYS_DOM curr_pdom = *(phys_domains_of_interest + pdom_idx);
        uint32_t num_threads, max_freq;
        uint64_t mem_types, mem_avail[HSTR_MEM_TYPE_SIZE];
        HSTR_CPU_MASK max_mask, avd_mask;
        HSTR_ISA_TYPE isa;

        ret = hStreams_GetPhysDomainDetails(curr_pdom, &num_threads, &isa, &max_freq,
                                            max_mask, avd_mask, &mem_types, mem_avail);
        if (ret != HSTR_RESULT_SUCCESS) {
            return ret;
        }
    }
    // Same loop, this time creating the domains/streams/whatnot
    for (uint32_t pdom_idx = 0; pdom_idx < num_phys_domains; ++pdom_idx) {
        HSTR_PHYS_DOM curr_pdom = *(phys_domains_of_interest + pdom_idx);
        uint32_t num_threads, max_freq;
        uint64_t mem_types, mem_avail[HSTR_MEM_TYPE_SIZE];
        HSTR_CPU_MASK max_mask, avd_mask;
        HSTR_ISA_TYPE isa;

        ret = hStreams_GetPhysDomainDetails(curr_pdom, &num_threads, &isa, &max_freq,
                                            max_mask, avd_mask, &mem_types, mem_avail);
        if (ret != HSTR_RESULT_SUCCESS) {
            return ret;
        }
        // How many logical domains will we create in this physical domain?
        uint32_t ldom_cnt = (pdom_idx >= last_high_pdom_idx) ? ldom_low : ldom_high;
        // Get the CPU mask that we'll use for this particular logical domain
        // Last logical domain will have more threads (if needed)
        HSTR_CPU_MASK use_mask;
        HSTR_CPU_MASK_XOR(use_mask, max_mask, avd_mask);
        uint32_t ldom_width_low = HSTR_CPU_MASK_COUNT(use_mask) / ldom_cnt;
        uint32_t ldom_width_high = ldom_width_low + HSTR_CPU_MASK_COUNT(use_mask) % ldom_cnt;

        // Bit we're operating on for the logical domain mask
        int ldom_bit_idx = 0;
        for (uint32_t ldom_idx = 0; ldom_idx < ldom_cnt; ++ldom_idx, ++global_ldom_idx) {
            uint32_t ldom_width = (ldom_idx == ldom_cnt - 1) ? ldom_width_high : ldom_width_low;
            uint32_t ldom_places_cnt = *(places_per_log_domain + global_ldom_idx);
            // how about a nice error code and message?
            HSTR_CPU_MASK ldom_mask;
            HSTR_CPU_MASK_ZERO(ldom_mask);
            for (uint32_t bits_set = 0; bits_set < ldom_width; ++bits_set) {
                // Get to the next bit which is set
                while (!HSTR_CPU_MASK_ISSET(ldom_bit_idx, use_mask)) {
                    ++ldom_bit_idx;
                }
                HSTR_CPU_MASK_SET(ldom_bit_idx, ldom_mask);
                ++ldom_bit_idx;
            }
            HSTR_LOG_DOM ldom_id;
            HSTR_OVERLAP_TYPE ldom_overlap;
            ret = hStreams_AddLogDomain(curr_pdom, ldom_mask, &ldom_id, &ldom_overlap);
            if (ret != HSTR_RESULT_SUCCESS) {
                return ret;
            }
            uint32_t place_width_low = ldom_width / ldom_places_cnt;
            uint32_t place_width_high = (ldom_width % ldom_places_cnt) ? place_width_low + 1 : place_width_low;
            uint32_t last_high_place_idx = ldom_width % ldom_places_cnt;
            // Bit we're operating on for the place mask
            int place_bit_idx = 0;
            for (uint32_t place_idx = 0; place_idx < ldom_places_cnt; ++place_idx, ++global_place_idx) {
                uint32_t place_width = (place_idx >= last_high_place_idx) ? place_width_low : place_width_high;
                HSTR_CPU_MASK place_mask;
                HSTR_CPU_MASK_ZERO(place_mask);
                for (uint32_t bits_set = 0; bits_set < place_width; ++bits_set) {
                    // Get to the next bit which is set
                    while (!HSTR_CPU_MASK_ISSET(place_bit_idx, ldom_mask)) {
                        ++place_bit_idx;
                    }
                    HSTR_CPU_MASK_SET(place_bit_idx, place_mask);
                    ++place_bit_idx;
                }
                // Now, add streams for this place.
                for (uint32_t lstr_idx = 0; lstr_idx < log_streams_per_place; ++lstr_idx) {
                    HSTR_LOG_STR lstr_id = global_place_idx + lstr_idx * num_places;
                    ret = hStreams_StreamCreate(lstr_id, ldom_id, place_mask);
                    //ret = hStreams_StreamCreate(lstr_id, 0, place_mask);
                    if (ret != HSTR_RESULT_SUCCESS) {
                        return ret;
                    }
                }
            }
        }
    }
    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
hStreams_custom_init_selected_domains(
    uint32_t num_phys_domains,
    HSTR_PHYS_DOM *phys_domains_of_interest,
    uint32_t num_log_domains,
    uint32_t *places_per_log_domain,
    uint32_t log_streams_per_place,
    HSTR_CPU_MASK src_cpu_mask)
{
    HSTR_RESULT ret = hStreams_Init();
    if (ret != HSTR_RESULT_SUCCESS) {
        return ret;
    }

    uint32_t num_places = 0;
    for (uint32_t i = 0; i < num_log_domains; ++i) {
        num_places += places_per_log_domain[i];
    }

    uint32_t ldom_low = num_log_domains / num_phys_domains;
    uint32_t ldom_high = (num_phys_domains % num_log_domains) ? ldom_low + 1 : ldom_low;
    uint32_t last_high_pdom_idx = num_log_domains % num_phys_domains;
    uint32_t global_place_idx = 0;
    uint32_t global_ldom_idx = 0;

    // Check that all the requested physical domains exist before creating anything
    for (uint32_t pdom_idx = 0; pdom_idx < num_phys_domains; ++pdom_idx) {
        HSTR_PHYS_DOM curr_pdom = phys_domains_of_interest[pdom_idx];
        uint32_t num_threads, max_freq;
        uint64_t mem_types, mem_avail[HSTR_MEM_TYPE_SIZE];
        HSTR_CPU_MASK max_mask, avd_mask;
        HSTR_ISA_TYPE isa;

        ret = hStreams_GetPhysDomainDetails(curr_pdom, &num_threads, &isa, &max_freq,
                                            max_mask, avd_mask, &mem_types, mem_avail);
        if (ret != HSTR_RESULT_SUCCESS) {
            return ret;
        }
    }
    // Same loop, this time creating the domains/streams/whatnot
    for (uint32_t pdom_idx = 0; pdom_idx < num_phys_domains; ++pdom_idx) {
        HSTR_PHYS_DOM curr_pdom = phys_domains_of_interest[pdom_idx];
        uint32_t num_threads, max_freq;
        uint64_t mem_types, mem_avail[HSTR_MEM_TYPE_SIZE];
        HSTR_CPU_MASK max_mask, avd_mask;
        HSTR_ISA_TYPE isa;

        ret = hStreams_GetPhysDomainDetails(curr_pdom, &num_threads, &isa, &max_freq,
                                            max_mask, avd_mask, &mem_types, mem_avail);
        if (ret != HSTR_RESULT_SUCCESS) {
            return ret;
        }
        // How many logical domains will we create in this physical domain?
        uint32_t ldom_cnt = (pdom_idx >= last_high_pdom_idx) ? ldom_low : ldom_high;
        // Get the CPU mask that we'll use for this particular logical domain
        // Last logical domain will have more threads (if needed)
        HSTR_CPU_MASK use_mask;

        //gbansal
        if (pdom_idx == 0) {
            HSTR_CPU_MASK_OR(avd_mask, avd_mask, src_cpu_mask);
        }

        HSTR_CPU_MASK_XOR(use_mask, max_mask, avd_mask);
        uint32_t ldom_width_low = HSTR_CPU_MASK_COUNT(use_mask) / ldom_cnt;
        uint32_t ldom_width_high = ldom_width_low + HSTR_CPU_MASK_COUNT(use_mask) % ldom_cnt;

        // Bit we're operating on for the logical domain mask
        int ldom_bit_idx = 0;
        for (uint32_t ldom_idx = 0; ldom_idx < ldom_cnt; ++ldom_idx, ++global_ldom_idx) {
            uint32_t ldom_width = (ldom_idx == ldom_cnt - 1) ? ldom_width_high : ldom_width_low;
            uint32_t ldom_places_cnt = places_per_log_domain[global_ldom_idx];
            // how about a nice error code and message?
            HSTR_CPU_MASK ldom_mask;
            HSTR_CPU_MASK_ZERO(ldom_mask);
            for (uint32_t bits_set = 0; bits_set < ldom_width; ++bits_set) {
                // Get to the next bit which is set
                while (!HSTR_CPU_MASK_ISSET(ldom_bit_idx, use_mask)) {
                    ++ldom_bit_idx;
                }
                HSTR_CPU_MASK_SET(ldom_bit_idx, ldom_mask);
                ++ldom_bit_idx;
            }
            HSTR_LOG_DOM ldom_id;
            HSTR_OVERLAP_TYPE ldom_overlap;
            ret = hStreams_AddLogDomain(curr_pdom, ldom_mask, &ldom_id, &ldom_overlap);
            if (ret != HSTR_RESULT_SUCCESS) {
                return ret;
            }
            uint32_t place_width_low = ldom_width / ldom_places_cnt;
            uint32_t place_width_high = (ldom_width % ldom_places_cnt) ? place_width_low + 1 : place_width_low;
            uint32_t last_high_place_idx = ldom_width % ldom_places_cnt;
            // Bit we're operating on for the place mask
            int place_bit_idx = 0;
            for (uint32_t place_idx = 0; place_idx < ldom_places_cnt; ++place_idx, ++global_place_idx) {
                uint32_t place_width = (place_idx >= last_high_place_idx) ? place_width_low : place_width_high;
                HSTR_CPU_MASK place_mask;
                HSTR_CPU_MASK place_mask0;
                HSTR_CPU_MASK_ZERO(place_mask);
                HSTR_CPU_MASK_ZERO(place_mask0);
                for (uint32_t bits_set = 0; bits_set < place_width; ++bits_set) {
                    // Get to the next bit which is set
                    while (!HSTR_CPU_MASK_ISSET(place_bit_idx, ldom_mask)) {
                        ++place_bit_idx;
                    }
                    HSTR_CPU_MASK_SET(place_bit_idx, place_mask);
                    //gbansal - avoid host thread
                    ++place_bit_idx;
                }
                //HSTR_CPU_MASK_XOR(place_mask, place_mask0, src_cpu_mask);
                // Now, add streams for this place.
                for (uint32_t lstr_idx = 0; lstr_idx < log_streams_per_place; ++lstr_idx) {
                    HSTR_LOG_STR lstr_id = global_place_idx + lstr_idx * num_places;
                    ret = hStreams_StreamCreate(lstr_id, ldom_id, place_mask);
                    if (ret != HSTR_RESULT_SUCCESS) {
                        return ret;
                    }
                }
            }
        }
    }
    return HSTR_RESULT_SUCCESS;
}
#endif /* CUST_INIT_H */
