#! /bin/sh

set -ue

echo "Running clang-format on all .h and .c files..."

find -name "*.[hc]" | xargs -n1 clang-format -i
