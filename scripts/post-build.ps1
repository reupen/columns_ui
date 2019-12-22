Param(
    [Parameter(Mandatory=$true)][string]$ComponentPath,
    [Parameter(Mandatory=$true)][string]$OutputDir
)

Import-Module "$PSScriptRoot\versioning"

$version = Get-Version

$inputBaseName = [io.path]::GetFileNameWithoutExtension($ComponentPath)

$verionPart = ''

if ($version -and ($version.IsRelease() -or $Env:TF_BUILD -eq 'True')) {
    $verionPart = "-$version"
}

$outputPath = "$OutputDir$inputBaseName$verionPart.fb2k-component"
$intermediateFilePath = "$outputPath.zip"

# Note that Compress-Archive won't accept a file extension other than .zip. Hence, we add .zip to the file name
# and then remove it using Move-Item.
# (Also note that both Compress-Archive and Move-Item should overwite the destination file if it exists.)
Compress-Archive -Path $ComponentPath -CompressionLevel Optimal -DestinationPath $intermediateFilePath -Force
Move-Item $intermediateFilePath $outputPath -Force
