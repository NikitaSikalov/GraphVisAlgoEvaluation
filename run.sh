#! /usr/bin/env bash

IMAGE="bundling"
LOG_LEVEL="info"

make -C build

LOG_LEVEL=$LOG_LEVEL ./build/main \
  --input ./samples/1/$IMAGE.png \
  --output ./results/1/$IMAGE \
  --vertex 0 \
  --curvature 13.0 \
  --stable-diff 20 \
  --state-diff 25 \
  --edges-union union \
  --dev-dir ./dev
