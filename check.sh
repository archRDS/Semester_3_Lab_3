#!/usr/bin/env bash
set -euo pipefail

project_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
tools_dir="$project_dir/.tools/linux"
build_dir="$project_dir/build/linux"
reports_dir="$build_dir/reports"
coverage_build="$build_dir/cpp-coverage"
release_build="$build_dir/cpp-release"
jobs=${CMAKE_BUILD_PARALLEL_LEVEL:-4}

for tool in "$tools_dir/go/bin/go" "$tools_dir/bin/staticcheck" "$tools_dir/venv/bin/gcovr"; do
    if [[ ! -x "$tool" ]]; then
        printf 'Run bash setup.sh before check.sh. Missing tool: %s\n' "$tool" >&2
        exit 1
    fi
done
export PATH="$tools_dir/go/bin:$PATH"
export GOMODCACHE="$tools_dir/go-mod" GOCACHE="$tools_dir/go-cache" GOTELEMETRY=off
export STATICCHECK_CACHE="$tools_dir/staticcheck-cache"
mkdir -p "$reports_dir"

printf '\nC++ static analysis\n'
cppcheck --std=c++17 --enable=warning,performance,portability --error-exitcode=1 \
    --quiet "$project_dir/cpp/src"

printf '\nC++ unit tests and coverage\n'
cmake -S "$project_dir/cpp" -B "$coverage_build" -G Ninja \
    -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug -DLAB3_COVERAGE=ON
cmake --build "$coverage_build" --parallel "$jobs"
find "$coverage_build" -type f -name '*.gcda' -delete
ctest --test-dir "$coverage_build" --output-on-failure
(
    cd -- "$project_dir/cpp"
    "$tools_dir/venv/bin/gcovr" --root . --filter 'src/' \
        --html-details "$reports_dir/cpp-coverage.html" \
        --print-summary --fail-under-line 85 "$coverage_build"
)

printf '\nC++ benchmarks\n'
cmake -S "$project_dir/cpp" -B "$release_build" -G Ninja \
    -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build "$release_build" --parallel "$jobs"
"$release_build/structures_bench"

(
    cd -- "$project_dir/go"
    printf '\nGo static analysis\n'
    go vet ./...
    "$tools_dir/bin/staticcheck" ./...

    printf '\nGo unit tests and coverage\n'
    profile="$reports_dir/go-coverage.out"
    go test -count=1 "-coverprofile=$profile" ./...
    go tool cover "-html=$profile" -o "$reports_dir/go-coverage.html"
    if ! go tool cover "-func=$profile" | awk '/^total:/ {print; passed = ($NF + 0 >= 85)} END {exit !passed}'; then
        printf 'Go coverage did not reach 85%%.\n' >&2
        exit 1
    fi

    printf '\nGo benchmarks\n'
    go test '-run=^$' '-bench=.' -benchmem -benchtime=200ms ./...
)

printf '\nHTML coverage reports: %s\n' "$reports_dir"
