
param (
    [switch]$clean,    
    [switch]$rebuild,
    [switch]$compile,
    [switch]$help,
    [switch]$test,
    [switch]$doc,
    [switch]$debug
)

# Function to display help
function Show-Help {
    Write-Host "Usage: .\build.ps1 [options]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -clean      Clean the build directory."
    Write-Host "  -rebuild    Rebuild the build directory."
    Write-Host "  -compile    Compile the project."
    Write-Host "  -test       Compile and run the tests."
    Write-Host "  -doc        Generate Doxygen documentaion."
    Write-Host "  -debug      Build debug project."
    Write-Host "  -help       Show this help message."
    Write-Host ""
    Write-Host "If no options are provided, the script will perform the default actions: clean, rebuild, and compile."
}

# Function to get the current build type from Meson configuration
function Get-BuildType {
    if (Test-Path -Path builddir/meson-info/intro-buildoptions.json) {
        $buildOptions = Get-Content builddir/meson-info/intro-buildoptions.json | ConvertFrom-Json
        foreach ($option in $buildOptions) {
            if ($option.name -eq "buildtype") {
                return $option.value
            }
        }
    }
    return $null
}

# Function to setup the Meson build directory
function MesonSetup([string]$buildtype = "release") {
    meson setup builddir --buildtype=$buildtype
}

# Function to clean the build directory
function Clean {
    Write-Host "Cleaning build directory..."
    meson compile --clean -C builddir
}

# Function to rebuild the build directory
function Rebuild {
    Write-Host "Checking current build type..."
    $currentBuildType = Get-BuildType
    if ($null -eq $currentBuildType) {
        Write-Host "Build directory does not exist. Setting up a new build directory."
        if ($debug) {
            MesonSetup "debug"
        }
        else {
            MesonSetup "release"
        }
    }
    elseif (($debug -and $currentBuildType -ne "debug") -or (-not $debug -and $currentBuildType -eq "debug")) {
        Write-Host "Current build type is $currentBuildType. Rebuilding for the correct build type..."
        if (Test-Path -Path builddir) {
            Remove-Item -Recurse -Force builddir
        }
        if ($debug) {
            MesonSetup "debug"
        }
        else {
            MesonSetup "release"
        }
    }
    else {
        Write-Host "Current build type is already $currentBuildType. No rebuild needed."
    }
}

# Function to compile the project
function Compile {
    Write-Host "Compiling the project..."
    meson compile -C builddir

    if (-not $?) {
        Write-Host "Compilation failed. Exiting."
        exit 1
    }
}

# Function to run the tests
function Test {
    Write-Host "No tests are implemented!"
    #./builddir/runtests.exe
}

# Function generate Doxygen documentation
function Write-Doxygen {
    Write-Host "Generating Doxygen documentation..."
    doxygen Doxyfile
}

# Execute actions based on flags
if ($help) {
    Show-Help
    exit
}

if ($clean) {
    Clean
}

if ($rebuild) {
    Rebuild
}

if ($compile) {
    Compile
}

if ($test) {
    Test
}

if ($doc) {
    Write-Doxygen
}


# Default action if no flags are provided
if (-not ($clean -or $rebuild -or $compile -or $test -or $doc)) {
    Write-Host "No flags provided. Performing default actions: clean, rebuild, and compile."
    Clean
    Rebuild
    Compile
}