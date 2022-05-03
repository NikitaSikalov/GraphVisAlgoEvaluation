#! /usr/bin/env bash

set -ex

IMAGE="bundling"
LOG_LEVEL="info"
SAMPLE=2
VERTEX=2

make -C build -j

# Release, process particular vertex (bundling version)
#LOG_LEVEL=$LOG_LEVEL ./build/main \
#  --input ./samples/$SAMPLE/$IMAGE.png \
#  --output ./results/$SAMPLE/$IMAGE \
#  --vertex $VERTEX \
#  --curvature 13.0 \
#  --stable-diff 20 \
#  --state-diff 40

# Release, process all vertexes (bundling version)
LOG_LEVEL=$LOG_LEVEL ./build/main \
  --input ./samples/$SAMPLE/$IMAGE.png \
  --output ./results/$SAMPLE/$IMAGE \
  --curvature 13.0 \
  --stable-diff 20 \
  --state-diff 40

#LOG_LEVEL=$LOG_LEVEL ./build/main \
#  --input ./samples/$SAMPLE/$IMAGE.png \
#  --output ./results/$SAMPLE/$IMAGE \
#  --vertex $VERTEX \
#  --curvature 13.0 \
#  --stable-diff 20 \
#  --state-diff 40 \
#  --edges-union union \
#  --dev-dir ./dev \
#  --dump-intermediate