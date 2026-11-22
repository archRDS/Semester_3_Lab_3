param([string]$Python)

$ErrorActionPreference = 'Stop'
$tools = Join-Path $PSScriptRoot '.tools'
New-Item -ItemType Directory -Path $tools -Force | Out-Null
$pythonPathFile = Join-Path $tools 'python-path.txt'
if (!$Python) {
    $Python = if (Test-Path -LiteralPath $pythonPathFile) {
        (Get-Content -LiteralPath $pythonPathFile -Raw).Trim()
    } else {
        'python'
    }
}

function Install-Archive($Url, $Marker, $Sha256 = '') {
    if (Test-Path -LiteralPath (Join-Path $tools $Marker)) { return }
    $archive = Join-Path $tools ($Url.Split('/')[-1])
    Write-Host "Downloading $($Url.Split('/')[-1])"
    & curl.exe --fail --location --silent --show-error --retry 2 --connect-timeout 20 --output $archive $Url
    if ($LASTEXITCODE -ne 0) { throw "Download failed: $Url" }
    if ($Sha256 -and (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash -ne $Sha256) {
        throw "SHA256 mismatch: $archive"
    }
    & tar.exe -xf $archive -C $tools
    if ($LASTEXITCODE -ne 0 -or !(Test-Path -LiteralPath (Join-Path $tools $Marker))) {
        throw "Extraction failed: $archive"
    }
    Remove-Item -LiteralPath $archive
}

Install-Archive 'https://codeload.github.com/google/googletest/zip/refs/tags/v1.17.0' 'googletest-1.17.0/googletest/src/gtest-all.cc'
Install-Archive 'https://dl.google.com/go/go1.26.2.windows-amd64.zip' 'go/bin/go.exe' '98eb3570bade15cb826b0909338df6cc6d2cf590bc39c471142002db3832b708'
Install-Archive 'https://github.com/dominikh/go-tools/releases/download/2026.1/staticcheck_windows_amd64.tar.gz' 'staticcheck/staticcheck.exe' '98755d516530d03058ac59cb58ab7fbb9d487d0bc2e16d16e1f700833381e135'
Install-Archive 'https://github.com/brechtsanders/winlibs_mingw/releases/download/16.2.0posix-14.0.0-ucrt-r1/winlibs-x86_64-posix-seh-gcc-16.2.0-mingw-w64ucrt-14.0.0-r1.zip' 'mingw64/bin/g++.exe' 'c1f52294597c0b73786b2a78eb5d176d89226d2f21875eab75e783a8b1cefcc4'

$env:GOMODCACHE = Join-Path $tools 'go-mod'
$env:GOTELEMETRY = 'off'
Push-Location (Join-Path $PSScriptRoot 'go')
try {
    & (Join-Path $tools 'go/bin/go.exe') mod download all
    if ($LASTEXITCODE -ne 0) { throw 'Go dependency download failed' }
} finally { Pop-Location }

if (!(Test-Path -LiteralPath (Join-Path $tools 'python/gcovr/__main__.py'))) {
    & $Python -m pip install --disable-pip-version-check --target (Join-Path $tools 'python') 'gcovr==8.6'
    if ($LASTEXITCODE -ne 0) { throw 'gcovr installation failed' }
}
(Get-Command $Python -CommandType Application).Source |
    Set-Content -LiteralPath $pythonPathFile -Encoding UTF8
