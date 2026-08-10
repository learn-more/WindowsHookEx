<#
.SYNOPSIS
    Opens a SimplySign session, so the cloud code signing certificate shows up in Cert:\CurrentUser\My.

.DESCRIPTION
    Logging in wants an account name and a TOTP code. The enrolment QR code is an otpauth:// URI, which a script
    can hold, so the code the phone would have shown is generated here and handed straight to:

        SimplySignDesktop.exe /autologin <user> <otp>

    The usual approach - driving the login dialog with WScript.Shell SendKeys - needs a window in the foreground
    and keystrokes landing in it, which no CI runner owes anybody. The command line has no window in the loop.

    Success is the certificate being in the store, not the process still being alive, so a failed login fails
    here rather than inside signtool.

.PARAMETER OtpUri
    The full otpauth://totp/... URI from SimplySign enrolment. Defaults to $env:CERTUM_OTP_URI.

.PARAMETER UserId
    SimplySign account name. Defaults to $env:CERTUM_USERNAME.

.PARAMETER Thumbprint
    SHA-1 thumbprint of the certificate to wait for. Defaults to $env:CERTUM_CERTIFICATE_SHA1. Without it the
    script settles for any code signing certificate appearing.

.PARAMETER ExePath
    SimplySign Desktop executable. Defaults to $env:CERTUM_EXE_PATH, then the standard install location.

.PARAMETER SelfTest
    Check the code generator against the RFC 6238 test vectors and exit, without touching SimplySign or needing
    an account. The half of this script that can be tested without credentials, tested.

.NOTES
    The TOTP generator follows blinkdisk's connect-simplysign.ps1; the login does not, for the reason above.

    /autologin is undocumented, and read positionally: it must be argv[1], the account argv[2], the code argv[3].
    See ci/SIGNING.md.
#>
[CmdletBinding()]
param(
    [string]$OtpUri = $env:CERTUM_OTP_URI,
    [string]$UserId = $env:CERTUM_USERNAME,
    [string]$Thumbprint = $env:CERTUM_CERTIFICATE_SHA1,
    [string]$ExePath = $env:CERTUM_EXE_PATH,
    [switch]$SelfTest
)

$ErrorActionPreference = 'Stop'

# --- TOTP generator --------------------------------------------------------------------------------------------

# Defined before anything is validated, so -SelfTest can reach it without an account or an install.
if (-not ('WindowsHookEx.Totp' -as [type]))
{
    Add-Type -Language CSharp -TypeDefinition @'
using System;
using System.Security.Cryptography;

namespace WindowsHookEx
{
    public static class Totp
    {
        private const string B32 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

        private static byte[] Base32Decode(string s)
        {
            s = s.TrimEnd('=').ToUpperInvariant();
            byte[] bytes = new byte[s.Length * 5 / 8];

            int bitBuffer = 0, bitsLeft = 0, idx = 0;
            foreach (char c in s)
            {
                int val = B32.IndexOf(c);
                if (val < 0) throw new ArgumentException("Invalid Base32 char: " + c);

                bitBuffer = (bitBuffer << 5) | val;
                bitsLeft += 5;

                if (bitsLeft >= 8)
                {
                    bytes[idx++] = (byte)(bitBuffer >> (bitsLeft - 8));
                    bitsLeft -= 8;
                }
            }
            return bytes;
        }

        private static HMAC GetHmac(string algorithm, byte[] key)
        {
            switch (algorithm.ToUpperInvariant())
            {
                case "SHA1": return new HMACSHA1(key);
                case "SHA256": return new HMACSHA256(key);
                case "SHA512": return new HMACSHA512(key);
                default: throw new ArgumentException("Unsupported algorithm: " + algorithm);
            }
        }

        // Seconds left in the current step, so the caller can avoid a code that expires mid-login.
        public static int SecondsRemaining(int period)
        {
            return period - (int)(DateTimeOffset.UtcNow.ToUnixTimeSeconds() % period);
        }

        public static string Now(string secret, int digits, int period, string algorithm)
        {
            return At(secret, digits, period, algorithm, DateTimeOffset.UtcNow.ToUnixTimeSeconds());
        }

        // Split out from Now so -SelfTest can pin the clock to the RFC 6238 vectors.
        public static string At(string secret, int digits, int period, string algorithm, long unixSeconds)
        {
            byte[] key = Base32Decode(secret);
            long counter = unixSeconds / period;

            byte[] cnt = BitConverter.GetBytes(counter);
            if (BitConverter.IsLittleEndian) Array.Reverse(cnt);

            byte[] hash;
            using (var hmac = GetHmac(algorithm, key))
            {
                hash = hmac.ComputeHash(cnt);
            }

            int offset = hash[hash.Length - 1] & 0x0F;
            int binary =
                ((hash[offset] & 0x7F) << 24) |
                ((hash[offset + 1] & 0xFF) << 16) |
                ((hash[offset + 2] & 0xFF) << 8) |
                (hash[offset + 3] & 0xFF);

            return (binary % (int)Math.Pow(10, digits)).ToString(new string('0', digits));
        }
    }
}
'@
}

