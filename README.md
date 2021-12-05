# Columns UI

[![Build status](https://reupen.visualstudio.com/Columns%20UI/_apis/build/status/reupen.columns_ui?branchName=master)](https://reupen.visualstudio.com/Columns%20UI/_build/latest?definitionId=3&branchName=master)

Columns UI is released under the Lesser GNU Public Licence (see COPYING and COPYING.LESSER).

## Downloads

Releases can be downloaded from the [Columns UI home page](http://yuo.be/columns-ui).

The latest development version can be downloaded by clicking on the Azure Pipelines badge above, then '1 published' and then the three dots shown while hovering over 'VS 2019 v142 Release'. Development versions may be buggier than formal releases; if you encounter a problem, please open an issue.

## Development

To clone the repo and dependencies, [download and install Git](https://git-scm.com/downloads), and then run:

`git clone --recursive https://github.com/reupen/columns_ui.git`

This repo makes use of Git submodules. If you're not familiar with them, [check out the guide here](https://git-scm.com/book/en/v2/Git-Tools-Submodules).

### Build instructions

Visual Studio 2019 is required to build Columns UI. You can use the [free community edition](https://www.visualstudio.com/downloads/) (select the Desktop development with C++ workload during installation).

#### Installing external dependencies

The following libraries are required to build Columns UI:

- Microsoft Guideline Support Library (GSL)
- Microsoft Windows Implementation Library (WIL)
- range-v3

The recommended way to install them is using [vcpkg](https://github.com/Microsoft/vcpkg).

You can set up vcpkg, and install Microsoft GSL, using the following commands (run outside of the Columns UI source tree):

```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install ms-gsl wil range-v3 --overlay-ports=..\columns_ui\ports
```

(Note: Change the `..\columns_ui\ports` path in the `.\vcpkg install` command as necessary.)

#### Building using the Visual Studio IDE
Open `vc16/columns_ui-public.sln` in Visual Studio 2019.

Select the Release configuration and the Win32 platform, and build the solution.

If the build is successful, `foo_ui_columns.dll` will be output in `vc16\Release`.

#### Building using MSBuild on the command line

You can use MSBuild if you prefer. In a Developer Command Prompt for VS 2019 (in the start menu), run:

```
msbuild /m /p:Platform=Win32 /p:Configuration=Release vc16\columns_ui-public.sln
```

If the build is successful, `foo_ui_columns.dll` will be output in `vc16\Release`.

For a clean build, run:

```
msbuild /m /p:Platform=Win32 /p:Configuration=Release /t:Rebuild vc16\columns_ui-public.sln
```

#### Using the Clang compiler (experimental)

Note: Currently not functional out of the box – should be functional again when the LLVM bundled with Visual Studio is updated to version 13.

Columns UI can be also compiled using the version of Clang distributed with Visual Studio. 

(Note that Clang is not installed by default – in the Visual Studio 2019 installer, you will need to select the Clang compiler and the Clang build tools components.)

With these installed, open a Developer Command Prompt for VS 2019 from the start menu, switch to the Columns UI source directory and run:

```
msbuild /m /p:PlatformToolset=ClangCL;UseLldLink=True;VcpkgAutoLink=False;WholeProgramOptimization=False;Platform=Win32;Configuration=Release /t:Rebuild vc16\columns_ui-public.sln
```
