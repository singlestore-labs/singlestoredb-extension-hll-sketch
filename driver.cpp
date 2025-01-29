#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <extension-nowasm.h>
#include <unistd.h>


double __wasm_export_extension_sketch_get_estimate(ADDR arg, SIZE arg0);
HANDLE __wasm_export_extension_sketch_handle_init(void);
HANDLE __wasm_export_extension_sketch_handle_union_accum(HANDLE arg, ADDR arg0, SIZE arg1);
HANDLE __wasm_export_extension_sketch_handle_merge(HANDLE arg, HANDLE arg0);
ADDR __wasm_export_extension_sketch_handle_serialize(HANDLE arg);
HANDLE __wasm_export_extension_sketch_handle_deserialize(ADDR arg, SIZE arg0);
ADDR __wasm_export_extension_sketch_to_string(ADDR arg, SIZE arg0);

int main(int argc, char* argv[])
{
    HANDLE s1 = __wasm_export_extension_sketch_handle_init();
    printf("S1=%lld\n", s1);
    HANDLE u = s1;
    for (int i = 0; i < 1000000; ++i)
    {
        int* ix = (int*) malloc(sizeof(int));
        *ix = i;
        u = __wasm_export_extension_sketch_handle_union_accum(u, (int64_t) ix, sizeof(int));
        //printf("u==%d\n", u);
    }
    s1 = u;
    printf("S1=%lld\n", s1);

    HANDLE s2 = __wasm_export_extension_sketch_handle_init();
    u = s2;
    for (int i = 0; i < 1000000; ++i)
    {
        int* ix = (int*) malloc(sizeof(int));
        *ix = i;
        u = __wasm_export_extension_sketch_handle_union_accum(u, (int64_t) ix, sizeof(int));
    }
    s2 = u;
    printf("S2=%lld\n", s2);

    //printf("SLEEPING...\n");
    //sleep(60);

    HANDLE m = __wasm_export_extension_sketch_handle_merge(s1, s2);
    printf("M=%lld\n", m);

    ADDR a = __wasm_export_extension_sketch_handle_serialize(m);
    ADDR p = *(ADDR*) ((char*) a);
    SIZE z = *(SIZE*) ((char*) a + 8);
    printf("SERIALIZED PTR=%lld, LENGTH=%lld\n", p, z);

    HANDLE deser = __wasm_export_extension_sketch_handle_deserialize(p, z);
    printf("DESER=%lld\n", deser);

    ADDR a2 = __wasm_export_extension_sketch_handle_serialize(deser);
    ADDR p2 = *(ADDR*) ((char*) a2);
    SIZE z2 = *(SIZE*) ((char*) a2 + 8);
    printf("SERIALIZED PTR=%lld, LENGTH=%lld\n", p2, z2);

    free((void*)p2);

    return 0;
}

int main1(int argc, char* argv[])
{
    HANDLE s1 = __wasm_export_extension_sketch_handle_init();
    printf("S1=%lld\n", s1);

#define THING "\x01\x03\x03\x00\x00\x3A\xCC\x93\x00\x01\xBF\xAB\xF4\x6A\x96\x0D"
    size_t size = sizeof(THING);
    char* ptr = (char*) malloc(size);
    memcpy(ptr, THING, size);

    HANDLE u = __wasm_export_extension_sketch_handle_union_accum(s1, (int64_t) ptr, size);
    printf("U=%lld\n", u);

    ADDR a = __wasm_export_extension_sketch_handle_serialize(u);
    ADDR p = *(ADDR*) ((char*) a);
    ADDR z = *(SIZE*) ((char*) a + 8);
    printf("SERIALIZED PTR=%lld, LENGTH=%lld\n", p, z);
    for (int i = 0; i < z; ++i)
    {
        printf("%02X", ((char*) p)[i] & 0xFF);
    }
    printf("\n");

    extension_string_t str;
    a = __wasm_export_extension_sketch_to_string(p, z);
    ADDR sp = *(ADDR*) ((char*) a); 
    ADDR sz = *(ADDR*) ((char*) a + 8); 
    printf("tostring=%.*s", (int) sz, (char*) sp);

    free((void*) p);
    free((void*) sp);

    return 0;
}

