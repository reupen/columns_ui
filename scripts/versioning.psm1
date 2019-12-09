class Version {
    [string]$BaseVersion;
    [int]$Distance;
    [string]$CommitHash;
    [bool]$Dirty;

    Version([string]$BaseVersion, [int]$Distance, [string]$CommitHash, [bool]$Dirty) {
        $this.BaseVersion = $BaseVersion
        $this.Distance = $Distance
        $this.CommitHash = $CommitHash
        $this.Dirty = $Dirty
    }

    [string]ToString() {
        $annotations = @()
    
        if ($this.Distance) {
            $annotations += @($this.Distance, "g$($this.CommitHash)")
        }
    
        if ($this.Dirty) {
            $annotations += 'dirty'
        }
    
        if ($annotations) {
            $joinedAnnotations = $annotations -Join '.'
            return "$($this.BaseVersion)+$joinedAnnotations"
        }
    
        return $this.BaseVersion
    }

    [bool]IsRelease() {
        return $this.Distance -eq 0 -and -not $this.Dirty
    }

    static [Version]FromGit() {
        $descriptionRegex = '^v(?<version>.+)-(?<distance>\d+)-g(?<commit>[0-9a-f]{7})(-(?<dirty>dirty))?$'

        $description = git describe --tags --dirty --match 'v[0-9]*' --long
        
        If (-not ($description -match $descriptionRegex)) {
            throw
        }
    
        return [Version]::new($Matches.version, [int]$Matches.distance, $Matches.commit, [bool]$Matches.dirty)
    }
}

function Get-Version {
    try {
        return [Version]::FromGit()
    }
    catch {
        return [Version]::new(0, 0, 'unknown', $false)
    }
}
