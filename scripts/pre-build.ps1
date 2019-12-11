Param(
    [Parameter(Mandatory=$true)][string]$TemplatePath,
    [Parameter(Mandatory=$true)][string]$OutputPath
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
