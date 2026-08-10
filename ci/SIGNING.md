# Code signing

Release binaries are Authenticode signed with a Certum code signing certificate. The certificate is not a file:
it lives in Certum's cloud HSM, reached through SimplySign Desktop, which emulates a local smart card reader and
registers the certificate into `Cert:\CurrentUser\My` once a session is open. `signtool` then uses it by
thumbprint like any other certificate.

**Signing is optional.** Without the variables below a build still produces its artifacts - unsigned, and saying
so. Nothing has to be configured to cut a release.

**Only tag builds sign.** A run downloads a 260 MB installer and spends a one-time code, and branch builds are
never published. The gate is `APPVEYOR_REPO_TAG` in [`appveyor-sign.ps1`](appveyor-sign.ps1); `-Force` overrides
it for testing.

Signing happens in `after_build`, **before** the `7z` lines. What a user runs is whatever came out of the zip.

## What to store

**Settings → Environment → Environment variables** on the AppVeyor project, padlocked. Project settings rather
than `secure:` blocks in `appveyor.yml`: `CERTUM_OTP_URI` is a full second factor and has no business being in
the repository.

All three are required; missing any one turns signing off.

| Variable | What it is |
| --- | --- |
| `CERTUM_USERNAME` | SimplySign account name - the e-mail address used to log in. |
| `CERTUM_OTP_URI` | The **whole** `otpauth://totp/...` URI from enrolment, including `?secret=...`. Not just the secret, and not the six digits the app is showing. |
| `CERTUM_CERTIFICATE_SHA1` | SHA-1 thumbprint of the certificate, 40 hex characters. Spaces stripped, case ignored. |

Two more are optional, so a changed URL needs no code change. Plain variables are fine - both are public
endpoints.

| Variable | Default |
| --- | --- |
| `CERTUM_TIMESTAMP_SERVER` | `http://time.certum.pl` |
| `CERTUM_INTERMEDIATE_URL` | `https://repository.certum.pl/ccsca2021.cer` |

AppVeyor does not decrypt secure variables on pull request builds. That looks like "not configured yet" from
inside the build and is handled the same way: it runs, unsigned.

`CERTUM_OTP_URI` is a full second factor, not a hint at one - anyone holding it can generate valid codes
indefinitely. Treat it like the account password, and re-issue the token if it is ever exposed.

### What the user sees in the prompt

`signtool /d` sets the program name in the UAC and SmartScreen dialogs, `/du` the "more information" link.
`/d` comes from each binary's own `FileDescription` where it has one, so `WindowsHookEx.exe` is described by
[`src/WindowsHookEx.rc2`](../src/WindowsHookEx.rc2) and the string stays in one place. `HookDll` has no version
resource, so it takes the `-Description` `appveyor.yml` passes. `/du` defaults to the project page in
`sign-files.ps1`.

## Where to get the values

### `CERTUM_OTP_URI`

The enrolment QR code *is* the URI - the phone app just scans it. Capture the text instead:

- during enrolment, most QR readers show the raw contents rather than acting on them
- already enrolled? Re-issue the token in the Certum panel and capture the new QR. The old one stops working, so
  re-enrol the phone from the same code if it is still wanted there.

All of it goes in the variable:

```
otpauth://totp/SimplySign:you@example.com?secret=BASE32SECRET&issuer=Certum&algorithm=SHA256&digits=6&period=30
```

`algorithm`, `digits` and `period` are read from the URI rather than assumed - Certum enrols with SHA-256 where
most authenticators default to SHA-1, and a code generated with the wrong one is simply wrong.

### `CERTUM_CERTIFICATE_SHA1`

Install SimplySign Desktop locally, connect it, then ask the store what arrived:

```powershell
Get-ChildItem Cert:\CurrentUser\My -CodeSigningCert | Format-List Subject, Thumbprint, NotAfter
```

It changes when the certificate is renewed, so *"Certificate ... is not in CurrentUser\My"* after a renewal
usually means this variable, not the login.

