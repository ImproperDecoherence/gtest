
param (
    [switch]$clean,    
    [switch]$rebuild,
    [switch]$compile,
    [switch]$help,
    [switch]$test,
    [switch]$app,
    [switch]$doc,
    [switch]$release,
    [switch]$wipe
)

# Function to display help
function Show-Help {
    Write-Host "Usage: .\build.ps1 [options]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -clean      Clean the build directory."
    Write-Host "  -rebuild    Rebuild the build directory."
    Write-Host "  -compile    Compile the project."
    Write-Host "  -test       Run the tests."
    Write-Host "  -app        Run the application."
    Write-Host "  -doc        Generate Doxygen documentaion."
    Write-Host "  -release    Build release project."
    Write-Host "  -help       Show this help message."
    Write-Host "  -wipe       Wipes the build directory."
    Write-Host ""
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
function MesonSetup([string]$buildtype = "debugoptimized") {
    meson setup builddir --buildtype=$buildtype
}

# Function to wipe the build directory
function Wipe {
    if (Test-Path -Path builddir) {
        Write-Host "Removing build directory..."
        Remove-Item -Recurse -Force builddir
    }
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
        if ($release) {            
            MesonSetup "release"
        }
        else {
            MesonSetup "debugoptimized"
        }
    }
    elseif (($release -and $currentBuildType -ne "release") -or (-not $release -and $currentBuildType -eq "release")) {
        Write-Host "Current build type is $currentBuildType. Rebuilding for the correct build type..."
        if (Test-Path -Path builddir) {
            Remove-Item -Recurse -Force builddir
        }
        if ($release) {
            MesonSetup "release"
        }
        else {            
            MesonSetup "debugoptimized"
        }
    }
    else {
        Write-Host "Current build type is already $currentBuildType. No rebuild needed."
    }
}

# Function to compile the project
function Compile {
    if (-not (Test-Path -Path builddir)) {
        Rebuild
    }

    Write-Host "Compiling the project..."
    meson compile -C builddir

    if (-not $?) {
        Write-Host "Compilation failed. Exiting."
        exit 1
    }
}

# Function to run the tests
function Test {
    Write-Host "Running the tests..."
    ./builddir/run_tests.exe
}

# Function to run the application
function App {
    Write-Host "Running the application..."
    ./builddir/gchordlabs.exe
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


if ($wipe) {
    Wipe
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

if ($app) {
    App
}

if ($doc) {
    Write-Doxygen
}


# Default action if no flags are provided
if (-not ($clean -or $rebuild -or $compile -or $test -or $doc -or $wipe)) {
    Write-Host "No flags provided."
    Show-Help
}