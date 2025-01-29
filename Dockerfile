FROM rust:slim-bullseye

RUN rm -f /etc/apt/apt.conf.d/docker-clean; echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > /etc/apt/apt.conf.d/keep-cache
RUN apt-get update && \
  export DEBIAN_FRONTEND=noninteractive && \
  apt-get --no-install-recommends install -y wget make libxml2

# wit-bindgen
ARG WIT_BINDGEN_REVISION="60e3c5b41e616fee239304d92128e117dd9be0a7"

RUN cargo install                                                           \
    --git https://github.com/bytecodealliance/wit-bindgen \
    --rev ${WIT_BINDGEN_REVISION} \
    wit-bindgen-cli

# WASI SDK
ARG WASI_VERSION=16
ARG WASI_VERSION_FULL=${WASI_VERSION}.0

RUN wget -q https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${WASI_VERSION}/wasi-sdk_${WASI_VERSION_FULL}_amd64.deb \
  && dpkg -i wasi-sdk_${WASI_VERSION_FULL}_amd64.deb \
  && rm -f wasi-sdk_${WASI_VERSION_FULL}_amd64.deb \
  && echo 'PATH=/opt/wasi-sdk/bin:$PATH' >> /etc/profile

ENV LANG C.UTF-8

ENV PATH /opt/wasi-sdk/bin:$PATH
ENV CC /opt/wasi-sdk/bin/clang
ENV CXX /opt/wasi-sdk/bin/clang++
