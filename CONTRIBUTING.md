# Contribution guidelines

Note: These guidelines are a work in progress.

## Code style

### Automated formatting

C++ code in this repository is formatted using
[ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) to ensure
consistency throughout the code base. [Prettier](https://prettier.io/) is used
for Markdown, JSON and YAML files.

A [pre-commit](https://pre-commit.com/) configuration is included to run these
tools automatically on commit. It’s recommended that you set it up locally.
(However, a bot will also check and reformat pull requests if needed.)

### Spaces and line endings

Spaces (four per level) are used for indentation.

Unix newlines are always committed to the repo. (Git should automatically do
this.)

### Naming conventions

Use UpperCamelCase for user-defined classes, types etc.

For functions and variables use lower_case_separated_by_underscores.

Static data and function members of classes are prefixed with s\_.

Non-static data members are prefixed with m\_.

(These prefixes can be omitted for simple data structures.)

British English is normally used both in names in code (class names, function
names etc.) and in text in the UI. (An exception can be made for subclasses and
instances of classes from other libraries.)

## Other notes

New code should not introduce additional warnings during builds.

Much of the code in the repo was written a long time ago, so it may not conform
to the above naming conventions and should not necessarily be used as a guide
for new code.
