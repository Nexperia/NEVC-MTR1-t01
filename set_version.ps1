# set_version.ps1 — Update the firmware version across all project files.
#
# Usage:
#   .\set_version.ps1 -Version 1.2.0
#
# Files updated:
#   main/scpi_config.h   — FIRMWARE_VERSION (single C source of truth)
#   README.md            — Version badge
#   Doxyfile             — PROJECT_NUMBER
#   main/scpi.h          — Example response in documentation comment

param(
    [Parameter(Mandatory=$true, HelpMessage="New version in X.Y.Z format (e.g. 1.2.0)")]
    [ValidatePattern('^\d+\.\d+\.\d+$')]
    [string]$Version
)

$Root = $PSScriptRoot

function Update-FileContent {
    param([string]$FilePath, [string]$Pattern, [string]$Replacement)
    $content = Get-Content $FilePath -Raw
    $updated = [regex]::Replace($content, $Pattern, $Replacement)
    if ($updated -eq $content) {
        Write-Warning "  No match found in: $FilePath"
    } else {
        Set-Content -Path $FilePath -Value $updated -NoNewline
        Write-Host "  Updated: $FilePath"
    }
}

Write-Host "Setting firmware version to $Version ..."

# 1. main/scpi_config.h — FIRMWARE_VERSION definition
Update-FileContent `
    -FilePath (Join-Path $Root "main\scpi_config.h") `
    -Pattern  '(#define FIRMWARE_VERSION\s+")[^"]+(")'`
    -Replacement "`${1}$Version`${2}"

# 2. README.md — shields.io version badge
Update-FileContent `
    -FilePath (Join-Path $Root "README.md") `
    -Pattern  '(Version-)[\d.]+(-blue)' `
    -Replacement "`${1}$Version`${2}"

# 3. Doxyfile — PROJECT_NUMBER
Update-FileContent `
    -FilePath (Join-Path $Root "Doxyfile") `
    -Pattern  '(PROJECT_NUMBER\s*=\s*"NEVB-MTR1-t01-)[\d.]+(")'`
    -Replacement "`${1}$Version`${2}"

# 4. main/scpi.h — example response in Doxygen comment
Update-FileContent `
    -FilePath (Join-Path $Root "main\scpi.h") `
    -Pattern  '(NEVC-MTR1-t01-)[\d.]+' `
    -Replacement "`${1}$Version"

Write-Host "Done."
