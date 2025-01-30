#ifdef NOTWASM
#  include "extension-nowasm.h"
#else
#  include "extension.h"
#endif
#include "log.h"
#include "hll.hpp"
#include "MurmurHash3.h"
#include <unistd.h>

using namespace datasketches;

#define NO_HANDLE ((extension_state_t) -1)
#define TO_HND(x_) (reinterpret_cast<extension_state_t>(x_))
#define TO_PTR(x_) (reinterpret_cast<void*>(x_))
#define VALID_HANDLE(s_) (TO_HND(s_) != NO_HANDLE)
#define BAIL(...) { fprintf(stderr, __VA_ARGS__); abort(); }
#define BAIL_IF(cond_, ...) { if (cond_) { BAIL(__VA_ARGS__); } }

using s2_hll_sketch = hll_sketch;
using s2_hll_union = hll_union;

#define hll_sketch                 s2_hll_sketch
#define hll_union                  s2_hll_union

const int DEFAULT_LG_K = 12;

static void* hll_sketch_new_default()
{
    return new hll_sketch(DEFAULT_LG_K);
}

static void* hll_union_new_default()
{
    return new hll_union(DEFAULT_LG_K);
}

static void hll_sketch_delete(void* sketchptr)
{
    if (sketchptr)
    {
        delete static_cast<hll_sketch*>(sketchptr);
    }
}

static void hll_union_delete(void* unionptr)
{
    if (unionptr)
    {
        delete static_cast<hll_union*>(unionptr);
    }
}

static void hll_sketch_update(void* sketchptr, const void* data, unsigned length)
{
    static_cast<hll_sketch*>(sketchptr)->update(data, length);
}

static void hll_data_set_type(void* dataptr, agg_state_type t)
{
    static_cast<hll_data*>(dataptr)->set_type(t);
}

static agg_state_type hll_data_get_type(void* dataptr)
{
    return static_cast<hll_data*>(dataptr)->get_type();
}

static void* hll_union_get_result(void* unionptr)
{
    auto u = static_cast<hll_union*>(unionptr);
    auto sketchptr = new hll_sketch(u->get_result());
    hll_union_delete(u);
    return sketchptr;
}

static void* hll_data_get_result(void* dataptr)
{
    switch (hll_data_get_type(dataptr))
    {
        case UNION:
            return hll_union_get_result(dataptr);

        default:
            return dataptr;
    }
}

static void hll_sketch_serialize(const void* sketchptr, extension_list_u8_t* output)
{
    unsigned char* bytes;
    size_t size;
    auto s = static_cast<const hll_sketch*>(sketchptr);
    if (!s->serialize2(true /*compact*/, &bytes, &size))
    {
        BAIL("Failed to serialize hll sketch");
    }
    output->ptr = bytes;
    output->len = size;
}

static void* hll_sketch_deserialize(extension_list_u8_t* bytes)
{
    return new hll_sketch(hll_sketch::deserialize(bytes->ptr, bytes->len));
}

static void hll_union_update_with_sketch(void* unionptr, const void* sketchptr)
{
    static_cast<hll_union*>(unionptr)->update(*static_cast<const hll_sketch*>(sketchptr));
}

static void hll_union_update_with_bytes(void* unionptr, extension_list_u8_t* bytes)
{
    auto sketchptr = hll_sketch_deserialize(bytes);
    hll_data_set_type(sketchptr, SKETCH);
    hll_union_update_with_sketch(unionptr, sketchptr);
}

static double hll_sketch_get_estimate(const void* sketchptr)
{
    return static_cast<const hll_sketch*>(sketchptr)->get_estimate();
}

static void hll_sketch_to_string(void* sketchptr, extension_string_t *output)
{
    auto str = static_cast<const hll_sketch*>(sketchptr)->to_string();
    output->len = str.length() + 1;
    char* buffer = (char*) malloc(output->len);
    strncpy(buffer, str.c_str(), output->len);
    output->ptr = buffer;
}

extension_state_t extension_sketch_handle_init()
{
    return NO_HANDLE;
}

extension_state_t
extension_sketch_handle_build_accum(
    extension_state_t handle, 
    extension_list_u8_t *input)
{
    if (!input)
    {
        return handle;
    }

    void* state = TO_PTR(handle);
    if (!VALID_HANDLE(handle))
    {
        state = hll_sketch_new_default();
        hll_data_set_type(state, SKETCH);
    }
    hll_sketch_update(state, input->ptr, input->len);
    extension_list_u8_free(input);
    return TO_HND(state);
}

extension_state_t
extension_sketch_handle_build_accum_emptyisnull(
    extension_state_t handle, 
    extension_list_u8_t *input)
{
    // If input is empty string, no update needed.
    //
    if (!input->len)
    {
        extension_list_u8_free(input);
        input = nullptr;
    }
    return extension_sketch_handle_build_accum(handle, input);
}

extension_state_t
extension_sketch_handle_union_accum(
    extension_state_t handle,
    extension_list_u8_t *input)
{
    if (!input)
    {
        return handle;
    }

    void* state = TO_PTR(handle);
    if (!VALID_HANDLE(handle))
    {
        state = hll_union_new_default();
        hll_data_set_type(state, UNION);
    }
    hll_union_update_with_bytes(state, input);
    extension_list_u8_free(input);
    return TO_HND(state);
}

