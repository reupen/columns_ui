Param(
    [Parameter(Mandatory=$true)][String]$ComponentPath,
    [Parameter(Mandatory=$true)][String]$OutputDir
)

$input_base_name = [io.path]::GetFileNameWithoutExtension($ComponentPath)
$output_path = "$OutputDir$input_base_name.fb2k-component"
$intermediate_file_path = "$output_path.zip"

# Note that Compress-Archive won't accept a file extension other than .zip. Hence, we add .zip to the file name
# and then remove it using Move-Item.
# (Also note that both Compress-Archive and Move-Item should overwite the destination file if it exists.)
Compress-Archive -Path $ComponentPath -CompressionLevel Optimal -DestinationPath $intermediate_file_path -Force
Move-Item $intermediate_file_path $output_path -Force
