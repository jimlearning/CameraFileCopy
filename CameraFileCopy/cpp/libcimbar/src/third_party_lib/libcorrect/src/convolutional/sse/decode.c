#include "correct/convolutional/sse/convolutional.h"
#ifndef CIMBAR_IOS_PLATFORM
#include <arm_neon.h>
#else
#include "sse_compat.h"
#endif

static void convolutional_sse_decode_inner(correct_convolutional_sse *sse_conv, unsigned int sets,
                                           const uint8_t *soft) {
    correct_convolutional *conv = &sse_conv->base_conv;
    shift_register_t highbit = 1 << (conv->order - 1);
    unsigned int hist_buf_index = conv->history_buffer->index;
    unsigned int hist_buf_cap = conv->history_buffer->cap;
    unsigned int hist_buf_len = conv->history_buffer->len;
    unsigned int hist_buf_rn_int = conv->history_buffer->renormalize_interval;
    unsigned int hist_buf_rn_cnt = conv->history_buffer->renormalize_counter;
    for (unsigned int i = conv->order - 1; i < (sets - conv->order + 1); i++) {
        distance_t *distances = conv->distances;
        // lasterrors are the aggregate bit errors for the states of
        // shiftregister for the previous time slice
        if (soft) {
            if (conv->soft_measurement == CORRECT_SOFT_LINEAR) {
                for (unsigned int j = 0; j < 1 << (conv->rate); j++) {
                    distances[j] =
                        metric_soft_distance_linear(j, soft + i * conv->rate, conv->rate);
                }
            } else {
                for (unsigned int j = 0; j < 1 << (conv->rate); j++) {
                    distances[j] =
                        metric_soft_distance_quadratic(j, soft + i * conv->rate, conv->rate);
                }
            }
        } else {
            unsigned int out = bit_reader_read(conv->bit_reader, conv->rate);
            for (unsigned int i = 0; i < 1 << (conv->rate); i++) {
                distances[i] = metric_distance(i, out);
            }
        }
        oct_lookup_t oct_lookup = sse_conv->oct_lookup;
        oct_lookup_fill_distance(oct_lookup, distances);

        // a mask to get the high order bit from the shift register
        unsigned int num_iter = highbit << 1;
        const distance_t *read_errors = conv->errors->read_errors;
        // aggregate bit errors for this time slice
        distance_t *write_errors = conv->errors->write_errors;

        uint8_t *history = conv->history_buffer->history[hist_buf_index];
        ;
        // walk through all states, ignoring oldest bit
        // we will track a best register state (path) and the number of bit
        // errors at that path at this time slice
        // this loop considers two paths per iteration (high order bit set,
        // clear)
        // so, it only runs numstates/2 iterations
        // we'll update the history for every state and find the path with the
        // least aggregated bit errors

        // now run the main loop
        // we calculate 2 sets of 2 register states here (4 states per iter)
        // this creates 2 sets which share a predecessor, and 2 sets which share
        // a successor
        //
        // the first set definition is the two states that are the same except
        // for the least order bit
        // these two share a predecessor because their high n - 1 bits are the
        // same (differ only by newest bit)
        //
        // the second set definition is the two states that are the same except
        // for the high order bit
        // these two share a successor because the oldest high order bit will be
        // shifted out, and the other bits will be present in the successor
        //
        shift_register_t highbase = highbit >> 1;
        shift_register_t oct_highbase = highbase >> 2;
        for (shift_register_t low = 0, high = highbit, base = 0, oct = 0; high < num_iter;
             low += 32, high += 32, base += 16, oct += 4) {
            // shifted-right ancestors
            // low and low_plus_one share low_past_error
            //   note that they are the same when shifted right by 1
            // same goes for high and high_plus_one
            uint8_t past_shuffle_mask_data[] = {0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00};
            uint8x16_t past_shuffle_mask = vld1q_u8((const uint8_t*)past_shuffle_mask_data);

            uint8_t hist_mask_data[] = {0x07, 0x05, 0x03, 0x01, 0x0e, 0x0c, 0x0a, 0x09, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08};
            uint8x16_t hist_mask = vld1q_u8((const uint8_t*)hist_mask_data);
            
            uint32x4_t low_past_error =
                vld1q_u32((const uint32_t *)(read_errors + base + base_offset));
            
            // 修正函数调用
            low_past_error = vqtbl1q_u8(low_past_error, past_shuffle_mask);

            // 修正指针类型
            __m128i *table = (uint8x16_t *)oct_lookup.table;

            // the loop below calculates 64 register states per loop iteration
            // it does this by packing the 128-bit xmm registers with 8, 16-bit
            // distances
            // 4 of these registers hold distances for convolutional shift
            // register states with the high bit cleared
            //      and 4 hold distances for the corresponding shift register
            //      states with the high bit set
            // since each xmm register holds 8 distances, this adds up to a
            // total of 8 * 8 = 64 shift register states
            for (shift_register_t offset = 0, base_offset = 0; base_offset < 16;
                 offset += 32, base_offset += 16) {
                // load the past error for the register states with the high
                // order bit cleared
                uint32x4_t low_past_error =
                    vld1q_u32((const uint32_t *)(read_errors + base + base_offset));
                uint32x4_t low_past_error0 =
                    vld1q_u32((const uint32_t *)(read_errors + base + base_offset + 4));
                uint32x4_t low_past_error1 =
                    vld1q_u32((const uint32_t *)(read_errors + base + base_offset + 8));
                uint32x4_t low_past_error2 =
                    vld1q_u32((const uint32_t *)(read_errors + base + base_offset + 12));

                // shuffle the low past error
                // register states that differ only by their low order bit share
                // a past error
                low_past_error = vqtbl1q_u8(low_past_error, past_shuffle_mask);
                low_past_error0 = vqtbl1q_u8(low_past_error0, past_shuffle_mask);
                low_past_error1 = vqtbl1q_u8(low_past_error1, past_shuffle_mask);
                low_past_error2 = vqtbl1q_u8(low_past_error2, past_shuffle_mask);

                // repeat past error lookup for register states with high order
                // bit set
                uint32x4_t high_past_error =
                    vld1q_u32((const uint32_t *)(read_errors + highbase + base + base_offset));
                uint32x4_t high_past_error0 = vld1q_u32(
                    (const uint32_t *)(read_errors + highbase + base + base_offset + 4));
                uint32x4_t high_past_error1 = vld1q_u32(
                    (const uint32_t *)(read_errors + highbase + base + base_offset + 8));
                uint32x4_t high_past_error2 = vld1q_u32(
                    (const uint32_t *)(read_errors + highbase + base + base_offset + 12));

                high_past_error = vqtbl1q_u8(high_past_error, past_shuffle_mask);
                high_past_error0 = vqtbl1q_u8(high_past_error0, past_shuffle_mask);
                high_past_error1 = vqtbl1q_u8(high_past_error1, past_shuffle_mask);
                high_past_error2 = vqtbl1q_u8(high_past_error2, past_shuffle_mask);

                // load the opaque oct distance table keys from out loop index
                distance_oct_key_t low_key = oct_lookup.keys[oct + (base_offset / 4)];
                distance_oct_key_t low_key0 = oct_lookup.keys[oct + (base_offset / 4) + 1];
                distance_oct_key_t low_key1 = oct_lookup.keys[oct + (base_offset / 4) + 2];
                distance_oct_key_t low_key2 = oct_lookup.keys[oct + (base_offset / 4) + 3];

                // load the distances for the register states with high order
                // bit cleared
                uint16x8_t low_this_error =
                    vld1q_u16((const uint16_t *)(oct_lookup.distances + low_key));
                uint16x8_t low_this_error0 =
                    vld1q_u16((const uint16_t *)(oct_lookup.distances + low_key0));
                uint16x8_t low_this_error1 =
                    vld1q_u16((const uint16_t *)(oct_lookup.distances + low_key1));
                uint16x8_t low_this_error2 =
                    vld1q_u16((const uint16_t *)(oct_lookup.distances + low_key2));

                // add the distance for this time slice to the past distances
                uint16x8_t low_error = vaddq_u16(low_past_error, low_this_error);
                uint16x8_t low_error0 = vaddq_u16(low_past_error0, low_this_error0);
                uint16x8_t low_error1 = vaddq_u16(low_past_error1, low_this_error1);
                uint16x8_t low_error2 = vaddq_u16(low_past_error2, low_this_error2);

                // repeat oct distance table lookup for registers with high
                // order bit set
                distance_oct_key_t high_key =
                    oct_lookup.keys[oct_highbase + oct + (base_offset / 4)];
                distance_oct_key_t high_key0 =
                    oct_lookup.keys[oct_highbase + oct + (base_offset / 4) + 1];
                distance_oct_key_t high_key1 =
                    oct_lookup.keys[oct_highbase + oct + (base_offset / 4) + 2];
                distance_oct_key_t high_key2 =
                    oct_lookup.keys[oct_highbase + oct + (base_offset / 4) + 3];

                uint16x8_t high_this_error =
                    vld1q_u16((const uint16_t *)(oct_lookup.distances + high_key));
                uint16x8_t high_this_error0 =
                    vld1q_u16((const uint16_t *)(oct_lookup.distances + high_key0));
                uint16x8_t high_this_error1 =
                    vld1q_u16((const uint16_t *)(oct_lookup.distances + high_key1));
                uint16x8_t high_this_error2 =
                    vld1q_u16((const uint16_t *)(oct_lookup.distances + high_key2));

                uint16x8_t high_error = vaddq_u16(high_past_error, high_this_error);
                uint16x8_t high_error0 = vaddq_u16(high_past_error0, high_this_error0);
                uint16x8_t high_error1 = vaddq_u16(high_past_error1, high_this_error1);
                uint16x8_t high_error2 = vaddq_u16(high_past_error2, high_this_error2);

                // distances for this time slice calculated

                // find the least error between registers who differ only in
                // their high order bit
                uint16x8_t min_error = vminq_u16(low_error, high_error);
                uint16x8_t min_error0 = vminq_u16(low_error0, high_error0);
                uint16x8_t min_error1 = vminq_u16(low_error1, high_error1);
                uint16x8_t min_error2 = vminq_u16(low_error2, high_error2);

                vst1q_u16((uint16_t *)(write_errors + low + offset), min_error);
                vst1q_u16((uint16_t *)(write_errors + low + offset + 8), min_error0);
                vst1q_u16((uint16_t *)(write_errors + low + offset + 16), min_error1);
                vst1q_u16((uint16_t *)(write_errors + low + offset + 24), min_error2);

                // generate history bits as (low_error > least_error)
                // this operation fills each element with all 1s if true and 0s
                // if false
                // in other words, we set the history bit to 1 if
                //      the register state with high order bit set was the least
                //      error
                uint8x16_t hist = vcgtq_u16(low_error, min_error);
                // pack the bits down from 16-bit wide to 8-bit wide to
                // accomodate history table
                hist = vtbl4q_u8(hist, hist_mask);

                uint8x16_t hist0 = vcgtq_u16(low_error0, min_error0);
                hist0 = vtbl4q_u8(hist0, hist_mask);

                uint8x16_t hist1 = vcgtq_u16(low_error1, min_error1);
                hist1 = vtbl4q_u8(hist1, hist_mask);

                uint8x16_t hist2 = vcgtq_u16(low_error2, min_error2);
                hist2 = vtbl4q_u8(hist2, hist_mask);

                // write the least error so that the next time slice sees it as
                // the past error
                // store the history bits set by cmp and shuffle operations
                vst1q_u8((uint8_t *)(history + low + offset), hist);
                vst1q_u8((uint8_t *)(history + low + offset + 8), hist0);
                vst1q_u8((uint8_t *)(history + low + offset + 16), hist1);
                vst1q_u8((uint8_t *)(history + low + offset + 24), hist2);
            }
        }

        // bypass the call to history buffer
        // we should really make that function inline and remove this below
        if (hist_buf_len == hist_buf_cap - 1 || hist_buf_rn_cnt == hist_buf_rn_int - 1) {
            // restore hist buffer state and invoke it
            conv->history_buffer->len = hist_buf_len;
            conv->history_buffer->index = hist_buf_index;
            conv->history_buffer->renormalize_counter = hist_buf_rn_cnt;
            history_buffer_process(conv->history_buffer, write_errors, conv->bit_writer);
            // restore our local values
            hist_buf_len = conv->history_buffer->len;
            hist_buf_index = conv->history_buffer->index;
            hist_buf_cap = conv->history_buffer->cap;
            hist_buf_rn_cnt = conv->history_buffer->renormalize_counter;
        } else {
            hist_buf_len++;
            hist_buf_index++;
            if (hist_buf_index == hist_buf_cap) {
                hist_buf_index = 0;
            }
            hist_buf_rn_cnt++;
        }
        error_buffer_swap(conv->errors);
    }
    conv->history_buffer->len = hist_buf_len;
    conv->history_buffer->index = hist_buf_index;
    conv->history_buffer->renormalize_counter = hist_buf_rn_cnt;
}