# --- Self test -------------------------------------------------------------------------------------------------

# The generator is the part that fails silently - a wrong code just looks like a refused login. RFC 6238
# Appendix B, all three algorithms, no account needed.
if ($SelfTest)
{
    # Pad the tail rather than dropping it: 20 bytes divides into 5-bit groups exactly, 32 and 64 do not, and a
    # secret one byte short fails only the SHA-256 and SHA-512 vectors.
    function ConvertTo-Base32
    {
        param([string]$Ascii)

        $alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ234567'
        $bits = ([Text.Encoding]::ASCII.GetBytes($Ascii) |
            ForEach-Object { [Convert]::ToString($_, 2).PadLeft(8, '0') }) -join ''
        if ($bits.Length % 5) { $bits = $bits.PadRight($bits.Length + (5 - $bits.Length % 5), '0') }

        $out = ''
        for ($i = 0; $i -lt $bits.Length; $i += 5)
        {
            $out += $alphabet[[Convert]::ToInt32($bits.Substring($i, 5), 2)]
        }
        return $out
    }

    $seeds = @{
        SHA1   = ConvertTo-Base32 '12345678901234567890'
        SHA256 = ConvertTo-Base32 '12345678901234567890123456789012'
        SHA512 = ConvertTo-Base32 '1234567890123456789012345678901234567890123456789012345678901234'
    }

    # Time, then the expected 8-digit code for SHA1, SHA256, SHA512.
    $vectors = @(
        @(59,          '94287082', '46119246', '90693936'),
        @(1111111109,  '07081804', '68084774', '25091201'),
        @(1111111111,  '14050471', '67062674', '99943326'),
        @(1234567890,  '89005924', '91819424', '93441116'),
        @(2000000000,  '69279037', '90698825', '38618901'),
        @(20000000000, '65353130', '77737706', '47863826')
    )

    $failed = 0
    $count = 0
    foreach ($row in $vectors)
    {
        $t = [long]$row[0]
        foreach ($case in @(@('SHA1', $row[1]), @('SHA256', $row[2]), @('SHA512', $row[3])))
        {
            $count++
            $actual = [WindowsHookEx.Totp]::At($seeds[$case[0]], 8, 30, $case[0], $t)
            $ok = $actual -eq $case[1]
            if (-not $ok) { $failed++ }
            Write-Host ("  {0}  T={1,-12} {2,-7} expected {3}  got {4}" -f $(if ($ok) { 'PASS' } else { 'FAIL' }), $t, $case[0], $case[1], $actual)
        }
    }

    Write-Host ''
    if ($failed) { throw "$failed of $count RFC 6238 vectors failed - the generated codes are wrong, so no login was ever going to be accepted." }

    Write-Host "All $count RFC 6238 vectors pass."
    exit 0
}

# --- Inputs ----------------------------------------------------------------------------------------------------

if (-not $OtpUri) { throw 'No OTP URI. Set CERTUM_OTP_URI (see ci/SIGNING.md).' }
if (-not $UserId) { throw 'No account name. Set CERTUM_USERNAME (see ci/SIGNING.md).' }

# A value pasted into AppVeyor with a trailing newline would otherwise look like a refused login.
$OtpUri = $OtpUri.Trim()
$UserId = $UserId.Trim()

# Start-Process does not quote what it is handed, so a space in the account would push the code into argv[4].
if ($UserId -match '\s')
{
    throw 'CERTUM_USERNAME contains whitespace. SimplySign takes the account as a single command-line argument, so this cannot be passed through as-is.'
}

if (-not $ExePath)
{
    $ExePath = Join-Path $env:ProgramFiles 'Certum\SimplySign Desktop\SimplySignDesktop.exe'
}
if (-not (Test-Path $ExePath))
{
    throw "SimplySign Desktop not found at $ExePath. Run install-simplysign.ps1 first."
}

# Certum enrols with SHA-256 where most authenticators default to SHA-1, so algorithm, digits and period are read
# from the URI rather than assumed.
$uri = [Uri]$OtpUri
$query = @{}
foreach ($part in $uri.Query.TrimStart('?') -split '&')
{
    $kv = $part -split '=', 2
    if ($kv.Count -eq 2) { $query[$kv[0].ToLowerInvariant()] = [Uri]::UnescapeDataString($kv[1]) }
}

$secret = $query['secret']
if (-not $secret) { throw 'The OTP URI has no secret= parameter. It should be the whole otpauth:// URI from the enrolment QR code, not just the account name.' }
$secret = $secret.Trim()

