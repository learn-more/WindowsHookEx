<#
.SYNOPSIS
    Says, loudly, whether the release binaries came out signed or not.

.DESCRIPTION
    Signing is optional, so "is this build signed?" is a question somebody has to answer later. Answered in four
    places: a table in the build log, an AppVeyor build message, the SIGNING_* build variables the deploy step
    quotes, and optionally a markdown block on disk.

    Read off the files themselves, not inferred from "the signing step ran".

.PARAMETER Path
    Files to report on.

.PARAMETER NotesPath
    Where to write the markdown block. Skipped when not given.

.PARAMETER RequireSigned
    Fail when anything is not validly signed. Set when signing was supposed to happen.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string[]]$Path,

    [string]$NotesPath,
    [switch]$RequireSigned
)

$ErrorActionPreference = 'Stop'

$results = foreach ($file in $Path)
{
    if (-not (Test-Path $file)) { throw "No such file: $file" }

    $sig = Get-AuthenticodeSignature -FilePath $file
    $signer = if ($sig.SignerCertificate) { $sig.SignerCertificate.Subject } else { $null }

    # The common name is what a person recognises; the whole DN is noise in a table.
    $shortSigner = if ($signer -and $signer -match 'CN=(?<cn>[^,]+)') { $Matches.cn.Trim('"') } else { $signer }

    [pscustomobject]@{
        Name    = Split-Path $file -Leaf
        Status  = $sig.Status.ToString()
        Signed  = $sig.Status -eq 'Valid'
        Signer  = $shortSigner
        Stamped = [bool]$sig.TimeStamperCertificate
    }
}

$signedCount = @($results | Where-Object Signed).Count
$total = @($results).Count

$state = if ($signedCount -eq $total) { 'signed' } elseif ($signedCount -eq 0) { 'unsigned' } else { 'partial' }
$noun = "release " + $(if ($total -eq 1) { 'binary' } else { 'binaries' })

$summary = switch ($state)
{
    'signed'   { "SIGNED - valid Authenticode signature on $signedCount of $total $noun" }
    'unsigned' { "NOT SIGNED - no Authenticode signature on any of the $total $noun" }
    'partial'  { "PARTIALLY SIGNED - valid Authenticode signature on only $signedCount of $total $noun" }
}

# --- console ---------------------------------------------------------------------------------------------------

$rule = '=' * 100
Write-Host ''
Write-Host $rule
Write-Host "  AUTHENTICODE: $summary"
Write-Host $rule
foreach ($r in $results)
{
    $mark = if ($r.Signed) { 'SIGNED  ' } else { 'UNSIGNED' }
    $detail = if ($r.Signed) { "$($r.Signer)$(if (-not $r.Stamped) { ' (no timestamp)' })" } else { $r.Status }
    Write-Host ("  {0}  {1,-36}  {2}" -f $mark, $r.Name, $detail)
}
Write-Host $rule
Write-Host ''

# --- notes -------------------------------------------------------------------------------------------------------

# One line: this ends up in a build variable, which appveyor.yml expands into the release description.
$notes = switch ($state)
{
    'signed'
    {
        "**Signed release.** The binaries in the archives below are Authenticode signed by ``$($results[0].Signer)`` and timestamped - right-click, Properties, Digital Signatures to check it yourself."
    }
    'unsigned'
    {
        '**Unsigned release.** The binaries in the archives below carry no Authenticode signature, so Windows SmartScreen will warn about them.'
    }
    'partial'
    {
        "**Partially signed release.** Only $signedCount of $total $noun came out Authenticode signed - check each binary individually."
    }
}

if ($NotesPath)
{
    Set-Content -Path $NotesPath -Value ($notes + "`n") -Encoding UTF8
    Write-Host "Notes block written to $NotesPath"
}

# --- AppVeyor ----------------------------------------------------------------------------------------------------

# The Messages tab survives the log scrolling past.
if ($env:APPVEYOR)
{
    $category = if ($state -eq 'signed') { 'Information' } else { 'Warning' }
    $detail = ($results | ForEach-Object {
        "$($_.Name): $(if ($_.Signed) { "signed by $($_.Signer)$(if (-not $_.Stamped) { ', NOT timestamped' })" } else { "not signed ($($_.Status))" })"
    }) -join '; '

    # A message that fails to post is no reason to fail a release; the throw at the end enforces things.
    try
    {
        Add-AppveyorMessage "Code signing: $summary" -Category $category -Details $detail
    }
    catch
    {
        Write-Host "Could not post the AppVeyor build message: $($_.Exception.Message)"
    }

    # Build variables, not $env:, so the deploy step can expand them as $(SIGNING_NOTES).
    Set-AppveyorBuildVariable -Name 'SIGNING_STATUS' -Value $state
    Set-AppveyorBuildVariable -Name 'SIGNING_SUMMARY' -Value $summary
    Set-AppveyorBuildVariable -Name 'SIGNING_NOTES' -Value $notes
}

if ($RequireSigned -and $state -ne 'signed')
{
    throw "Signing was expected but the result is '$state'. Refusing to publish a release that claims to be signed and is not."
}
