#!/bin/bash

parabuild \
    . \
    main \
    -t gemm.cpp.template \
    -w workspaces \
    --data-file parabuild_example/data.json \
    --init-bash-script="" \
    --compile-bash-script="make main -B -j" \
    --run-bash-script="python test/gemm_test.py --disable_auto_make" \
    -j 1 \
    -J="-1" \
    --seperate-template \
    --panic-on-compile-error \
    --format-output \
    --makefile
