# Columns UI

[![Build status](https://github.com/reupen/columns_ui/actions/workflows/build.yml/badge.svg)](https://github.com/reupen/columns_ui/actions/workflows/build.yml?query=branch%3Amain)
[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/reupen/columns_ui/main.svg)](https://results.pre-commit.ci/latest/github/reupen/columns_ui/main)
[![Codacy code quality](https://app.codacy.com/project/badge/Grade/7a892c27551745ea883810ab7493983d)](https://www.codacy.com/gh/reupen/columns_ui/dashboard)

Columns UI is released under the Lesser GNU Public Licence (see COPYING and
COPYING.LESSER).

## Downloads

### Releases

Stable and pre-release versions can be downloaded from the
[Columns UI home page](http://yuo.be/columns-ui).

### Development versions

If you’re logged into GitHub, you can download the latest development version by
visiting the
[list of recent GitHub Actions builds](https://github.com/reupen/columns_ui/actions/workflows/build.yml?query=branch%3Amain),
clicking on the most recent entry with a green tick, and then scrolling down to
the links named ‘Component package (release, win32)’ and ‘Component package
(release, x64)’ at the bottom.

Development versions may be buggier than formal releases; if you encounter a
problem, open an issue.

## Development

To clone the repo and dependencies,
[download and install Git](https://git-scm.com/downloads), and then run:

```powershell
git clone --recursive https://github.com/reupen/columns_ui.git
```

This repo makes use of Git submodules. If you're not familiar with them,
[check out the guide here](https://git-scm.com/book/en/v2/Git-Tools-Submodules).

### Build instructions

Visual Studio 2022, the Windows 11 SDK (version 10.0.26100.0 or newer),
[vcpkg](https://github.com/Microsoft/vcpkg) and Python 3.12 (including the `py`
launcher) are required to build Columns UI.

You can use the
[free community edition of Visual Studio](https://www.visualstudio.com/downloads/).
During installation, select the Desktop development with C++ workload and the
Windows 11 SDK from the right-hand side.

#### Installing vcpkg

You can install and set up vcpkg by running the following commands (in a
directory of your choice outside the Columns UI source tree):

```powershell
git clone https://github.com/Microsoft/vcpkg.git
vcpkg\bootstrap-vcpkg.bat
vcpkg\vcpkg integrate install
```

Dependencies will then be automatically installed when Columns UI is built.

(You’ll need to occasionally run `git pull` in the vcpkg directory to fetch
updated package metatdata.)

#### Building using the Visual Studio IDE

Open `vc17/columns_ui-public.sln` in Visual Studio 2022.

Select the Release configuration and a platform (Win32 or x64), and build the
solution.

If the build is successful, `foo_ui_columns.<platform>.fb2k-component` will be
output in `vc17\release-<platform>-v143`.

#### Building using MSBuild on the command line

You can use MSBuild if you prefer. To build a 32-bit component, start a
Developer Command Prompt for VS 2022 (from the start menu), and run:

```powershell
msbuild /m "/p:Platform=Win32;Configuration=Release" vc17\columns_ui-public.sln
```

If the build is successful, `foo_ui_columns.x86.fb2k-component` will be output
in `vc17\release-win32-v143`.

For a clean build, run:

```powershell
msbuild /m "/p:Platform=Win32;Configuration=Release" "/t:Rebuild" vc17\columns_ui-public.sln
```

##### Using the Clang compiler (experimental)

Columns UI can be also compiled using the version of Clang distributed with
Visual Studio.

(Note that Clang is not installed by default – in the Visual Studio 2022
installer, you will need to select the Clang compiler and the Clang build tools
components.)

With these installed, open a Developer Command Prompt for VS 2022 from the start
menu, switch to the Columns UI source directory and run:

```powershell
msbuild /m "/p:PlatformToolset=ClangCL;LinkToolExe=link.exe;VcpkgAutoLink=true;Platform=Win32;Configuration=Release" vc17\columns_ui-public.sln
```

(Note: Currently `lld-link.exe` can't be used due to
[missing wildcard support](https://github.com/llvm/llvm-project/issues/38333).)

#### Building a release package

A universal release package (containing x86 and x64 build) can be created by
running:

```
py scripts\build-release.package.py
```

If successful, a component package will be created in the `component-packages`
directory.

(A Visual Studio Developer Command Prompt is not required to run the script.)

## Documentation site

This repo also includes the source for a
[documentation site](https://columns-ui.readthedocs.io) in the `docs` directory.
See [docs/README.md](./docs/README.md) for more information.
