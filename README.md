# Theta sketches in SingleStoreDB

**Attention**: The code in this repository is intended for experimental use only and is not fully tested, documented, or supported by SingleStore. Visit the [SingleStore Forums](https://www.singlestore.com/forum/) to ask questions about this repository.

## Introduction

[Theta sketches](https://datasketches.apache.org/docs/Theta/ThetaSketchFramework.html) are a probabilistic data structure intended to approximate the cardinality of all common set operations,
e.g. union, intersection, ..

## Contents
This library provides the following User Defined Aggregates (UDAFs) and UDFs (User Defined Functions).  Since SingleStoreDB does not support function name overloading, we shall distinguish between UDAFs and UDFs by suffixing each UDAFs with `_agg`.

For those with familiarity, the interface has been intentionally designed to mimic the Theta Sketch API of the (datasketches)[https://github.com/apache/datasketches-postgresql] extension for Postgresql.  The notable exception is treatment of `NULL`-valued inputs.  SingleStoreDB does not yet support passing `NULL` values into Wasm, so the empty string (`''`) is treated as such.  This, of course, then precludes the generation of hashes from empty strings.

### `theta_sketch_build_agg` (UDAF)
- **Type**: Aggregate
- **Syntax**: `THETA_SKETCH_BUILD_AGG(BLOB)`
- **Arguments**: The column containing the raw data from which to create a theta sketch.
- **Return Type**: The theta sketch representation, as a `BLOB`.
- **Description**: This is a UDAF that will generate a theta sketch from a column of data and return it as a serialized blob.

### `theta_sketch_build_by_hash_agg` (UDAF)
- **Type**: Aggregate
- **Syntax**: `THETA_SKETCH_BUILD_RAW_AGG(BIGINT)`
- **Arguments**: The column containing pre-computed hashes (as `BIGINT`s) from which to create a theta sketch.  This column should *not* contain the blobs themselves.
- **Return Type**: The theta sketch representation, as a `BLOB`.
- **Description**: This is a UDAF that will generate a theta sketch from a column of hashes and return it as a serialized blob.  If it makes sense for your use case, this function can be used in conjunction with the `theta_sketch_hash` build a theta sketch from precomputed hashes.  Doing this allows the theta sketch generation to be decoupled from the hashing algorithm, providing significantly improved performance.  For example, you might generate a theta sketch hash each time a row is inserted and run this aggregate on the hash column instead of the raw data.

### `theta_sketch_intersection_agg` (UDAF)
- **Type**: Aggregate
- **Syntax**: `THETA_SKETCH_INTERSECTION_AGG(SKETCH)`
- **Arguments**: The column containing the raw data from which to create a theta sketch.
- **Return Type**: The theta sketch representation, as a `BLOB`.
- **Description**: This is a UDAF that will generate a theta sketch using the intersection operation against all rows in a column of data.  The sketch is returned as a serialized blob.

### `theta_sketch_union_agg` (UDAF)
- **Type**: Aggregate
- **Syntax**: `THETA_SKETCH_UNION_AGG(SKETCH)`
- **Arguments**: The column containing the raw data from which to create a theta sketch.
- **Return Type**: The theta sketch representation, as a `BLOB`.
- **Description**: This is a UDAF that will generate a theta sketch using the union operation against all rows in a column of data.  The sketch is returned as a serialized blob.

### `theta_sketch_get_estimate` (UDF)
- **Type**: Scalar Function
- **Syntax**: `THETA_SKETCH_GET_ESTIMATE(SKETCH)`
- **Arguments**: A serialized theta sketch blob that has been generated using one of the above aggregate functions or one of the scalar functions `theta_sketch_union`, `theta_sketch_intersection`, or `theta_sketch_a_not_b`.
- **Return Type**: The theta sketch estimate, as a `DOUBLE`.
- **Description**: This is a UDF that takes a single serialized theta sketch and estimates the number of unique samples in it.

### `theta_sketch_union` (UDF)
- **Type**: Scalar Function
- **Syntax**: `THETA_SKETCH_UNION(SKETCH, SKETCH)`
- **Arguments**: Two serialized theta sketch blobs that have been generated using one of the above aggregate functions or one of the scalar functions `theta_sketch_union`, `theta_sketch_intersection`, or `theta_sketch_a_not_b`.
- **Return Type**: A new theta sketch, as a `BLOB`.
- **Description**: This is a UDF that takes two serialized theta sketches and combines them using the union operation.  A new theta sketch blob is returned.

### `theta_sketch_intersection` (UDF)
- **Type**: Scalar Function
- **Syntax**: `THETA_SKETCH_INTERSECTION(SKETCH, SKETCH)`
- **Arguments**: Two serialized theta sketch blobs that have been generated using one of the above aggregate functions or one of the scalar functions `theta_sketch_union`, `theta_sketch_intersection`, or `theta_sketch_a_not_b`.
- **Return Type**: A new theta sketch, as a `BLOB`.
- **Description**: This is a UDF that takes two serialized theta sketches and combines them using the intersection operation.  A new theta sketch blob is returned.

### `theta_sketch_a_not_b` (UDF)
- **Type**: Scalar Function
- **Syntax**: `THETA_SKETCH_A_NOT_B(SKETCH, SKETCH)`
- **Arguments**: Two serialized theta sketch blobs that have been generated using one of the above aggregate functions or one of the scalar functions `theta_sketch_union`, `theta_sketch_intersection`, or `theta_sketch_a_not_b`.
- **Return Type**: A new theta sketch, as a `BLOB`.
- **Description**: This is a UDF that takes two serialized theta sketches and combines them using the set difference operation.  A new theta sketch blob is returned.

### `theta_sketch_hash` (UDF)
- **Type**: Scalar Function
- **Syntax**: `THETA_SKETCH_HASH(BLOB)`
- **Arguments**: An arbitrary `BLOB` of data from which to generate a hash suitable for theta sketch indexing.
- **Return Type**: The hash value, as a `BIGINT`.
- **Description**: This is UDF is intended to be used in conjunction with the UDAF `theta_sketch_build_by_hash_agg`.  It will generate a 64-bit hash value from a `BLOB` of data that can then be stored directly in a theta sketch.  See `theta_sketch_build_by_hash_agg` for more information.

### `theta_sketch_to_string` (UDF)
- **Type**: Scalar Function
- **Syntax**: `SKETCH_TO_STRING(BLOB)`
- **Arguments**: A serialized theta sketch blob that has been generated using one of the above aggregate functions.
- **Return Type**: A string containing diagnostic information.
- **Description**: This is a UDF that takes a single serialized theta sketch and returns a string containing diagnostic information about it.

## Building
The Wasm module can be built using the following commands.  The build requires you to first install the [Wasm WASI SDK](https://github.com/WebAssembly/wasi-sdk).  The included Makefile assumes that the version of `clang` from the WASI SDK directory is in your path.  Replace the variable `$WASI_SDK_PATH` below with this directory.
```bash
# Make sure the WASI SDK binaries are in your PATH.
export PATH=$WASI_SDK_PATH/bin:$PATH

# Compile the Wasm module.
make release
```
The binary will be placed in the file `./extension.wasm`.

## Deployment to SingleStoreDB
To install these functions using the MySQL client, use the following commands.  They assume you have built the Wasm module and your current directory is the root of this Git repo.  Replace '$DBUSER`, `$DBHOST`, `$DBPORT`, and `$DBNAME` with, respectively, your database username, hostname, port, and the name of the database where you want to deploy the functions.
```bash
mysql -u $DBUSER -h $DBHOST -P $DBPORT -D $DBNAME -p < load_extension.sql
```

To install the legacy version of this library, substitute `load_extension_legacynames_emptyisnull.sql` for `load_extension`, above.

### Usage
The following is simple example that creates a table with two columns of integers.  It generates a theta sketch for the `data` column and then computes its estimate.
```sql
CREATE TABLE IF NOT EXISTS sketch_input(data BLOB);
INSERT INTO sketch_input VALUES ("doing"), ("some"), ("thetasketch"), ("stuff");

SELECT theta_sketch_get_estimate(theta_sketch_build_agg(data)) FROM sketch_input;
```

This next example is similar to the above one, except that it shows how to "save" the theta sketche in a User-Defined Variables so it can be re-used.
```sql
CREATE TABLE IF NOT EXISTS sketch_input(data BLOB);
INSERT INTO sketch_input VALUES ("doing"), ("some"), ("thetasketch"), ("stuff");

SELECT theta_sketch_build_agg(data) FROM sketch_input INTO @sketch1;
SELECT theta_sketch_get_estimate(@sketch1);
```

This example shows how to pre-compute hashes and then later generate a theta sketch from them.
```sql
CREATE TABLE IF NOT EXISTS sketch_input(data BIGINT);
CREATE TABLE IF NOT EXISTS sketch_hashes(hash BIGINT);
INSERT INTO sketch_input(data) VALUES ("doing"), ("some"), ("thetasketch"), ("stuff");
INSERT INTO sketch_hashes(hash) SELECT theta_sketch_hash(data) hash FROM sketch_input;

SELECT theta_sketch_get_estimate(theta_sketch_build_by_hash_agg(hash)) FROM sketch_hashes;
```

## Legacy Function Names and Behavior
We've recently renamed the functions in this library to be consistent with those in the DataSketches Postgresql extension.  If your code uses our previous function naming conventions and you don't want to change them, you can install `load_extension_legacynames_emptyisnull.sql` to retain the same naming scheme and behavior (see Deployment to SingleStoreDB, above).

The following table maps the old names to the new names.  Every old function has a new equivalent, but not vice-versa.

| Type | Legacy Name      | New Name                       |
|------|------------------|--------------------------------|
| UDF  | sketch_anotb     | theta_sketch_a_not_b           |
| UDAF | n/a              | theta_sketch_build_agg         |
| UDAF | n/a              | theta_sketch_build_by_hash_agg |
| UDF  | sketch_estimate  | theta_sketch_get_estimate      |
| UDF  | n/a              | theta_sketch_hash              |
| UDF  | sketch_intersect | theta_sketch_intersection      |
| UDAF | n/a              | theta_sketch_intersection_agg  |
| UDF  | sketch_to_string | theta_sketch_to_string         |
| UDF  | sketch_union     | theta_sketch_union             |
| UDAF | theta_sketch_agg | theta_sketch_union_agg         |

Additionally, there has been a behavior change.  Since SingleStoreDB does not yet support passing `NULL`s to Wasm functions, the legacy behavior treats empty strings as `NULL`s.  This, of course, precludes the generation of a Theta Sketch hash from an actual empty string.  In the new version, the semantics are different:  empty strings are hashed normally, and there is no support for NULL values (yet).

## Additional Information
To learn about the process of developing a Wasm UDF or TVF in more detail, please have a look at our [tutorial](https://singlestore-labs.github.io/singlestore-wasm-toolkit/html/Tutorial-Overview.html).

The SingleStoreDB Wasm UDF/TVF documentation is [here](https://docs.singlestore.com/managed-service/en/reference/code-engine---powered-by-wasm.html).

## Acknowledgements
This implementation is based on https://github.com/apache/datasketches-postgresql.

## Resources
* [Theta Sketches](https://datasketches.apache.org/docs/Theta/ThetaSketchFramework.html)
* [Documentation](https://docs.singlestore.com)
* [Twitter](https://twitter.com/SingleStoreDevs)
* [SingleStore forums](https://www.singlestore.com/forum)
* [SingleStoreDB extension template for Rust](https://github.com/singlestore-labs/singlestoredb-extension-rust-template)

