<#
.SYNOPSIS
    Authenticode-signs the release binaries with the Certum cloud certificate.

.DESCRIPTION
    Runs after connect-simplysign.ps1 has put the certificate in Cert:\CurrentUser\My.

    Timestamped, so the binaries stay valid after the certificate expires. Verified before returning: signtool
    exiting 0 and the file carrying a chain that validates are not the same claim.

.PARAMETER Path
    Files to sign.

.PARAMETER Thumbprint
    SHA-1 thumbprint of the signing certificate. Defaults to $env:CERTUM_CERTIFICATE_SHA1.

.PARAMETER TimestampServer
    RFC 3161 timestamp server. Defaults to $env:CERTUM_TIMESTAMP_SERVER, then Certum's.

.PARAMETER IntermediateUrl
    Where to fetch the intermediate certificate signtool embeds in the chain. Defaults to
    $env:CERTUM_INTERMEDIATE_URL, then Certum's CCSCA 2021 intermediate.

.PARAMETER Description
    signtool /d - the name Windows shows in the UAC and SmartScreen dialogs. Only used for files with no
    FileDescription of their own; a file that has one keeps it.

.PARAMETER DescriptionUrl
    signtool /du - where "more information" in those dialogs points.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string[]]$Path,

    [string]$Thumbprint = $env:CERTUM_CERTIFICATE_SHA1,
    [string]$TimestampServer = $env:CERTUM_TIMESTAMP_SERVER,
    [string]$IntermediateUrl = $env:CERTUM_INTERMEDIATE_URL,
    [string]$Description,
    [string]$DescriptionUrl = 'https://learn-more.github.io/WindowsHookEx/'
)

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

if (-not $Thumbprint) { throw 'No certificate thumbprint. Set CERTUM_CERTIFICATE_SHA1 (see ci/SIGNING.md).' }
if (-not $TimestampServer) { $TimestampServer = 'http://time.certum.pl' }
if (-not $IntermediateUrl) { $IntermediateUrl = 'https://repository.certum.pl/ccsca2021.cer' }

# Spaces from the certificate dialog, a newline or BOM from a pasted value: none survive the comparison, and all
# look like "certificate not found".
$Thumbprint = ($Thumbprint -replace '[^0-9A-Fa-f]', '').ToUpperInvariant()

# signtool comes from the Windows SDK, which is not on PATH in an AppVeyor build. Newest SDK, x64 build.
function Get-SignTool
{
    $onPath = Get-Command 'signtool.exe' -ErrorAction SilentlyContinue
    if ($onPath) { return $onPath.Source }

    $roots = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\bin'),
        (Join-Path $env:ProgramFiles 'Windows Kits\10\bin')
    ) | Where-Object { $_ -and (Test-Path $_) }

    $candidates = foreach ($root in $roots)
    {
        Get-ChildItem -Path $root -Filter 'signtool.exe' -Recurse -ErrorAction SilentlyContinue |
            Where-Object { $_.Directory.Name -eq 'x64' }
    }

    # The SDK version is the directory above x64 ("10.0.22621.0"). Older SDKs use bare bin\x64, which parses as
    # no version and sorts last - which is what we want.
    $best = $candidates | Sort-Object -Descending -Property @{ Expression = {
        $v = $null
        if ([version]::TryParse($_.Directory.Parent.Name, [ref]$v)) { $v } else { [version]'0.0' }
    } } | Select-Object -First 1

    if (-not $best) { throw 'signtool.exe not found. Is the Windows SDK installed on this runner?' }
    return $best.FullName
}

$signTool = Get-SignTool
Write-Host "signtool: $signTool"

# X509Store rather than the Cert: drive: the provider caches, and this runs right after a certificate arrived.
$store = [System.Security.Cryptography.X509Certificates.X509Store]::new('My', 'CurrentUser')
try
{
    $store.Open([System.Security.Cryptography.X509Certificates.OpenFlags]::ReadOnly)
    $cert = @($store.Certificates | Where-Object { $_.Thumbprint -eq $Thumbprint })
}
finally
{
    $store.Close()
}
if (-not $cert)
{
    throw "Certificate $Thumbprint is not in CurrentUser\My. Run connect-simplysign.ps1 first, and check CERTUM_CERTIFICATE_SHA1 against the certificate that account holds."
}
Write-Host "certificate: $($cert[0].Subject)"
Write-Host "timestamp:   $TimestampServer"

# /ac wants the intermediate as a file. Fetched per run rather than checked in, so the chain stays current if
# Certum re-issues. It is a public certificate.
$intermediate = Join-Path $env:TEMP 'certum-intermediate.cer'
Write-Host "Fetching intermediate certificate from $IntermediateUrl"
Invoke-WebRequest -Uri $IntermediateUrl -OutFile $intermediate -UseBasicParsing -TimeoutSec 120

$signArgs = @(
    'sign',
    '/sha1', $Thumbprint,
    '/fd', 'sha256',        # file digest
    '/td', 'sha256',        # timestamp digest
    '/tr', $TimestampServer,
    '/ac', $intermediate,
    '/v'
)

if ($DescriptionUrl) { $signArgs += @('/du', $DescriptionUrl) }

# Left off, /d falls back to the file name. Preferring the binary's own FileDescription keeps that string in the
# version resource: WindowsHookEx.exe describes itself from WindowsHookEx.rc2, and only HookDll - which has no
# version resource - falls back to what the caller passed.
function Get-SignDescription
{
    param([string]$File)

    $fileDescription = (Get-Item $File).VersionInfo.FileDescription
    if (-not [string]::IsNullOrWhiteSpace($fileDescription)) { return $fileDescription.Trim() }
    return $Description
}

foreach ($file in $Path)
{
    if (-not (Test-Path $file)) { throw "Nothing to sign at $file" }

    $fileArgs = $signArgs
    $description = Get-SignDescription $file
    if ($description) { $fileArgs += @('/d', $description) }

    $shownAs = if ($description) { " as '$description'" } else { '' }

    Write-Host ''
    Write-Host "Signing $file$shownAs"

    # Timestamp servers rate-limit; a release should not fall over for one refused request.
    for ($attempt = 1; $attempt -le 3; $attempt++)
    {
        & $signTool @fileArgs $file
        if ($LASTEXITCODE -eq 0) { break }

        if ($attempt -eq 3) { throw "signtool failed on $file after $attempt attempts (exit $LASTEXITCODE)" }
        Write-Host "  signtool exit $LASTEXITCODE, retrying in 15s..."
        Start-Sleep -Seconds 15
    }

    & $signTool verify /pa /v $file
    if ($LASTEXITCODE -ne 0) { throw "$file was signed but the signature does not verify (signtool verify exit $LASTEXITCODE)" }
}

Write-Host ''
Write-Host "Signed and verified $($Path.Count) file(s)."
