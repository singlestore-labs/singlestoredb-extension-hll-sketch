SHELL := bash

.PHONY: release
release: DBGFLAGS = -O3
release: module loader

.PHONY: debug
debug: DBGFLAGS = -g
debug: module loader

.PHONY: loader
loader: load_extension.sql

.PHONY: module
module: extension.wasm

extension.wasm: extension.cpp
	@if [ -z "$$WASI_SDK_PATH" ] ; then \
		echo "Please set the WASI_SDK_PATH environment variable to the location of the WASI SDK."; \
		exit 1; \
	fi; \
	export CLANGPP="$$WASI_SDK_PATH/bin/clang++"; \
	if [ ! -f "$$CLANGPP" ] ; then \
		echo "No clang++ compiler was found at $$CLANGPP.  Aborting."; \
		exit 1; \
	fi; \
	export CLANGPPTGT=`$$CLANGPP --version | grep Target: | awk '{print $$2;}'`; \
	if [ "$$CLANGPPTGT" != "wasm32-unknown-wasi" ] ; then \
		echo "The clang++ at $$CLANGPP does not support the WASI SDK.  Aborting."; \
		exit 1; \
	fi; \
	$$CLANGPP \
		${DBGFLAGS} \
		-fno-exceptions \
 		--target=wasm32-unknown-wasi \
 		-mexec-model=reactor \
 		-I. \
 		-Icommon \
 		-Ihll-sketch \
 		-o extension.wasm \
 		extension.cpp extension_impl_handle.cpp

extension.cpp: extension.wit
	wit-bindgen c --export extension.wit
	# temporary: https://github.com/bytecodealliance/wit-bindgen/issues/290
	sed "s:canonical_abi_realloc(NULL, 0, 1, ret->len:\(char \*\)canonical_abi_realloc(NULL, 0, 1, ret->len:g" extension.c >extension.cpp
	rm extension.c

load_extension.sql: create_loader.sh
	./create_loader.sh
	./create_loader.sh --legacy-names
	./create_loader.sh --empty-is-null
	./create_loader.sh --legacy-names --empty-is-null

.PHONY: driver
driver:
	clang++ \
        -g \
        -DNOTWASM \
        -I. \
        -Icommon \
        -Ihll-sketch \
        -o driver \
        driver.cpp extension-nowasm.cpp extension_impl_handle.cpp

.PHONY: clean
clean:
	@rm -f extension.wasm load_extension*.sql unload_extension.sql driver

.PHONY: distclean
distclean: clean
	@rm -f extension.c extension.h extension.cpp