## Where the status shows up

Signed or not, every tag build says which it was:

- a **table in the build log**, one row per binary, with signer and timestamp
- an **AppVeyor build message** on the Messages tab
- the **draft release description**, via `$(SIGNING_NOTES)` in `appveyor.yml`
- `SIGNING_STATUS` and `SIGNING_SUMMARY` as build variables

Read back off the finished files with `Get-AuthenticodeSignature`, not inferred from "the signing step ran". When
signing was configured but a binary came out unsigned, the build fails rather than publishing a release that
claims to be signed.

## The scripts

| Script | Does |
| --- | --- |
| [`appveyor-sign.ps1`](appveyor-sign.ps1) | Decides whether this build signs, then runs the rest in order. The only entry point `appveyor.yml` calls. |
| [`install-simplysign.ps1`](install-simplysign.ps1) | Installs SimplySign Desktop, pinned by version and SHA-256. |
| [`configure-simplysign.ps1`](configure-simplysign.ps1) | Registry settings for unattended use. |
| [`connect-simplysign.ps1`](connect-simplysign.ps1) | Generates the TOTP code, logs in, waits for the certificate. `-SelfTest` checks the generator against RFC 6238 and exits. |
| [`sign-files.ps1`](sign-files.ps1) | `signtool sign` with SHA-256 and an RFC 3161 timestamp, then verifies. |
| [`signing-status.ps1`](signing-status.ps1) | Reports the outcome everywhere listed above. |

Adapted from [depcheck](https://github.com/learn-more/depcheck/tree/main/.github), which runs the same flow on
GitHub Actions, and through it from [blinkdisk](https://github.com/blinkdisk/blinkdisk/tree/main/.github). The
login does not follow blinkdisk: it drives the login dialog with `WScript.Shell` SendKeys, which needs a window
in the foreground and keystrokes landing in it. On a CI runner that produced a login that never completed.
SimplySign Desktop takes the credentials directly instead:

```
SimplySignDesktop.exe /autologin <account> <otp>
```

Undocumented, and read positionally - `/autologin` must be `argv[1]`, account `argv[2]`, code `argv[3]`. The
launcher does not quote the account, so whitespace in `CERTUM_USERNAME` would push the code into `argv[4]`; the
script rejects that up front. Values are trimmed for the same reason.

## One login per build

A login spends a one-time code, so there must be exactly one per build. Two inside the same 30-second TOTP step
present the *same* code twice, which Certum may refuse - surfacing as a release that fails to sign for no visible
reason, on some builds and not others.

Hence `appveyor.yml` builds x86 and x64 in **one job** with an explicit `build_script:`, rather than the
`platform: [x86, x64]` matrix it used to be. Sequential jobs would usually land in different steps, but only by
accident of timing, and raising the concurrent job limit would quietly start breaking releases.

Restoring the matrix means solving the login first - serialising it, or signing in a single job the others feed.
The cost of one job: a compile error no longer names its platform without reading the log, and the platforms no
longer build in parallel.

## Testing it locally

Most of this needs a Certum account. The part that fails *silently* does not - a wrong code is indistinguishable
from a refused login, so the generator is checked against the RFC 6238 vectors:

```bash
pwsh -File ci/connect-simplysign.ps1 -SelfTest
```

No account, no install, no certificate store.

It does not cover the login itself. Testing that means a real account and a real code, so it belongs on a machine
that already has SimplySign connected - and note that `connect-simplysign.ps1` stops any running SimplySign
Desktop first, ending whatever session was open. Repeated failed attempts are worth avoiding on a live account.

Whether a given SimplySign build has the flag can be checked without running it:

```powershell
$exe = Join-Path $env:ProgramFiles 'Certum\SimplySign Desktop\SimplySignDesktop.exe'
$s = [Text.Encoding]::Unicode.GetString([IO.File]::ReadAllBytes($exe))
$s.Contains('/autologin')
```

Bumping the pinned SimplySign version means changing the version *and* the hash in `install-simplysign.ps1`.
