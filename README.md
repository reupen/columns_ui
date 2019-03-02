# Columns UI

http://yuo.be/columns_ui

[![Build status](https://ci.appveyor.com/api/projects/status/h1iqjogb73f3yqp1/branch/master?svg=true)](https://ci.appveyor.com/project/reupen/columns-ui/branch/master)

Columns UI is released under the Lesser GNU Public Licence (see COPYING and COPYING.LESSER).

To clone the repo and dependencies, [download and install Git](https://git-scm.com/downloads), and then run:

`git clone --recursive https://github.com/reupen/columns_ui.git`

This repo makes use of Git submodules. If you're not familiar with them, [check out the guide here](https://git-scm.com/book/en/v2/Git-Tools-Submodules).

## Build instructions

Visual Studio 2017 15.9 is required to build Columns UI. You can use the [free community edition](https://www.visualstudio.com/downloads/).

You'll need Windows 10 SDK version 10.0.17763.0 (an installation option in Visual Studio).

### Installing external dependencies

The Microsoft Guideline Support Library (GSL) and range-v3 are required to build Columns UI.

The recommended way to install them is using [vcpkg](https://github.com/Microsoft/vcpkg).

You can set up vcpkg, and install Microsoft GSL, using the following commands (run outside of the Columns UI source tree):

```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install ms-gsl range-v3
```

### Building using the Visual Studio IDE
Open `vc15/columns_ui-public.sln` in Visual Studio 2017.

Select the Release configuration and the Win32 platform, and build the solution.

If the build is successful, `foo_ui_columns.dll` will be output in `vc15\Release`.

### Building using MSBuild on the command line

You can use MSBuild if you prefer. In a Developer Command Prompt for VS 2017 (in the start menu), run:

```
msbuild /m /p:Platform=Win32 /p:Configuration=Release vc15\columns_ui-public.sln
```

If the build is successful, `foo_ui_columns.dll` will be output in `vc15\Release`.

For a clean build, run:

```
msbuild /m /p:Platform=Win32 /p:Configuration=Release /t:Rebuild vc15\columns_ui-public.sln
```

### Using the Clang compiler (experimental)

Compilation using Clang is not currently supported, as Clang does not currently integrate with VS 2017, and also as Clang rejects the `/permissive-` compiler option.
