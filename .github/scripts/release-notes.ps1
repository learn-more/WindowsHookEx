<#
.SYNOPSIS
    Appends a changelog to the release notes, one bullet per commit subject.

.DESCRIPTION
    Subject lines only. A commit body is written for someone reading the history; someone downloading a binary
    wants the one-line version.

    The release is created as a draft, so this is a starting point to edit in the UI rather than the final text.

    Appends, so the signing banner written by certum-signing runs first and stays on top.

.PARAMETER Tag
    The tag being released, e.g. v0.9.0.

.PARAMETER NotesPath
    Markdown file to append to. Created if it does not exist.

.PARAMETER PreviousTag
    Start of the range. Defaults to the previous release tag reachable from $Tag; without one, the whole history.

.PARAMETER ExcludePattern
    Subjects matching this regex are left out, case-insensitively. Defaults to the "ci:" prefix: a change to the
    workflow that built the binary is not a change to the binary. Pass '' to keep everything.

.PARAMETER RepoRoot
    Passed as -C to git, so this works regardless of the current directory. Defaults to this repository.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string]$Tag,

    [Parameter(Mandatory)]
    [string]$NotesPath,

    [string]$PreviousTag,
    [string]$ExcludePattern = '^ci:',
    [string]$RepoRoot = (Split-Path -Parent (Split-Path -Parent $PSScriptRoot))
)

$ErrorActionPreference = 'Stop'

# A final release picks up from the last final release, a prerelease from whatever came last. Otherwise v0.9.0
# released after v0.9.0-rc1 would list only the handful of commits that landed since the rc.
#
# Two --match patterns because this repository changed tag schemes: 0.8.1 and earlier are bare, v0.9.0 onwards is
# v-prefixed. Without the bare pattern the first v-tagged release would find no predecessor and dump the entire
# history into the notes. The --exclude pair mirrors it, so a bare 0.8.1-rc1 is skipped the same way v0.9.0-rc1 is.
if (-not $PreviousTag)
{
    $describeArgs = @('describe', '--tags', '--abbrev=0', '--match', 'v*', '--match', '[0-9]*')
    if ($Tag -notmatch '^v?\d+\.\d+\.\d+-') { $describeArgs += @('--exclude', 'v*-*', '--exclude', '[0-9]*-*') }

    $PreviousTag = (& git -C $RepoRoot @describeArgs "$Tag^" 2>$null) | Select-Object -First 1
}

$range = if ($PreviousTag) { "$PreviousTag..$Tag" } else { $Tag }
Write-Host "Collecting commit subjects for $range"

# %s is the subject: the first line, without the blank line that follows it. --no-merges because "Merge branch 'x'"
# describes the history rather than the release.
$subjects = @(& git -C $RepoRoot log --no-merges --pretty=format:%s $range) |
    Where-Object { -not [string]::IsNullOrWhiteSpace($_) } |
    ForEach-Object { $_.Trim() }

if ($LASTEXITCODE -ne 0) { throw "git log $range failed (exit $LASTEXITCODE). Are the tags fetched?" }

$found = $subjects.Count

# -notmatch is case-insensitive, so "CI:" goes too. Guarded because an empty pattern matches every subject, which
# would drop the lot rather than keep it.
if ($ExcludePattern)
{
    $subjects = @($subjects | Where-Object { $_ -notmatch $ExcludePattern })
    if ($subjects.Count -ne $found)
    {
        Write-Host "Skipped $($found - $subjects.Count) subject(s) matching '$ExcludePattern'"
    }
}

# The release lives in this same repository, so "#123" in a subject autolinks to the issue it meant and is left
# alone. "<" is escaped because a release body renders HTML.
$subjects = $subjects | ForEach-Object { $_ -replace '<', '&lt;' }

$heading = if ($PreviousTag) { "## Changes since $PreviousTag" } else { '## Changes' }

$body = @($heading, '')
if ($subjects.Count)
{
    $body += $subjects | ForEach-Object { "- $_" }
}
else
{
    # Retagging the same commit, or a range that was nothing but excluded subjects. Which one matters to whoever
    # has to decide whether an empty changelog is right, so say which.
    $why = if ($found) { "all $found commit(s) in $range matched '$ExcludePattern'" } else { "no commits in $range" }
    Write-Host "::warning title=Release notes::Empty changelog - $why."
    $body += '_No changes recorded for this release._'
}

$dir = Split-Path -Parent $NotesPath
if ($dir -and -not (Test-Path $dir))
{
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
}

# Rewrite rather than Add-Content: trimming what is already there and putting the blank line back is the only way
# to get exactly one separating the banner from the heading, whatever line endings the banner arrived with.
# Markdown needs that blank line - without it the heading is swallowed into the blockquote above.
$existing = if (Test-Path $NotesPath) { (Get-Content -Path $NotesPath -Raw).TrimEnd() } else { '' }
$separator = if ($existing) { "`n`n" } else { '' }

Set-Content -Path $NotesPath -Value ($existing + $separator + ($body -join "`n") + "`n") -NoNewline -Encoding UTF8

Write-Host "$($subjects.Count) commit subject(s) appended to $NotesPath"
Write-Host ''
Get-Content $NotesPath | ForEach-Object { Write-Host "  $_" }
