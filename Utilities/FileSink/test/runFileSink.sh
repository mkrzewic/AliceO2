#!/bin/bash -e
CONFIG=FileSink.json
[[ -e $CONFIG ]] || CONFIG="$O2_ROOT/share/config/$CONFIG"
exec runFileSink --id sink1 --mq-config "$CONFIG"
