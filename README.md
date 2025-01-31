# HyperLogLog sketches in SingleStoreDB

**Attention**: The code in this repository is intended for experimental use only and is not fully tested, documented, or supported by SingleStore. Visit the [SingleStore Forums](https://www.singlestore.com/forum/) to ask questions about this repository.

## Introduction

[HyperLogLog (HLL) sketches](https://datasketches.apache.org/docs/HLL/HllSketches.html) are a probabilistic data structure for the count-distinct problem, approximating the number of distinct elements in a multiset.

## Contents
This library provides the following User Defined Aggregates (UDAFs) and UDFs (User Defined Functions).  Since SingleStoreDB does not support function name overloading, we shall distinguish between UDAFs and UDFs by suffixing each UDAFs with `_agg`.

For those with familiarity, the interface has been intentionally designed to mimic the HyperLogLog Sketch API of the (datasketches)[https://github.com/apache/datasketches-postgresql] extension for Postgresql.  The notable exception is treatment of `NULL`-valued inputs.  SingleStoreDB does not yet support passing `NULL` values into Wasm, so the empty string (`''`) is treated as such.

Currently, the `lgK` parameter is hardcoded to 12 and the HLL bucket size is always 4.

### `hll_sketch_build_agg` (UDAF)
- **Type**: Aggregate
- **Syntax**: `HLL_SKETCH_BUILD_AGG(BLOB)`
- **Arguments**: The column containing the raw data from which to create an HLL sketch.
- **Return Type**: The HLL sketch representation, as a `BLOB`.
- **Description**: This is a UDAF that will generate an HLL sketch from a column of data and return it as a serialized blob.

### `hll_sketch_union_agg` (UDAF)
- **Type**: Aggregate
- **Syntax**: `HLL_SKETCH_UNION_AGG(SKETCH)`
- **Arguments**: The column containing the raw data from which to create an HLL sketch.
- **Return Type**: The HLL sketch representation, as a `BLOB`.
- **Description**: This is a UDAF that will generate an HLL sketch using the union operation against all rows in a column of data.  The sketch is returned as a serialized blob.

### `hll_sketch_get_estimate` (UDF)
- **Type**: Scalar Function
- **Syntax**: `HLL_SKETCH_GET_ESTIMATE(SKETCH)`
- **Arguments**: A serialized HLL sketch blob that has been generated using one of the above aggregate functions or the scalar function `hll_sketch_union`.
- **Return Type**: The HLL sketch estimate, as a `DOUBLE`.
- **Description**: This is a UDF that takes a single serialized HLL sketch and estimates the number of unique samples in it.

### `hll_sketch_union` (UDF)
- **Type**: Scalar Function
- **Syntax**: `HLL_SKETCH_UNION(SKETCH, SKETCH)`
- **Arguments**: Two serialized HLL sketch blobs that have been generated using one of the above aggregate functions or the scalar function `hll_sketch_union`.
- **Return Type**: A new HLL sketch, as a `BLOB`.
- **Description**: This is a UDF that takes two serialized HLL sketches and combines them using the union operation.  A new HLL sketch blob is returned.

### `hll_sketch_hash` (UDF)
- **Type**: Scalar Function
- **Syntax**: `HLL_SKETCH_HASH(BLOB)`
- **Arguments**: An arbitrary `BLOB` of data from which to generate a hash suitable for HLL sketch indexing.
- **Return Type**: The hash value, as a `BIGINT`.
- **Description**: This UDF will generate a 64-bit hash value from a `BLOB` of data using the HLL internal hashing algorithm.

### `hll_sketch_to_string` (UDF)
- **Type**: Scalar Function
- **Syntax**: `HLL_SKETCH_TO_STRING(BLOB)`
- **Arguments**: A serialized HLL sketch blob that has been generated using one of the above aggregate or scalar functions.
- **Return Type**: A string containing diagnostic information.
- **Description**: This is a UDF that takes a single serialized HLL sketch and returns a string containing diagnostic information about it.

## Deployment to SingleStoreDB
To install these functions using the MySQL client, use the following commands.  They assume you have built the Wasm module and your current directory is the root of this Git repo.  Replace '$DBUSER`, `$DBHOST`, `$DBPORT`, and `$DBNAME` with, respectively, your database username, hostname, port, and the name of the database where you want to deploy the functions.
```bash
mysql -u $DBUSER -h $DBHOST -P $DBPORT -D $DBNAME -p < load_extension.sql
```

Since SingleStoreDB does not yet support passing `NULL`s to Wasm functions, we are providing two deployment options for handling them.  The first way treats empty strings as `NULL`s, with the side effect being that an HLL Sketch hash cannot be generated from an empty string.  The second way hashes empty strings normally, thereby disallowing the concept of a NULL value.  To use the first option, deploy one of the `load_extension...empty_is_null` scripts.  To use the second option, deploy one of the scripts without the `empty_is_null` tag.

### Usage
The following is simple example that creates a table with a column of strings.  It generates an HLL sketch for the `data` column and then computes its estimate.
```sql
CREATE TABLE IF NOT EXISTS sketch_input(data BLOB);
INSERT INTO sketch_input VALUES ("doing"), ("some"), ("hllsketch"), ("stuff");

SELECT hll_sketch_get_estimate(hll_sketch_build_agg(data)) FROM sketch_input;
```

This next example is similar to the above one, except that it shows how to "save" an HLL sketch into a User-Defined Variable so it can be re-used.
```sql
CREATE TABLE IF NOT EXISTS sketch_input(data BLOB);
INSERT INTO sketch_input VALUES ("doing"), ("some"), ("hllsketch"), ("stuff");

SELECT hll_sketch_build_agg(data) FROM sketch_input INTO @sketch1;
SELECT hll_sketch_get_estimate(@sketch1);
```

## Legacy Function Names and Behavior
We've recently renamed the functions in this library to be consistent with those in the DataSketches Postgresql extension.  If your code uses our previous function naming conventions and you don't want to change them, you can install `load_extension_legacynames_emptyisnull.sql` to retain the same naming scheme and behavior (see Deployment to SingleStoreDB, above).

The following table maps the old names to the new names.  Note that the generated HLL sketch is now always returned in a compact representation so the non-compact UDAF variants have been deprecated.

| Type | Legacy Name           | New Name                       |
|------|-----------------------|--------------------------------|
| UDAF | hll_add_agg           | (deprecated)                   |
| UDAF | hll_add_agg_compact   | hll_sketch_build_agg           |
| UDF  | hll_cardinality       | hll_sketch_get_estimate        |
| UDF  | hll_print             | hll_sketch_to_string           |
| UDF  | hll_union             | hll_sketch_union               |
| UDAF | hll_union_agg         | (deprecated)                   |
| UDAF | hll_union_agg_compact | hll_sketch_union_agg           |
| UDF  | n/a                   | hll_sketch_hash                |

## Building
This repository contains all the pre-built artifacts needed to install HLL sketch functions in the database.  However, if you wish to make changes and deploy your code, this Wasm module can be rebuilt using the following commands.

For the easiest path, you can invoke our [Wasm development shell](https://github.com/singlestore-labs/singlestore-wasm-toolkit/blob/main/scripts/dev-shell), which has all of the tools pre-installed.  For example: `dev-shell /my/code/directory`.

If you don't want to use the shell, the build requires you to first install the [Wasm WASI SDK](https://github.com/WebAssembly/wasi-sdk).  The included Makefile assumes that the version of `clang` from the WASI SDK directory is in your path.  You will need to have `$WASI_SDK_PATH/bin` at the beginning of your path.

To rebuild the Wasm module, run this command:
```
make distclean release
```

The binary will be placed in the file `./extension.wasm`.

## Additional Information
To learn about the process of developing a Wasm UDF or TVF in more detail, please have a look at our [tutorial](https://singlestore-labs.github.io/singlestore-wasm-toolkit/html/Tutorial-Overview.html).

The SingleStoreDB Wasm UDF/TVF documentation is [here](https://docs.singlestore.com/managed-service/en/reference/code-engine---powered-by-wasm.html).

## Acknowledgements
This implementation is based on https://github.com/apache/datasketches-postgresql.

## Resources
* [HLL Sketches](https://datasketches.apache.org/docs/HLL/HllSketches.html)
* [Documentation](https://docs.singlestore.com)
* [SingleStore forums](https://www.singlestore.com/forum)

