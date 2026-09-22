#!/usr/bin/env bash

set -e

for dir in "recipes"/*/; do
    conan export "$dir" "$@"
done
