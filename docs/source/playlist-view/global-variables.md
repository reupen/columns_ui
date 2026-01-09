# Global variables

Global variables are variables that are defined in the global variables script,
and can be accessed in the display and style scripts of columns.

To use global variables, the ‘Use global variables for display’ option on the
Globals tab on the Playlist view preferences page must be enabled.

The global variables script is executed once per track; it does not let you
persist or share variables across different tracks.

## Global variables script

This script is where global variables are defined. It can be edited from the
Globals tab on the Playlist view preferences page.

### Setting variables

You can set variables using the `$set_global()` function. The syntax is as
follows:

```
$set_global(<variable name>,<variable value>)
```

```{note}
You cannot access any defined global variables within the global
script itself.
```

## Accessing global variables

You can access global variables in your display and style scripts using the
`$get_global()` function. You can also use `$get_global()` in column sorting
scripts if the ‘Use global variables when sorting by column’ option is enabled.

The syntax for using `$get_global()` is as follows:

```
$get_global(<variable name>)
```

`$get_global()` can also be used in boolean contexts such as with `$if()` (where
it will evaluate to true if the variable exists).
