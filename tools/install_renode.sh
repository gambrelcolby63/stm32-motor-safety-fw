#!/bin/sh
# Install the Renode portable build used by the simulator tests.
set -eu
version=1.17.0
prefix=${1:-/opt/renode}
url="https://github.com/renode/renode/releases/download/v${version}/renode-${version}.linux-portable.tar.gz"
archive=$(mktemp)
curl -fsSL "$url" -o "$archive"
if [ "$(id -u)" -eq 0 ]; then
    sudo=""
else
    sudo="sudo"
fi
$sudo mkdir -p "$prefix"
$sudo tar -xzf "$archive" -C "$prefix" --strip-components=1
rm -f "$archive"
$sudo ln -sfn "$prefix/renode" /usr/local/bin/renode
$sudo ln -sfn "$prefix/renode-test" /usr/local/bin/renode-test
python3 -m pip install --user --break-system-packages \
    'robotframework==6.1' \
    'psutil==5.9.4' \
    'pyyaml==6.0.2' \
    'telnetlib3==2.0.1' \
    'robotframework-retryfailed==0.2.0'
echo "Renode ${version} installed in ${prefix}"
