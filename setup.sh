#!/usr/bin/env bash
set -euo pipefail

project_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
tools_dir="$project_dir/.tools/linux"

missing=()
for tool in curl cmake gcc g++ gcov ninja cppcheck python3; do
    if ! command -v "$tool" >/dev/null; then
        missing+=("$tool")
    fi
done
if command -v python3 >/dev/null && ! python3 -c 'import venv, ensurepip' 2>/dev/null; then
    missing+=(python3-venv)
fi
if ((${#missing[@]})); then
    printf 'Required system tools: %s\n' "${missing[*]}"
    if command -v apt-get >/dev/null; then
        install_prefix=()
        if ((EUID != 0)); then
            install_prefix=(sudo)
        fi
        "${install_prefix[@]}" apt-get update
        "${install_prefix[@]}" apt-get install -y \
            ca-certificates curl build-essential cmake ninja-build cppcheck python3-venv
    else
        printf 'Install the listed tools and Python venv with your package manager, then rerun setup.sh.\n' >&2
        exit 1
    fi
fi

mkdir -p "$tools_dir/bin"
go_version=1.26.2
if [[ ! -x "$tools_dir/go/bin/go" ]]; then
    case $(uname -m) in
        x86_64)
            go_arch=amd64
            go_sha256=990e6b4bbba816dc3ee129eaeaf4b42f17c2800b88a2166c265ac1a200262282
            ;;
        aarch64)
            go_arch=arm64
            go_sha256=c958a1fe1b361391db163a485e21f5f228142d6f8b584f6bef89b26f66dc5b23
            ;;
        *)
            printf 'The bundled Go download supports Linux x86_64 and aarch64.\n' >&2
            exit 1
            ;;
    esac
    (
        temp_dir=$(mktemp -d "$tools_dir/go-download.XXXXXX")
        trap 'rm -rf -- "$temp_dir"' EXIT
        archive="$temp_dir/go.tar.gz"
        curl --fail --location --silent --show-error --retry 2 \
            --output "$archive" "https://dl.google.com/go/go${go_version}.linux-${go_arch}.tar.gz"
        printf '%s  %s\n' "$go_sha256" "$archive" | sha256sum --check
        tar -xzf "$archive" -C "$temp_dir"
        mv -T -- "$temp_dir/go" "$tools_dir/go"
    )
fi

export PATH="$tools_dir/go/bin:$PATH"
export GOMODCACHE="$tools_dir/go-mod" GOCACHE="$tools_dir/go-cache" GOTELEMETRY=off
GOBIN="$tools_dir/bin" go install honnef.co/go/tools/cmd/staticcheck@v0.7.0
(cd -- "$project_dir/go" && go mod download)

if [[ ! -x "$tools_dir/venv/bin/python" ]]; then
    python3 -m venv "$tools_dir/venv"
fi
"$tools_dir/venv/bin/python" -m pip install --disable-pip-version-check 'gcovr==8.6'

printf 'Setup complete. Run bash check.sh from the project directory.\n'
