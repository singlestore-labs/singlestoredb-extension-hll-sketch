#!/usr/bin/bash

usage()
{
    cat <<EOF
Usage: $0 [OPTIONS]

Generates Wasm-based UDAs and UDFs for Theta Sketches in SingleStoreDB.

OPTIONS:
    --legacy-names   Use the legacy naming convention.
    --empty-is-null  Treat empty strings as null values.

EOF
    exit 1
}

# Depending on the setting of LEGACY_NAMES, emits either the provided name or
# the legacy version of it.  Returns success (0) if a remap occurred (or wasn't
# required), otherwise failure (1).
#
map-name()
{
    local BASE_NAME="$1"
    
    if [ $LEGACY_NAMES -eq 0 ] ; then
        echo "$BASE_NAME"
        return 0
    fi

    local MAPPED_NAME=""

    case "$BASE_NAME" in
        theta_sketch_union_agg)
            MAPPED_NAME="theta_sketch_agg"
            ;;
        theta_sketch_union)
            MAPPED_NAME="sketch_union"
            ;;
        theta_sketch_intersection)
            MAPPED_NAME="sketch_intersect"
            ;;
        theta_sketch_a_not_b)
            MAPPED_NAME="sketch_anotb"
            ;;
        theta_sketch_get_estimate)
            MAPPED_NAME="sketch_estimate"
            ;;
        theta_sketch_to_string)
            MAPPED_NAME="sketch_to_string"
            ;;
    esac

    echo "$MAPPED_NAME"

    if [ -n "$MAPPED_NAME" ] ; then
        return 0
    else
        return 1
    fi
}

get-outfile-name()
{
    local BASE_NAME="load_extension"

    if [ $LEGACY_NAMES -eq 1 ] ; then
        BASE_NAME="${BASE_NAME}_legacynames"
    fi

    if [ $EMPTY_IS_NULL -eq 1 ] ; then
        BASE_NAME="${BASE_NAME}_emptyisnull"
    fi

    BASE_NAME="$BASE_NAME.sql"
    echo "$BASE_NAME"
}

get-function-suffix()
{
    local DELIM="$1"
    local SFX=""
    if [ $EMPTY_IS_NULL -eq 1 ] ; then
        SFX="${SFX}${DELIM}emptyisnull"
    fi
    echo "$SFX"
}

emit-aggregates()
{
    local SUFFIX=$(get-function-suffix '_')

    if TARGET_NAME=$(map-name theta_sketch_build_agg) ; then cat <<EOF
CREATE OR REPLACE AGGREGATE $TARGET_NAME(LONGBLOB NOT NULL)
RETURNS LONGBLOB NOT NULL
WITH STATE HANDLE
AS WASM FROM BASE64 '$WASM_B64'
WITH WIT FROM BASE64 '$WIT_B64'
INITIALIZE WITH sketch_handle_init
ITERATE WITH sketch_handle_build_accum${SUFFIX}
MERGE WITH sketch_handle_union_merge
TERMINATE WITH sketch_handle_serialize
SERIALIZE WITH sketch_handle_serialize
DESERIALIZE WITH sketch_handle_deserialize;
EOF
    fi

    if TARGET_NAME=$(map-name theta_sketch_build_by_hash_agg) ; then cat <<EOF
CREATE OR REPLACE AGGREGATE $TARGET_NAME(BIGINT UNSIGNED NOT NULL)
RETURNS LONGBLOB NOT NULL
WITH STATE HANDLE
AS WASM FROM BASE64 '$WASM_B64'
WITH WIT FROM BASE64 '$WIT_B64'
INITIALIZE WITH sketch_handle_init
ITERATE WITH sketch_handle_build_accum_by_hash${SUFFIX}
MERGE WITH sketch_handle_union_merge
TERMINATE WITH sketch_handle_serialize
SERIALIZE WITH sketch_handle_serialize
DESERIALIZE WITH sketch_handle_deserialize;
EOF
    fi

    if TARGET_NAME=$(map-name theta_sketch_union_agg) ; then cat <<EOF
CREATE OR REPLACE AGGREGATE $TARGET_NAME(LONGBLOB NOT NULL)
RETURNS LONGBLOB NOT NULL
WITH STATE HANDLE
AS WASM FROM BASE64 '$WASM_B64'
WITH WIT FROM BASE64 '$WIT_B64'
INITIALIZE WITH sketch_handle_init
ITERATE WITH sketch_handle_union_accum${SUFFIX}
MERGE WITH sketch_handle_union_merge
TERMINATE WITH sketch_handle_serialize
SERIALIZE WITH sketch_handle_serialize
DESERIALIZE WITH sketch_handle_deserialize;
EOF
    fi

    if TARGET_NAME=$(map-name theta_sketch_intersection_agg) ; then cat <<EOF
CREATE OR REPLACE AGGREGATE $TARGET_NAME(LONGBLOB NOT NULL)
RETURNS LONGBLOB NOT NULL
WITH STATE HANDLE
AS WASM FROM BASE64 '$WASM_B64'
WITH WIT FROM BASE64 '$WIT_B64'
INITIALIZE WITH sketch_handle_init
ITERATE WITH sketch_handle_intersection_accum${SUFFIX}
MERGE WITH sketch_handle_intersection_merge
TERMINATE WITH sketch_handle_serialize
SERIALIZE WITH sketch_handle_serialize
DESERIALIZE WITH sketch_handle_deserialize;
EOF
    fi
}

emit-scalars()
{
    local SUFFIX=$(get-function-suffix '-')

    VALUE_NAMES=($(cat ${EXTENSION_NAME}.wit | grep 'func(' | grep -v handle | grep -v emptyisnull | sed -E -e 's/([\w]*):.*/\1/g' | sed 's/-/_/g'))
    VALUE_EXPORTS=($(cat ${EXTENSION_NAME}.wit | grep 'func(' | grep -v handle | grep -v emptyisnull | sed -E -e 's/([\w]*):.*/\1/g'))

    for I in `seq 1 ${#VALUE_NAMES[@]}` ; do
        if TARGET_NAME=$(map-name theta_${VALUE_NAMES[$I-1]}) ; then cat <<EOF
CREATE OR REPLACE FUNCTION ${TARGET_NAME}
AS WASM FROM BASE64 '$WASM_B64'
WITH WIT FROM BASE64 '$WIT_B64'
USING EXPORT '${VALUE_EXPORTS[$I-1]}${SUFFIX}';
EOF
        fi
    done
}

###############################################################################
# MAIN
###############################################################################

LEGACY_NAMES=0
EMPTY_IS_NULL=0

while [ $# -gt 0 ] ; do
    case $1 in
        --legacy-names)
            LEGACY_NAMES=1
            shift
            ;;
        --empty-is-null)
            EMPTY_IS_NULL=1
            shift
            ;;
        *)
            usage
            ;;
    esac
done

OUTFILE=$(get-outfile-name)
rm -f "$OUTFILE"

EXTENSION_NAME=$(find . -iname "*.wit" -exec basename {} .wit ';')
WASM_B64=$(cat ${EXTENSION_NAME}.wasm | base64 -w 0)
WIT_B64=$(cat ${EXTENSION_NAME}.wit | base64 -w 0)

emit-aggregates >> "$OUTFILE"
emit-scalars    >> "$OUTFILE"

