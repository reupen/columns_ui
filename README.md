# Columns UI

http://yuo.be/columns_ui

[![Build status](https://ci.appveyor.com/api/projects/status/h1iqjogb73f3yqp1/branch/master?svg=true)](https://ci.appveyor.com/project/reupen/columns-ui/branch/master)

Columns UI is released under the Lesser GNU Public Licence (see COPYING and COPYING.LESSER).

A VS2015 solution can be found in the vc14 folder.

To clone the repo and dependencies, run:

`git clone --recursive https://github.com/reupen/columns_ui.git`

## Updates

### 31 December 2015

I'm in the process of transferring all repos to my main GitHub account, so don't be alarmed at the change in repo owner. GitHub should keep redirects in place for the current repo URIs.

### 26 December 2015

There has been further restructure of all the repos with more stuff moved to submodules. The instructions below about cloning/updating clones still apply.

### 29 November 2015

Some libraries have been moved to the [foobar2000-common](https://github.com/msquared2/foobar2000-common) repository and added to this repository as a submodule. 

Use `--recursive` when cloning this repository to clone foobar2000-common at the same time.

For an existing clone, use `git submodule init` and `git submodule update`.
