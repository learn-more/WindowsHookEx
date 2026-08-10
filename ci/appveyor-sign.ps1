<#
.SYNOPSIS
    The signing half of an AppVeyor release build: install SimplySign, log in, sign, report.

.DESCRIPTION
    appveyor.yml has no step-level conditions, so the two decisions live here:

      - Only tag builds sign. A run downloads a 260 MB installer and spends a one-time code; branch builds are
        never published. -Force overrides.
      - Signing is optional. Without the CERTUM_* variables the build still produces artifacts, unsigned, and
        says so.

    Called from after_build, before the archives are zipped.

.PARAMETER Path
    Binaries to sign.

.PARAMETER Description
    signtool /d for files with no FileDescription of their own - HookDll has no version resource.

.PARAMETER Force
    Sign a non-tag build, for testing the flow.

.NOTES
    One call per build, all platforms at once: a second login in the same 30-second TOTP step reuses the code,
    which Certum may refuse. See ci/SIGNING.md.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string[]]$Path,

    [string]$Description = 'SetWindowsHookEx test utility',
    [switch]$Force
)

$ErrorActionPreference = 'Stop'

$scripts = $PSScriptRoot

if (-not $Force -and $env:APPVEYOR_REPO_TAG -ne 'true')
{
    Write-Host 'Not a tag build - skipping code signing. (Pass -Force to sign a branch build anyway.)'
    exit 0
}

# AppVeyor leaves secure variables undecrypted on pull request builds, which looks like "not configured yet" and
# gets the same handling: build, do not sign, say so.
$required = 'CERTUM_OTP_URI', 'CERTUM_USERNAME', 'CERTUM_CERTIFICATE_SHA1'
$missing = @($required | Where-Object { [string]::IsNullOrWhiteSpace([Environment]::GetEnvironmentVariable($_)) })

$signing = $missing.Count -eq 0
if ($signing)
{
    Write-Host 'All Certum credentials present - this build will be signed.'
}
else
{
    Write-Host "Unsigned build: missing $($missing -join ', '). See ci/SIGNING.md."
}

if ($signing)
{
    # Each throws on failure, so there is nothing to check between them.
    & (Join-Path $scripts 'install-simplysign.ps1')
    & (Join-Path $scripts 'configure-simplysign.ps1')
    & (Join-Path $scripts 'connect-simplysign.ps1')

    & (Join-Path $scripts 'sign-files.ps1') -Path $Path -Description $Description
}

# Reads the answer off the files, not off "the signing block ran". -RequireSigned only when signing was meant to
# happen, so a tag build cannot ship an archive that is not what it claims.
& (Join-Path $scripts 'signing-status.ps1') -Path $Path -RequireSigned:$signing
