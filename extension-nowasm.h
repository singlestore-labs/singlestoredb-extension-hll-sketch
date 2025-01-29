#ifndef __BINDINGS_EXTENSION_H
#define __BINDINGS_EXTENSION_H

#ifdef NOTWASM
#  include <stddef.h>
#  include <string.h>
#  define ADDR int64_t
#else
#  define ADDR int32_t
#endif
#define HANDLE ADDR
#define SIZE   ADDR

#ifdef __cplusplus
extern "C"
{
  #endif

  #include <stdint.h>
  #include <stdbool.h>
  typedef HANDLE extension_state_t;
  typedef struct {
    uint8_t *ptr;
    size_t len;
  } extension_list_u8_t;
  void extension_list_u8_free(extension_list_u8_t *ptr);
  double extension_sketch_estimate(extension_list_u8_t *data);
  extension_state_t extension_sketch_agg_init_handle(void);
  extension_state_t extension_sketch_agg_update_handle(extension_state_t state, extension_list_u8_t *input);
  extension_state_t extension_sketch_agg_merge_handle(extension_state_t left, extension_state_t right);
  void extension_sketch_agg_serialize_handle(extension_state_t state, extension_list_u8_t *ret0);
  extension_state_t extension_sketch_agg_deserialize_handle(extension_list_u8_t *data);

  typedef struct {
    char *ptr;
    size_t len;
  } extension_string_t;

  void extension_string_set(extension_string_t *ret, const char *s);
  void extension_string_dup(extension_string_t *ret, const char *s);
  void extension_string_free(extension_string_t *ret);
  void extension_sketch_to_string(extension_list_u8_t *data, extension_string_t *ret0);

  #ifdef __cplusplus
}
#endif
#endif
