#! /usr/bin/env bash

set -ex

LOG_LEVEL="none"
SAMPLE=1
VERTEX=2

make -C build -j

# Release, process all vertexes (bundling version)
LOG_LEVEL=$LOG_LEVEL ./build/main \
  --input ./samples/$SAMPLE \
  --output ./results/$SAMPLE \
  --curvature 13.0 \
  --stable-diff 20 \
  --state-diff 40
