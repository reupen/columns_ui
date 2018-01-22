# Contribution guidelines

Note: These guidelines are a work in progress.

## Code style

Code in this repository is formatted using [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) to ensure consistency throughout the code base. Code style rules are defined in [.clang-format](.clang-format).

You're advised to format any contributions using ClangFormat tools. There are various options for this, but two useful ones are:

* [The ClangFormat Visual Studio extension](https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.ClangFormat).
  This can format a selection or an entire file.
* `git clang-format`. This formats staged changes (requires [LLVM](https://llvm.org/) and [Python](https://www.python.org/)).

Generally, `git clang-format` is preferred as this limits formatting to the changes being made.
