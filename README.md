# Columns UI

http://yuo.be/columns_ui

[![Build status](https://ci.appveyor.com/api/projects/status/h1iqjogb73f3yqp1/branch/master?svg=true)](https://ci.appveyor.com/project/reupen/columns-ui/branch/master)

Columns UI is released under the Lesser GNU Public Licence (see COPYING and COPYING.LESSER).

To clone the repo and dependencies, [download and install Git](https://git-scm.com/downloads), and then run:

`git clone --recursive https://github.com/reupen/columns_ui.git`

This repo makes use of Git submodules. If you're not familiar with them, [check out the guide here](https://git-scm.com/book/en/v2/Git-Tools-Submodules).

## Build instructions

Visual Studio 2017 is required to build Columns UI. You can use the [free community edition](https://www.visualstudio.com/downloads/).

You'll need the Windows SDK for Windows 8.1 (installed by default with Visual Studio 2017).

### Using the Visual Studio IDE
Open `vc15/columns_ui-public.sln` in Visual Studio 2017. 
Select the Release configuration and the Win32 platform, and build the solution. 
If the build is successful, `foo_ui_columns.dll` will be output in `vc15\Release`.

### Using MSBuild on the command line

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

If you're feeling adventurous, you can compile Columns UI using Clang 4.0.0 or newer and MSBuild. Download and install a [release build](http://llvm.org/releases/download.html) or a [snapshot build](http://llvm.org/builds/) from the LLVM website. In a VS2015 Native Tools x86 command prompt, run:

```
msbuild /m /p:PlatformToolset=LLVM-vs2014 /p:Platform=Win32 /p:Configuration=Release /t:Rebuild vc15\columns_ui-public.sln
```
