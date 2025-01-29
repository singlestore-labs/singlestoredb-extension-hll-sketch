#ifndef _WASM_LOG_H_
#define _WASM_LOG_H_

#ifndef UNUSED
template <typename... Args> void UNUSED(Args &&...args) {
  (void)(sizeof...(args));
}

#endif

#ifdef DEBUG
#define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define DEBUG_LOG(...) UNUSED(__VA_ARGS__)
#endif

#endif // _WASM_LOG_H_