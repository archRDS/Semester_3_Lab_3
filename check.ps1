param(
    [string]$Python,
    [string]$CppWorkspace = (Join-Path $env:TEMP 'Semester2Lab3Cpp')
)

$ErrorActionPreference = 'Stop'
$tools = Join-Path $PSScriptRoot '.tools'
$reports = Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Path $reports -Force | Out-Null
if (!$Python) {
    $pythonPathFile = Join-Path $tools 'python-path.txt'
    $Python = if (Test-Path -LiteralPath $pythonPathFile) {
        (Get-Content -LiteralPath $pythonPathFile -Raw).Trim()
    } else {
        'python'
    }
}

function Invoke-Step([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program exited with code $LASTEXITCODE" }
}

$goBin = Join-Path $tools 'go/bin'
$go = Join-Path $goBin 'go.exe'
$mingw = Join-Path $CppWorkspace 'mingw64'
$cppBin = Join-Path $mingw 'bin'
$env:PATH = "$goBin;$cppBin;$env:PATH"
$env:GOMODCACHE = Join-Path $tools 'go-mod'
$env:GOCACHE = Join-Path $tools 'go-cache'
$env:STATICCHECK_CACHE = Join-Path $tools 'staticcheck-cache'
$env:GOTELEMETRY = 'off'
$env:PYTHONPATH = Join-Path $tools 'python'

if ($CppWorkspace -match '[^\x20-\x7e]') {
    throw 'Choose a CppWorkspace path with ASCII characters so MinGW can resolve its libraries.'
}
New-Item -ItemType Directory -Path $CppWorkspace -Force | Out-Null
if (!(Test-Path -LiteralPath (Join-Path $cppBin 'g++.exe'))) {
    & robocopy.exe (Join-Path $tools 'mingw64') $mingw /E /NFL /NDL /NJH /NJS /NP /R:1 /W:1
    if ($LASTEXITCODE -ge 8) { throw 'Could not copy MinGW to the C++ workspace' }
}

$project = Join-Path $CppWorkspace 'cpp'
$coverageBuild = Join-Path $CppWorkspace 'coverage'
$releaseBuild = Join-Path $CppWorkspace 'release'
$cpp = Join-Path $PSScriptRoot 'cpp'
$gtestSource = Join-Path $tools 'googletest-1.17.0'
Copy-Item -LiteralPath $cpp, $gtestSource -Destination $CppWorkspace -Recurse -Force
$gtest = Join-Path $CppWorkspace 'googletest-1.17.0'
$cmake = Join-Path $cppBin 'cmake.exe'
$compiler = Join-Path $cppBin 'g++.exe'
$ninja = Join-Path $cppBin 'ninja.exe'

Invoke-Step (Join-Path $cppBin 'cppcheck.exe') @(
    '--std=c++17', '--enable=warning,performance,portability', '--error-exitcode=1',
    '--quiet', (Join-Path $project 'src'))
Invoke-Step $cmake @(
    '-S', $project, '-B', $coverageBuild, '-G', 'Ninja',
    "-DCMAKE_CXX_COMPILER=$compiler", "-DCMAKE_MAKE_PROGRAM=$ninja",
    '-DCMAKE_BUILD_TYPE=Debug', '-DLAB3_COVERAGE=ON', "-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=$gtest")
Invoke-Step $cmake @('--build', $coverageBuild, '--parallel', '4')
Get-ChildItem -LiteralPath $coverageBuild -Recurse -File -Filter '*.gcda' |
    ForEach-Object { Remove-Item -LiteralPath $_.FullName }
Invoke-Step (Join-Path $cppBin 'ctest.exe') @('--test-dir', $coverageBuild, '--output-on-failure')
Push-Location $project
try {
    Invoke-Step $Python @(
        '-m', 'gcovr', '--root', '.', '--filter', 'src/',
        '--html-details', (Join-Path $reports 'cpp-coverage.html'),
        '--print-summary', '--fail-under-line', '85', $coverageBuild)
} finally { Pop-Location }

Invoke-Step $cmake @(
    '-S', $project, '-B', $releaseBuild, '-G', 'Ninja',
    "-DCMAKE_CXX_COMPILER=$compiler", "-DCMAKE_MAKE_PROGRAM=$ninja",
    '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_TESTING=OFF')
Invoke-Step $cmake @('--build', $releaseBuild, '--parallel', '4')
Invoke-Step (Join-Path $releaseBuild 'structures_bench.exe') @()

Push-Location (Join-Path $PSScriptRoot 'go')
try {
    Invoke-Step $go @('vet', './...')
    Invoke-Step (Join-Path $tools 'staticcheck/staticcheck.exe') @('./...')
    $profile = Join-Path $reports 'go-coverage.out'
    Invoke-Step $go @('test', '-count=1', "-coverprofile=$profile", './...')
    Invoke-Step $go @('tool', 'cover', "-html=$profile", '-o', (Join-Path $reports 'go-coverage.html'))
    $summary = & $go tool cover "-func=$profile" | Select-Object -Last 1
    if ($LASTEXITCODE -ne 0) { throw 'Could not read Go coverage' }
    if ($summary -notmatch 'total:\s+\(statements\)\s+([\d.]+)%' -or
        [double]::Parse($Matches[1], [Globalization.CultureInfo]::InvariantCulture) -lt 85) {
        throw "Go coverage did not reach 85% ($summary)"
    }
    Invoke-Step $go @('test', '-run=^$', '-bench=.', '-benchmem', '-benchtime=200ms', './...')
} finally { Pop-Location }
