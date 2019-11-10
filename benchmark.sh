#!/bin/sh

./compile.sh -u
(cd build; python3 base_operation.py)