$digits = if ($query['digits']) { [int]$query['digits'] } else { 6 }
$period = if ($query['period']) { [int]$query['period'] } else { 30 }
$algorithm = if ($query['algorithm']) { $query['algorithm'].ToUpperInvariant() } else { 'SHA256' }

if ($algorithm -notin @('SHA1', 'SHA256', 'SHA512'))
{
    throw "Unsupported TOTP algorithm '$algorithm'. Supported: SHA1, SHA256, SHA512."
}

# --- Login -----------------------------------------------------------------------------------------------------

# /autologin closes an instance it finds running, but a forced stop leaves nothing holding the old session.
$existing = Get-Process -Name 'SimplySignDesktop' -ErrorAction SilentlyContinue
if ($existing)
{
    Write-Host 'Stopping a SimplySign Desktop that is already running...'
    $existing | Stop-Process -Force
    Start-Sleep -Seconds 2
}

# A code about to roll over would be checked against the next step. Waiting beats a failed release.
$remaining = [WindowsHookEx.Totp]::SecondsRemaining($period)
if ($remaining -lt 5)
{
    Write-Host "TOTP step expires in ${remaining}s, waiting for the next one..."
    Start-Sleep -Seconds ($remaining + 1)
}

$otp = [WindowsHookEx.Totp]::Now($secret, $digits, $period, $algorithm)

Write-Host "Starting SimplySign Desktop with /autologin (account plus a $digits-digit $algorithm code)..."
$proc = Start-Process -FilePath $ExePath -ArgumentList '/autologin', $UserId, $otp -PassThru

Write-Host "Process started with id $($proc.Id)"

# --- Wait for the certificate ----------------------------------------------------------------------------------

# X509Store rather than the Cert: drive: the provider caches per process, so a certificate arriving after the
# first poll would stay invisible for the life of this loop. Re-opening the store per poll re-reads it.
function Get-MyCertificates
{
    param([string]$Location = 'CurrentUser')

    $store = [System.Security.Cryptography.X509Certificates.X509Store]::new('My', $Location)
    try
    {
        $store.Open([System.Security.Cryptography.X509Certificates.OpenFlags]::ReadOnly)
        return @($store.Certificates)
    }
    finally
    {
        $store.Close()
    }
}

# Strip everything that is not a hex digit: the certificate dialog copies thumbprints with spaces, and a pasted
# value can carry a newline or a BOM. None of those survive the comparison, and all look like "not found".
$wanted = ($Thumbprint -replace '[^0-9A-Fa-f]', '').ToUpperInvariant()
$CodeSigningOid = '1.3.6.1.5.5.7.3.3'

# The real test: until the certificate is in the store, signing cannot work however healthy the process looks.
Write-Host 'Waiting for the certificate to appear in the CurrentUser\My store...'

$deadline = (Get-Date).AddSeconds(90)
$found = $null
while ((Get-Date) -lt $deadline)
{
    $certs = Get-MyCertificates 'CurrentUser'

    if ($wanted)
    {
        $found = @($certs | Where-Object { $_.Thumbprint -eq $wanted })
    }
    else
    {
        # Match the OID, not the friendly name - that is localised.
        $found = @($certs | Where-Object { $_.EnhancedKeyUsageList.ObjectId -contains $CodeSigningOid })
    }

    if ($found) { break }

    if (-not (Get-Process -Id $proc.Id -ErrorAction SilentlyContinue))
    {
        throw 'SimplySign Desktop exited during login. Usually a rejected account name or an out-of-step TOTP code - check the clock skew and that CERTUM_OTP_URI is the current enrolment.'
    }

    Start-Sleep -Seconds 3
}

if (-not $found)
{
    # The one question worth answering here: nothing arrived (login refused), or something did and
    # CERTUM_CERTIFICATE_SHA1 names a different certificate. Thumbprints are public, so listing them is safe.
    # LocalMachine too - a certificate registered there is invisible to signtool but plain to see in certmgr.
    foreach ($location in 'CurrentUser', 'LocalMachine')
    {
        $present = @(Get-MyCertificates $location)
        Write-Host "Certificates in $location\My at this point: $($present.Count)"
        foreach ($cert in $present)
        {
            Write-Host "  $($cert.Thumbprint)  $($cert.Subject)"
        }
    }

    $what = if ($wanted) { 'the expected certificate' } else { 'a code signing certificate' }
    throw "SimplySign is running but $what never showed up in the user store. Either the login was refused - check CERTUM_USERNAME and the runner's clock - or CERTUM_CERTIFICATE_SHA1 matches none of the certificates listed above."
}

foreach ($cert in @($found))
{
    Write-Host "  $($cert.Thumbprint)  $($cert.Subject)"
    Write-Host "  valid until $($cert.NotAfter.ToString('yyyy-MM-dd'))"
}

Write-Host 'SimplySign session is up; the signing certificate is available.'
