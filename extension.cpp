#include <stdlib.h>
#include <extension.h>

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
#include <string.h>

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
void extension_list_u8_free(extension_list_u8_t *ptr) {
  canonical_abi_free(ptr->ptr, ptr->len * 1, 1);
}

__attribute__((aligned(4)))
static uint8_t RET_AREA[8];
__attribute__((export_name("sketch-get-estimate")))
double __wasm_export_extension_sketch_get_estimate(int32_t arg, int32_t arg0) {
  extension_list_u8_t arg1 = (extension_list_u8_t) { (uint8_t*)(arg), (size_t)(arg0) };
  double ret = extension_sketch_get_estimate(&arg1);
  return ret;
}
__attribute__((export_name("sketch-get-estimate-emptyisnull")))
double __wasm_export_extension_sketch_get_estimate_emptyisnull(int32_t arg, int32_t arg0) {
  extension_list_u8_t arg1 = (extension_list_u8_t) { (uint8_t*)(arg), (size_t)(arg0) };
  double ret = extension_sketch_get_estimate_emptyisnull(&arg1);
  return ret;
}
__attribute__((export_name("sketch-union")))
int32_t __wasm_export_extension_sketch_union(int32_t arg, int32_t arg0, int32_t arg1, int32_t arg2) {
  extension_list_u8_t arg3 = (extension_list_u8_t) { (uint8_t*)(arg), (size_t)(arg0) };
  extension_list_u8_t arg4 = (extension_list_u8_t) { (uint8_t*)(arg1), (size_t)(arg2) };
  extension_list_u8_t ret;
  extension_sketch_union(&arg3, &arg4, &ret);
  int32_t ptr = (int32_t) &RET_AREA;
  *((int32_t*)(ptr + 4)) = (int32_t) (ret).len;
  *((int32_t*)(ptr + 0)) = (int32_t) (ret).ptr;
  return ptr;
}
__attribute__((export_name("sketch-union-emptyisnull")))
int32_t __wasm_export_extension_sketch_union_emptyisnull(int32_t arg, int32_t arg0, int32_t arg1, int32_t arg2) {
  extension_list_u8_t arg3 = (extension_list_u8_t) { (uint8_t*)(arg), (size_t)(arg0) };
  extension_list_u8_t arg4 = (extension_list_u8_t) { (uint8_t*)(arg1), (size_t)(arg2) };
  extension_list_u8_t ret;
  extension_sketch_union_emptyisnull(&arg3, &arg4, &ret);
  int32_t ptr = (int32_t) &RET_AREA;
  *((int32_t*)(ptr + 4)) = (int32_t) (ret).len;
  *((int32_t*)(ptr + 0)) = (int32_t) (ret).ptr;
  return ptr;
}
__attribute__((export_name("sketch-hash")))
int64_t __wasm_export_extension_sketch_hash(int32_t arg, int32_t arg0) {
  extension_list_u8_t arg1 = (extension_list_u8_t) { (uint8_t*)(arg), (size_t)(arg0) };
  uint64_t ret = extension_sketch_hash(&arg1);
  return (int64_t) (ret);
}
__attribute__((export_name("sketch-hash-emptyisnull")))
int64_t __wasm_export_extension_sketch_hash_emptyisnull(int32_t arg, int32_t arg0) {
  extension_list_u8_t arg1 = (extension_list_u8_t) { (uint8_t*)(arg), (size_t)(arg0) };
  uint64_t ret = extension_sketch_hash_emptyisnull(&arg1);
  return (int64_t) (ret);
}
__attribute__((export_name("sketch-to-string")))
int32_t __wasm_export_extension_sketch_to_string(int32_t arg, int32_t arg0) {
  extension_list_u8_t arg1 = (extension_list_u8_t) { (uint8_t*)(arg), (size_t)(arg0) };
  extension_string_t ret;
  extension_sketch_to_string(&arg1, &ret);
  int32_t ptr = (int32_t) &RET_AREA;
  *((int32_t*)(ptr + 4)) = (int32_t) (ret).len;
  *((int32_t*)(ptr + 0)) = (int32_t) (ret).ptr;
  return ptr;
}
__attribute__((export_name("sketch-to-string-emptyisnull")))
int32_t __wasm_export_extension_sketch_to_string_emptyisnull(int32_t arg, int32_t arg0) {
  extension_list_u8_t arg1 = (extension_list_u8_t) { (uint8_t*)(arg), (size_t)(arg0) };
  extension_string_t ret;
  extension_sketch_to_string_emptyisnull(&arg1, &ret);
  int32_t ptr = (int32_t) &RET_AREA;
  *((int32_t*)(ptr + 4)) = (int32_t) (ret).len;
  *((int32_t*)(ptr + 0)) = (int32_t) (ret).ptr;
  return ptr;
}
__attribute__((export_name("sketch-handle-init")))
int32_t __wasm_export_extension_sketch_handle_init(void) {
  extension_state_t ret = extension_sketch_handle_init();
  return ret;
}
__attribute__((export_name("sketch-handle-build-accum")))
int32_t __wasm_export_extension_sketch_handle_build_accum(int32_t arg, int32_t arg0, int32_t arg1) {
  extension_list_u8_t arg2 = (extension_list_u8_t) { (uint8_t*)(arg0), (size_t)(arg1) };
  extension_state_t ret = extension_sketch_handle_build_accum(arg, &arg2);
  return ret;
}
__attribute__((export_name("sketch-handle-build-accum-emptyisnull")))
int32_t __wasm_export_extension_sketch_handle_build_accum_emptyisnull(int32_t arg, int32_t arg0, int32_t arg1) {
  extension_list_u8_t arg2 = (extension_list_u8_t) { (uint8_t*)(arg0), (size_t)(arg1) };
  extension_state_t ret = extension_sketch_handle_build_accum_emptyisnull(arg, &arg2);
  return ret;
}
__attribute__((export_name("sketch-handle-union-accum")))
int32_t __wasm_export_extension_sketch_handle_union_accum(int32_t arg, int32_t arg0, int32_t arg1) {
  extension_list_u8_t arg2 = (extension_list_u8_t) { (uint8_t*)(arg0), (size_t)(arg1) };
  extension_state_t ret = extension_sketch_handle_union_accum(arg, &arg2);
  return ret;
}
__attribute__((export_name("sketch-handle-union-accum-emptyisnull")))
int32_t __wasm_export_extension_sketch_handle_union_accum_emptyisnull(int32_t arg, int32_t arg0, int32_t arg1) {
  extension_list_u8_t arg2 = (extension_list_u8_t) { (uint8_t*)(arg0), (size_t)(arg1) };
  extension_state_t ret = extension_sketch_handle_union_accum_emptyisnull(arg, &arg2);
  return ret;
}
__attribute__((export_name("sketch-handle-merge")))
int32_t __wasm_export_extension_sketch_handle_merge(int32_t arg, int32_t arg0) {
  extension_state_t ret = extension_sketch_handle_merge(arg, arg0);
  return ret;
}
__attribute__((export_name("sketch-handle-serialize")))
int32_t __wasm_export_extension_sketch_handle_serialize(int32_t arg) {
  extension_list_u8_t ret;
  extension_sketch_handle_serialize(arg, &ret);
  int32_t ptr = (int32_t) &RET_AREA;
  *((int32_t*)(ptr + 4)) = (int32_t) (ret).len;
  *((int32_t*)(ptr + 0)) = (int32_t) (ret).ptr;
  return ptr;
}
__attribute__((export_name("sketch-handle-deserialize")))
int32_t __wasm_export_extension_sketch_handle_deserialize(int32_t arg, int32_t arg0) {
  extension_list_u8_t arg1 = (extension_list_u8_t) { (uint8_t*)(arg), (size_t)(arg0) };
  extension_state_t ret = extension_sketch_handle_deserialize(&arg1);
  return ret;
}
