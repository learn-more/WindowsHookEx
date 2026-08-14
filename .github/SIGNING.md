# Code signing

Release binaries are Authenticode signed with a Certum code signing certificate, through
[learn-more/certum-signing](https://github.com/learn-more/certum-signing). How that works - the cloud HSM, the
SimplySign session, the TOTP login, where to get each value - is documented there, in
[SIGNING.md](https://github.com/learn-more/certum-signing/blob/main/SIGNING.md). This file is only what is
specific to WindowsHookEx.

**Signing is optional.** Without the settings below a release still builds; it just comes out unsigned and says
so, in an annotation, the job summary, and the release notes. Nothing has to be configured to cut a release.

Only [`release.yml`](workflows/release.yml) signs. `msbuild.yml` does not, on purpose: pull request builds from
forks cannot see secrets anyway, and every signature costs a login to a certificate that is meant for artifacts
people actually download.

## What to store

**Settings → Secrets and variables → Actions → Secrets**

| Secret | What it is |
| --- | --- |
| `CERTUM_USERNAME` | The SimplySign account name. |
| `CERTUM_OTP_URI` | The whole `otpauth://totp/...` URI from SimplySign enrolment. |

**Settings → Secrets and variables → Actions → Variables**

| Variable | Required | Default |
| --- | --- | --- |
| `CERTUM_CERTIFICATE_SHA1` | yes | - |
| `CERTUM_TIMESTAMP_SERVER` | no | `http://time.certum.pl` |
| `CERTUM_INTERMEDIATE_URL` | no | `https://repository.certum.pl/ccsca2021.cer` |

All three of the first are required together; missing any one turns signing off for the whole workflow. The
thumbprint is a variable rather than a secret because a certificate thumbprint is public information, and a wrong
one is much easier to spot in a log than a masked `***`.

These are stored per repository. Renewing the certificate changes `CERTUM_CERTIFICATE_SHA1` here and in every
other repository that signs.

## What gets signed, and in what order

Four files, not two: `WindowsHookEx-x86.exe`, `HookDll-x86.dll`, `WindowsHookEx-x64.exe`, `HookDll-x64.dll`.

The DLLs are the hook payloads - `SetWindowsHookEx` maps them into every process that is hooked, so they matter
at least as much as the executable that installs them. They ship *next to* the executable rather than embedded in
it, so all four go in the action's `path:`, none in `embedded-path:`, and each one counts towards the
signed/unsigned verdict on its own.

Nothing is embedded, so the ordering is the simple one - build everything, then open the SimplySign session, then
sign. That keeps the session as short as it can be: one TOTP code has to cover the signing only, not the build in
front of it.

1. **Build** both platforms into `src\bin\`.
2. **Sign and report** - `certum-signing@v1` with `stage: all`, which connects, signs, and then reads the result
   back off the files. The run fails here if signing was configured and did not take.
3. **Package** the four binaries and their PDBs into two per-platform archives, *after* signing - what a user
   runs is whatever came out of the zip.

`WindowsHookEx.exe` carries a `FileDescription` from [`src/WindowsHookEx.rc2`](../src/WindowsHookEx.rc2), so that
is the name Windows shows in its UAC and SmartScreen dialogs and the string stays in a single place. `HookDll`
has no version resource, so it takes the `description:` the workflow passes instead.

## One login per build

A login spends a one-time code, so there must be exactly one per build. Two inside the same 30-second TOTP step
present the *same* code twice, which Certum may refuse - surfacing as a release that fails to sign for no visible
reason, on some builds and not others.

Hence `release.yml` builds x86 and x64 in **one job** with two explicit `msbuild` steps, rather than the
`strategy.matrix` that `msbuild.yml` uses. Restoring the matrix here means solving the login first - serialising
it, or signing in a single job that the others feed. The cost of one job: a compile error no longer names its
platform without reading the log, and the platforms no longer build in parallel.
