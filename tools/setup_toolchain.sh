#!/bin/bash

set -eu

declare -r OBGGCC_TOOLCHAIN='/tmp/obggcc-toolchain'

if [ -d "${OBGGCC_TOOLCHAIN}" ]; then
	PATH+=":${OBGGCC_TOOLCHAIN}/bin"
	export OBGGCC_TOOLCHAIN \
		PATH
	return 0
fi

declare -r OBGGCC_CROSS_TAG="$(jq --raw-output '.tag_name' <<< "$(curl --retry 10 --retry-delay 3 --silent --url 'https://api.github.com/repos/AmanoTeam/obggcc/releases/latest')")"
declare -r OBGGCC_CROSS_TARBALL='/tmp/obggcc.tar.xz'
declare -r OBGGCC_CROSS_URL="https://github.com/AmanoTeam/obggcc/releases/download/${OBGGCC_CROSS_TAG}/x86_64-unknown-linux-gnu.tar.xz"

curl --connect-timeout '10' --retry '15' --retry-all-errors --fail --silent --location --url "${OBGGCC_CROSS_URL}" --output "${OBGGCC_CROSS_TARBALL}"
tar --directory="$(dirname "${OBGGCC_CROSS_TARBALL}")" --extract --file="${OBGGCC_CROSS_TARBALL}"

rm "${OBGGCC_CROSS_TARBALL}"

mv '/tmp/obggcc' "${OBGGCC_TOOLCHAIN}"

PATH+=":${OBGGCC_TOOLCHAIN}/bin"

export OBGGCC_TOOLCHAIN \
	PATH
