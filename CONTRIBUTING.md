# Contribution guidelines

Note: These guidelines are a work in progress.

## Code style

### Automated formatting

Code in this repository is formatted using [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) to ensure consistency throughout the code base. Code style rules are defined in [.clang-format](.clang-format) (including rules for the placement of braces).

You're advised to format any contributions using ClangFormat tools. There are various options for this, but two useful ones are:

- `git clang-format`. This formats staged changes (requires [LLVM](https://llvm.org/) and [Python](https://www.python.org/)).

- [The ClangFormat Visual Studio extension](https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.ClangFormat). This can format a selection or an entire file.

Generally, `git clang-format` is preferred as this limits formatting to the changes being made.

### Spaces and line endings

Spaces (four per level) are used for indentation.

Unix newlines are always committed to the repo. (Git should automatically do this.)

### Naming conventions

Use UpperCamelCase for user-defined classes, types etc.

For functions and variables use lower_case_separated_by_underscores.

Static data and function members of classes are prefixed with s\_.

Non-static data members are prefixed with m\_.

British English is normally used both in names in code (class names, function names etc.) and in text in the UI. (An exception can be made for subclasses and instances of classes from other libraries.)

## Other notes

New code should not introduce additional warnings during builds.

Much of the code in the repo was written a long time ago, so it may not conform to the above naming conventions and should not necessarily be used as a guide for new code.
