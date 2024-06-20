# CLI commands

Columns UI adds a small number of command-line interface (CLI) commands to
foobar2000 for importing and exporting Columns UI configuration files.

## Available commands

### /columnsui:?

Shows a dialogue box describing the available commands.

#### Example

```shell
foobar2000.exe /columnsui:?
```

### /columnsui:import <path>

Imports a configuration from an FCL file, showing a dialogue box that allows
selection of the configuration sections to import.

#### Example

```shell
foobar2000.exe /columnsui:import "C:\path\to\configuration.fcl"
```

### /columnsui:import-quiet <path>

Imports a configuration from an FCL file, without first showing a dialogue box.

#### Example

```shell
foobar2000.exe /columnsui:import-quiet "C:\path\to\configuration.fcl"
```

### /columnsui:export <path>

Exports the current configuration to an FCL file, showing a dialogue box that
allows selection of the configuration sections to export.

#### Example

```shell
foobar2000.exe /columnsui:export "C:\path\to\configuration.fcl"
```

### /columnsui:export-quiet <path>

Exports the current configuration to an FCL file, without first showing a
dialogue box.

#### Example

```shell
foobar2000.exe /columnsui:export-quiet "C:\path\to\configuration.fcl"
```
