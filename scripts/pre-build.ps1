Param(
    [Parameter(Mandatory=$true)][String]$TemplatePath,
    [Parameter(Mandatory=$true)][String]$OutputPath
)

Import-Module "$PSScriptRoot\versioning"

$version = Get-Version
$date = (Get-Date).ToString('d MMMM yyyy')

$header_template = Get-Content $TemplatePath -Encoding UTF8 -Raw
$header = (
    $header_template.
        Replace('${{ Date }}', $date).
        Replace('${{ Version }}', $version)
)

Write-Host "Version: $version"
Out-File $OutputPath -Encoding 'UTF8' -InputObject $header