static void _convolutional_sse_decode_init(correct_convolutional_sse *conv,
                                           unsigned int min_traceback,
                                           unsigned int traceback_length,
                                           unsigned int renormalize_interval) {
    _convolutional_decode_init(&conv->base_conv, min_traceback, traceback_length,
                               renormalize_interval);
    conv->oct_lookup =
        oct_lookup_create(conv->base_conv.rate, conv->base_conv.order, conv->base_conv.table);
}

static ssize_t _convolutional_sse_decode(correct_convolutional_sse *sse_conv,
                                         size_t num_encoded_bits, size_t num_encoded_bytes,
                                         uint8_t *msg, const soft_t *soft_encoded) {
    correct_convolutional *conv = &sse_conv->base_conv;
    if (!conv->has_init_decode) {
        uint64_t max_error_per_input = conv->rate * soft_max;
        // sse implementation unfortunately uses signed math on our unsigned values
        // reduces usable distance by /2
        unsigned int renormalize_interval = (distance_max / 2) / max_error_per_input;
        _convolutional_sse_decode_init(sse_conv, 5 * conv->order, 100 * conv->order,
                                       renormalize_interval);
    }

    size_t sets = num_encoded_bits / conv->rate;
    // XXX fix this vvvvvv
    size_t decoded_len_bytes = num_encoded_bytes;
    bit_writer_reconfigure(conv->bit_writer, msg, decoded_len_bytes);

    error_buffer_reset(conv->errors);
    history_buffer_reset(conv->history_buffer);

    // no outputs are generated during warmup
    convolutional_decode_warmup(conv, sets, soft_encoded);
    convolutional_sse_decode_inner(sse_conv, sets, soft_encoded);
    convolutional_decode_tail(conv, sets, soft_encoded);

    history_buffer_flush(conv->history_buffer, conv->bit_writer);

    return bit_writer_length(conv->bit_writer);
}

