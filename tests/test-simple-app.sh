#!/bin/bash

# Test script to run simple-app with im-bridge IM module

set -x

ROOT_DIR="$(dirname "$(readlink -f "$0")")"/..
# Export necessary paths for the IM module to be found
export GTK_IM_MODULE=im-bridge
export LD_LIBRARY_PATH="${ROOT_DIR}/builddir/gtk3:${LD_LIBRARY_PATH}"
export GTK_IM_MODULE_PATH="${ROOT_DIR}/builddir/gtk3"

# Run the simple app with timeout
timeout 5 ${ROOT_DIR}/builddir/gtk3/simple-app-gtk3/simple-app-gtk3 2>&1

echo "Exit code: $?"
