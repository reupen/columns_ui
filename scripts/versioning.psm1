function Get-GitDescription {
    $description_regex = '^v(?<version>.+)-(?<distance>\d+)-g(?<commit>[0-9a-f]{7})(-(?<dirty>dirty))?$'

    Try {
        $description = git describe --tags --dirty --match 'v[0-9]*' --long
    } Catch {
        return $null
    }
    
    If (-not ($description -match $description_regex)) {
        return $null
    }

    return $Matches
}

function Get-Version {
    $description = Get-GitDescription
    
    if (-not $description) {
        return '0.0.0+unknown'
    }

    $base_version = $description.version
    $annotations = @()

    if ([int]$description.distance) {
        $annotations += @($description.distance, "g$($description.commit)")
    }

    if ($description.dirty) {
        $annotations += @($description.dirty)
    }

    if ($annotations) {
        $joined_annotations = $annotations -Join '.'
        return "$base_version+$joined_annotations"
    }

    return $base_version
}
