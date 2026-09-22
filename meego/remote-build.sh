#!/bin/sh
# Builds the N9 package from the phone: syncs this tree to the build machine
# (tools/buildhost.sh of nfsshift-sfos picks LAN or tunnel), restores the cross
# toolchain there if a reboot wiped /tmp, builds, packages, and fetches the
# .deb into ~/ps/rpms/seaprint/.
#
#   meego/remote-build.sh            # build + package
#   meego/remote-build.sh build      # only the ARM binary
#   meego/remote-build.sh tools      # pdf2printable/ippposter for the qemu tests
#   meego/remote-build.sh x86        # only the desktop (Qt Simulator) build
set -e
HERE=$(cd "$(dirname "$0")/.." && pwd)
HOST=$(sh "$HERE/../nfsshift-sfos/tools/buildhost.sh")
REMOTE=/tmp/seaprint/src
TOOLCHAIN_TAR=${TOOLCHAIN_TAR:-$HOME/ps/toolchains/xgcc-harmattan-gcc14-hardfp.tar.gz}
MODE=${1:-package}

echo "== build host: $HOST"
ssh "$HOST" "mkdir -p $REMOTE"
rsync -a --partial --delete --exclude .git --exclude build "$HERE/" "$HOST:$REMOTE/"

if [ "$MODE" != x86 ]; then
    # The toolchain lives in /tmp on the build machine and is kept as a tarball
    # on the phone; a reboot there wipes it.
    if ! ssh "$HOST" test -x /tmp/xgcc-harmattan/bin/arm-none-linux-gnueabi-g++; then
        if [ -f "$TOOLCHAIN_TAR" ]; then
            echo "== restoring the cross toolchain from $TOOLCHAIN_TAR"
            rsync -a --partial "$TOOLCHAIN_TAR" "$HOST:/tmp/xgcc-harmattan.tar.gz"
            ssh "$HOST" "tar xzf /tmp/xgcc-harmattan.tar.gz -C /tmp && rm /tmp/xgcc-harmattan.tar.gz"
        else
            echo "== building the cross toolchain (about half an hour)"
            ssh "$HOST" "sh $REMOTE/meego/toolchain.sh"
            mkdir -p "$(dirname "$TOOLCHAIN_TAR")"
            ssh "$HOST" "tar czf /tmp/xgcc-harmattan.tar.gz -C /tmp xgcc-harmattan"
            rsync -a --partial "$HOST:/tmp/xgcc-harmattan.tar.gz" "$TOOLCHAIN_TAR"
        fi
    fi
fi

case "$MODE" in
build)
    ssh "$HOST" "cd $REMOTE && sh meego/build.sh arm"
    exit 0 ;;
tools)
    ssh "$HOST" "cd $REMOTE && sh meego/build.sh tools"
    exit 0 ;;
x86)
    ssh "$HOST" "cd $REMOTE && sh meego/build.sh x86"
    exit 0 ;;
esac

ssh "$HOST" "cd $REMOTE && sh meego/build.sh arm && sh meego/build-deb.sh"
mkdir -p "$HOME/ps/rpms/seaprint"
rsync -a --partial "$HOST:$REMOTE/build/meego/harbour-seaprint_*_armel.deb" "$HOME/ps/rpms/seaprint/"
ls -la "$HOME/ps/rpms/seaprint/"
