#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <extension-nowasm.h>
#include <unistd.h>


double __wasm_export_extension_sketch_estimate(ADDR arg, SIZE arg0);
HANDLE __wasm_export_extension_sketch_agg_init_handle(void);
HANDLE __wasm_export_extension_sketch_agg_update_handle(HANDLE arg, ADDR arg0, SIZE arg1);
HANDLE __wasm_export_extension_sketch_agg_merge_handle(HANDLE arg, HANDLE arg0);
ADDR __wasm_export_extension_sketch_agg_serialize_handle(HANDLE arg);
HANDLE __wasm_export_extension_sketch_agg_deserialize_handle(ADDR arg, SIZE arg0);
ADDR __wasm_export_extension_sketch_to_string(ADDR arg, SIZE arg0);

int main(int argc, char* argv[])
{
    HANDLE s1 = __wasm_export_extension_sketch_agg_init_handle();
    printf("S1=%ld\n", s1);
    HANDLE u = s1;
    for (int i = 0; i < 1000000; ++i)
    {
        int* ix = (int*) malloc(sizeof(int));
        *ix = i;
        u = __wasm_export_extension_sketch_agg_update_handle(u, (int64_t) ix, sizeof(int));
        //printf("u==%d\n", u);
    }
    s1 = u;
    printf("S1=%ld\n", s1);

    HANDLE s2 = __wasm_export_extension_sketch_agg_init_handle();
    u = s2;
    for (int i = 0; i < 1000000; ++i)
    {
        int* ix = (int*) malloc(sizeof(int));
        *ix = i;
        u = __wasm_export_extension_sketch_agg_update_handle(u, (int64_t) ix, sizeof(int));
    }
    s2 = u;
    printf("S2=%ld\n", s2);

    //printf("SLEEPING...\n");
    //sleep(60);

    HANDLE m = __wasm_export_extension_sketch_agg_merge_handle(s1, s2);
    printf("M=%ld\n", m);

    ADDR a = __wasm_export_extension_sketch_agg_serialize_handle(m);
    ADDR p = *(ADDR*) ((char*) a);
    SIZE z = *(SIZE*) ((char*) a + 8);
    printf("SERIALIZED PTR=%ld, LENGTH=%ld\n", p, z);

    HANDLE deser = __wasm_export_extension_sketch_agg_deserialize_handle(p, z);
    printf("DESER=%ld\n", deser);

    ADDR a2 = __wasm_export_extension_sketch_agg_serialize_handle(deser);
    ADDR p2 = *(ADDR*) ((char*) a2);
    SIZE z2 = *(SIZE*) ((char*) a2 + 8);
    printf("SERIALIZED PTR=%ld, LENGTH=%ld\n", p2, z2);

    free((void*)p2);

    return 0;
}

int main1(int argc, char* argv[])
{
    HANDLE s1 = __wasm_export_extension_sketch_agg_init_handle();
    printf("S1=%ld\n", s1);

#define THING "\x01\x03\x03\x00\x00\x3A\xCC\x93\x00\x01\xBF\xAB\xF4\x6A\x96\x0D"
    size_t size = sizeof(THING);
    char* ptr = (char*) malloc(size);
    memcpy(ptr, THING, size);

    HANDLE u = __wasm_export_extension_sketch_agg_update_handle(s1, (int64_t) ptr, size);
    printf("U=%ld\n", u);

    ADDR a = __wasm_export_extension_sketch_agg_serialize_handle(u);
    ADDR p = *(ADDR*) ((char*) a);
    ADDR z = *(SIZE*) ((char*) a + 8);
    printf("SERIALIZED PTR=%ld, LENGTH=%ld\n", p, z);
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

