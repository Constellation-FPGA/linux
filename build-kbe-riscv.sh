#!/usr/bin/env sh

make V=1 \
     -C "/lib/modules/$(uname -r)/build" \
     M="$(realpath linux/drivers/firesim/kernel-bypass)" \
     modules
