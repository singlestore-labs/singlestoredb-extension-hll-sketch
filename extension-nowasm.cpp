#include <stdlib.h>
#include <extension-nowasm.h>

__attribute__((weak, export_name("canonical_abi_realloc")))
void *canonical_abi_realloc(
void *ptr,
size_t orig_size,
size_t org_align,
size_t new_size
) {
  void *ret = realloc(ptr, new_size);
  if (!ret)
  abort();
  return ret;
}

__attribute__((weak, export_name("canonical_abi_free")))
void canonical_abi_free(
void *ptr,
size_t size,
size_t align
) {
  free(ptr);
}
void extension_list_u8_free(extension_list_u8_t *ptr) {
  canonical_abi_free(ptr->ptr, ptr->len * 1, 1);
}

void extension_string_set(extension_string_t *ret, const char *s) {
  ret->ptr = (char*) s;
  ret->len = strlen(s);
}

void extension_string_dup(extension_string_t *ret, const char *s) {
  ret->len = strlen(s);
  ret->ptr = (char *)canonical_abi_realloc(NULL, 0, 1, ret->len);
  memcpy(ret->ptr, s, ret->len);
}

void extension_string_free(extension_string_t *ret) {
  canonical_abi_free(ret->ptr, ret->len, 1);
  ret->ptr = NULL;
  ret->len = 0;
}

__attribute__((aligned(4)))
static uint8_t RET_AREA[8];
__attribute__((export_name("sketch-estimate")))
double __wasm_export_extension_sketch_estimate(ADDR arg, SIZE arg0) {
  extension_list_u8_t arg1 = (extension_list_u8_t) { (uint8_t*)(arg), (size_t)(arg0) };
  double ret = extension_sketch_estimate(&arg1);
  return ret;
}
__attribute__((export_name("sketch-agg-init-handle")))
HANDLE __wasm_export_extension_sketch_agg_init_handle(void) {
  extension_state_t ret = extension_sketch_agg_init_handle();
  return ret;
}
__attribute__((export_name("sketch-agg-update-handle")))
HANDLE __wasm_export_extension_sketch_agg_update_handle(HANDLE arg, ADDR arg0, SIZE arg1) {
  extension_list_u8_t arg2 = (extension_list_u8_t) { (uint8_t*)(arg0), (size_t)(arg1) };
  extension_state_t ret = extension_sketch_agg_update_handle(arg, &arg2);
  return ret;
}
__attribute__((export_name("sketch-agg-merge-handle")))
HANDLE __wasm_export_extension_sketch_agg_merge_handle(HANDLE arg, HANDLE arg0) {
  extension_state_t ret = extension_sketch_agg_merge_handle(arg, arg0);
  return ret;
}
__attribute__((export_name("sketch-agg-serialize-handle")))
ADDR __wasm_export_extension_sketch_agg_serialize_handle(HANDLE arg) {
  extension_list_u8_t ret;
  extension_sketch_agg_serialize_handle(arg, &ret);
  ADDR ptr = (ADDR) &RET_AREA;
  *((ADDR*)(ptr + sizeof(ADDR))) = (ADDR) (ret).len;
  *((ADDR*)(ptr + 0)) = (ADDR) (ret).ptr;
  return ptr;
}
__attribute__((export_name("sketch-agg-deserialize-handle")))
HANDLE __wasm_export_extension_sketch_agg_deserialize_handle(ADDR arg, SIZE arg0) {
  extension_list_u8_t arg1 = (extension_list_u8_t) { (uint8_t*)(arg), (size_t)(arg0) };
  extension_state_t ret = extension_sketch_agg_deserialize_handle(&arg1);
  return ret;
}
__attribute__((export_name("sketch-to-string")))
ADDR __wasm_export_extension_sketch_to_string(ADDR arg, SIZE arg0) {
  extension_list_u8_t arg1 = (extension_list_u8_t) { (uint8_t*)(arg), (size_t)(arg0) };
  extension_string_t ret;
  extension_sketch_to_string(&arg1, &ret);
  ADDR ptr = (ADDR) &RET_AREA;
  *((ADDR*)(ptr + sizeof(ADDR))) = (ADDR) (ret).len;
  *((ADDR*)(ptr + 0)) = (ADDR) (ret).ptr;
  return ptr;
}
