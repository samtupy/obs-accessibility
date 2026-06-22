[CmdletBinding()]
param(
    [ValidateSet('x64')]
    [string] $Target = 'x64',
    [ValidateSet('Debug', 'RelWithDebInfo', 'Release', 'MinSizeRel')]
    [string] $Configuration = 'RelWithDebInfo',
    [switch] $Package
)

$ErrorActionPreference = 'Stop'

if ( $DebugPreference -eq 'Continue' ) {
    $VerbosePreference = 'Continue'
    $InformationPreference = 'Continue'
}

if ( $env:CI -eq $null ) {
    throw "Package-Windows.ps1 requires CI environment"
}

if ( ! ( [System.Environment]::Is64BitOperatingSystem ) ) {
    throw "Packaging script requires a 64-bit system to build and run."
}

if ( $PSVersionTable.PSVersion -lt '7.2.0' ) {
    Write-Warning 'The packaging script requires PowerShell Core 7. Install or upgrade your PowerShell version: https://aka.ms/pscore6'
    exit 2
}

function Package {
    trap {
        Write-Error $_
        exit 2
    }

    $ScriptHome = $PSScriptRoot
    $ProjectRoot = Resolve-Path -Path "$PSScriptRoot/../.."
    $BuildSpecFile = "${ProjectRoot}/buildspec.json"

    $UtilityFunctions = Get-ChildItem -Path $PSScriptRoot/utils.pwsh/*.ps1 -Recurse

    foreach( $Utility in $UtilityFunctions ) {
        Write-Debug "Loading $($Utility.FullName)"
        . $Utility.FullName
    }

    $BuildSpec = Get-Content -Path ${BuildSpecFile} -Raw | ConvertFrom-Json
    $ProductName = $BuildSpec.name
    $ProductVersion = $BuildSpec.version
    $ProductAuthor = $BuildSpec.author
    $ProductWebsite = $BuildSpec.website

    $OutputName = "${ProductName}-${ProductVersion}-windows-${Target}"

    $RemoveArgs = @{
        ErrorAction = 'SilentlyContinue'
        Path = @(
            "${ProjectRoot}/release/${ProductName}-*-windows-*.zip"
            "${ProjectRoot}/release/${ProductName}-*-windows-*-setup.exe"
        )
    }

    Remove-Item @RemoveArgs

    Log-Group "Archiving ${ProductName}..."
    $CompressArgs = @{
        Path = (Get-ChildItem -Path "${ProjectRoot}/release/${Configuration}" -Exclude "${OutputName}*.*")
        CompressionLevel = 'Optimal'
        DestinationPath = "${ProjectRoot}/release/${OutputName}.zip"
        Verbose = ($Env:CI -ne $null)
    }
    Compress-Archive -Force @CompressArgs
    Log-Group

    if ( $Package ) {
        Log-Group "Building installer for ${ProductName}..."

        $PackageDir = "${ProjectRoot}/release/Package/${ProductName}"
        if ( Test-Path $PackageDir ) { Remove-Item -Recurse -Force $PackageDir }
        New-Item -ItemType Directory -Force -Path $PackageDir | Out-Null

        $CopyArgs = @{
            Path = (Get-ChildItem -Path "${ProjectRoot}/release/${Configuration}" -Exclude "*.zip")
            Destination = $PackageDir
            Recurse = $true
            Force = $true
        }
        Copy-Item @CopyArgs

        $IssContent = Get-Content -Path "${ProjectRoot}/install.iss.in" -Raw
        $IssContent = $IssContent -replace '@CMAKE_PROJECT_NAME@', $ProductName
        $IssContent = $IssContent -replace '@CMAKE_PROJECT_VERSION@', $ProductVersion
        $IssContent = $IssContent -replace '@PLUGIN_AUTHOR@', $ProductAuthor
        $IssContent = $IssContent -replace '@PLUGIN_WEBSITE@', $ProductWebsite
        Set-Content -Path "${ProjectRoot}/install.iss" -Value $IssContent

        $env:PATH = "C:\Program Files (x86)\Inno Setup 6;$env:PATH"
        Invoke-External ISCC.exe "${ProjectRoot}/install.iss"

        $InstallerSrc = "${ProjectRoot}/release/${ProductName}-${ProductVersion}-setup.exe"
        $InstallerDst = "${ProjectRoot}/release/${OutputName}-setup.exe"
        if ( Test-Path $InstallerSrc ) {
            Move-Item -Force $InstallerSrc $InstallerDst
        }

        Log-Group
    }
}

Package
