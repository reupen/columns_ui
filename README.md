# Columns UI

http://yuo.be/columns_ui

[![Build status](https://ci.appveyor.com/api/projects/status/h1iqjogb73f3yqp1/branch/master?svg=true)](https://ci.appveyor.com/project/reupen/columns-ui/branch/master)

Columns UI is released under the Lesser GNU Public Licence (see COPYING and COPYING.LESSER).

To clone the repo and dependencies, run:

`git clone --recursive https://github.com/reupen/columns_ui.git`

This repo makes use of git submodules. If you're not familiar with them, [check out the guide here](https://git-scm.com/book/en/v2/Git-Tools-Submodules).

## Build instructions

Visual Studio 2015 Update 1 is required to build Columns UI. You can use the [free community edition](https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx).

You may also need the [Windows SDK for Windows 10](https://dev.windows.com/en-us/downloads/windows-10-sdk) and the [Windows SDK for Windows 7 and .NET Framework 4](https://www.microsoft.com/en-gb/download/details.aspx?id=8279).

### Using the Visual Studio IDE
Open `vc14/columns_ui-public.sln` in Visual Studio 2015 Update 1. 
Select the Release configuration and the Win32 platform, and build the solution. 
If the build is successful, `foo_ui_columns.dll` will be output in `vc14\Release`.

### Using MSBuild

You can use MSBuild if you prefer. In a VS2015 Native Tools x86 command prompt, run:

```
msbuild /m /p:Platform=Win32 /p:Configuration=Release vc14\columns_ui-public.sln
```

If the build is successful, `foo_ui_columns.dll` will be output in `vc14\Release`.

For a clean build, run:

```
msbuild /m /p:Platform=Win32 /p:Configuration=Release /t:Rebuild vc14\columns_ui-public.sln
```
