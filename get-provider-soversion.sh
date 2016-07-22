#!/bin/sh

set -eu

[ -n "${SERIES:-}" ] || SERIES=$(lsb_release -c -s)

case "$SERIES" in
    vivid)
        # Old C++11 ABI, Boost 1.55
        echo 0
        ;;
    xenial)
        # New C++11 ABI, Boost 1.58
        echo 1
        ;;
    yakkety)
        # New C++11 ABI, Boost 1.60
        echo 2
        ;;
    *)
        echo "Unknown distro series $SERIES" >&2
        exit 1
        ;;
esac
