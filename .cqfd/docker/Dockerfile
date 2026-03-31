FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc \
    libc6-dev \
    make \
    bash \
    bats \
    jq \
    shellcheck \
    && rm -rf /var/lib/apt/lists/*
