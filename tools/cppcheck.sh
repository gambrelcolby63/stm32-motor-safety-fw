#!/bin/sh
# Static analysis for the firmware sources.
# CMSIS-Core is vendored and not checked. MC_HOST_BUILD keeps the device
# header from pulling core_cm4.h into the analysis.
set -eu
cd "$(dirname "$0")/.."
cppcheck --error-exitcode=1 \
    --enable=warning,performance,portability \
    --inline-suppr \
    --suppress=missingIncludeSystem \
    --suppress=unusedFunction \
    --suppress=unmatchedSuppression \
    --suppress=comparePointers \
    --suppress=memsetClassFloat \
    --suppress=toomanyconfigs \
    -DMC_HOST_BUILD=1 \
    -I include \
    src include/mc