ssize_t correct_convolutional_sse_decode(correct_convolutional_sse *conv, const uint8_t *encoded,
                                         size_t num_encoded_bits, uint8_t *msg) {
    if (num_encoded_bits % conv->base_conv.rate) {
        // XXX turn this into an error code
        // printf("encoded length of message must be a multiple of rate\n");
        return -1;
    }

    size_t num_encoded_bytes =
        (num_encoded_bits % 8) ? (num_encoded_bits / 8 + 1) : (num_encoded_bits / 8);
    bit_reader_reconfigure(conv->base_conv.bit_reader, encoded, num_encoded_bytes);

    return _convolutional_sse_decode(conv, num_encoded_bits, num_encoded_bytes, msg, NULL);
}

ssize_t correct_convolutional_sse_decode_soft(correct_convolutional_sse *conv, const soft_t *encoded,
                                              size_t num_encoded_bits, uint8_t *msg) {
    if (num_encoded_bits % conv->base_conv.rate) {
        // XXX turn this into an error code
        // printf("encoded length of message must be a multiple of rate\n");
        return -1;
    }

    size_t num_encoded_bytes =
        (num_encoded_bits % 8) ? (num_encoded_bits / 8 + 1) : (num_encoded_bits / 8);

    return _convolutional_sse_decode(conv, num_encoded_bits, num_encoded_bytes, msg, encoded);
}