extension_state_t
extension_sketch_handle_union_accum_emptyisnull(
    extension_state_t handle,
    extension_list_u8_t *input)
{
    // If input is empty string, no update needed.
    //
    if (!input->len)
    {
        extension_list_u8_free(input);
        input = nullptr;
    }
    return extension_sketch_handle_union_accum(handle, input);
}

extension_state_t
extension_sketch_handle_merge(
    extension_state_t left,
    extension_state_t right)
{
    if (!VALID_HANDLE(left) && !VALID_HANDLE(right))
    {
        return NO_HANDLE;
    }

    void* u = hll_union_new_default();
    hll_data_set_type(u, UNION);
    if (VALID_HANDLE(left))
    {
        auto lptr = hll_data_get_result(TO_PTR(left));
        hll_union_update_with_sketch(u, lptr);
        hll_sketch_delete(lptr);
    }
    if (VALID_HANDLE(right))
    {
        auto rptr = hll_data_get_result(TO_PTR(right));
        hll_union_update_with_sketch(u, rptr);
        hll_sketch_delete(rptr);
    }

    return TO_HND(u);
}

void
extension_sketch_handle_serialize(
    extension_state_t handle,
    extension_list_u8_t *output)
{
    if (!VALID_HANDLE(handle))
    {
        output->ptr = nullptr;
        output->len = 0;
        return;
    }
    auto state = TO_PTR(handle);
    state = hll_data_get_result(state);
    hll_sketch_serialize(state, output);
    hll_sketch_delete(state);
}

extension_state_t
extension_sketch_handle_deserialize(
    extension_list_u8_t *data)
{
    extension_state_t res;
    if (data->len == 0)
    {
        res = NO_HANDLE;
    }
    else
    {
        auto stateptr = hll_sketch_deserialize(data);
        hll_data_set_type(stateptr, SKETCH);
        res = TO_HND(stateptr);
    }
    extension_list_u8_free(data);
    return res;
}

double extension_sketch_get_estimate(extension_list_u8_t* data)
{
    if (!data)
    {
        extension_list_u8_free(data);
        return 0.0;
    }
    //BAIL_IF(data->len < 8, "at least 8 bytes expected, actual %zu", data->len);

    auto sketchptr = hll_sketch_deserialize(data);
    auto estimate = hll_sketch_get_estimate(sketchptr);
    hll_sketch_delete(sketchptr);
    extension_list_u8_free(data);
    return estimate;
}

double extension_sketch_get_estimate_emptyisnull(extension_list_u8_t* data)
{
    if (!data->len)
    {
        extension_list_u8_free(data);
        data = nullptr;
    }
    return extension_sketch_get_estimate(data);
}

void
extension_sketch_union(
    extension_list_u8_t *left,
    extension_list_u8_t *right,
    extension_list_u8_t *output)
{
    void* unionptr = hll_union_new_default();
    if (left)
    {
        //BAIL_IF(left->len < 8, "at least 8 bytes expected, actual %zu", left->len);
        hll_union_update_with_bytes(unionptr, left);
    }
    if (right)
    {
        //BAIL_IF(right->len < 8, "at least 8 bytes expected, actual %zu", right->len);
        hll_union_update_with_bytes(unionptr, right);
    }

    void* sketchptr = hll_union_get_result(unionptr);
    hll_sketch_serialize(sketchptr, output);

    if (left)  extension_list_u8_free(left);
    if (right) extension_list_u8_free(right);
    hll_sketch_delete(sketchptr);
}

void
extension_sketch_union_emptyisnull(
    extension_list_u8_t *left,
    extension_list_u8_t *right,
    extension_list_u8_t *output)
{
    if (!left->len)
    {
        extension_list_u8_free(left);
        left = nullptr;
    }
    if (!right->len)
    {
        extension_list_u8_free(right);
        right = nullptr;
    }
    extension_sketch_union(left, right, output);
}

void 
extension_sketch_to_string(
    extension_list_u8_t *data,
    extension_string_t *ret)
{
    if (!data)
    {
        ret->ptr = nullptr;
        ret->len = 0;
        return;
    }

    auto sketchptr = hll_sketch_deserialize(data);
    hll_sketch_to_string(sketchptr, ret);
    hll_sketch_delete(sketchptr);
    extension_list_u8_free(data);
}

void 
extension_sketch_to_string_emptyisnull(
    extension_list_u8_t *data,
    extension_string_t *ret)
{
    if (!data->len)
    {
        extension_list_u8_free(data);
        data = nullptr;
    }
    extension_sketch_to_string(data, ret);
}

uint64_t extension_sketch_hash(extension_list_u8_t *data)
{
    if (!data)
    {
        return 0;
    }
    HashState hashes;
    MurmurHash3_x64_128(data->ptr, data->len, DEFAULT_SEED, hashes);
    extension_list_u8_free(data);
    return (hashes.h1 >> 1);
}

uint64_t extension_sketch_hash_emptyisnull(extension_list_u8_t *data)
{
    if (!data->len)
    {
        extension_list_u8_free(data);
        data = nullptr;
    }
    return extension_sketch_hash(data);
}

