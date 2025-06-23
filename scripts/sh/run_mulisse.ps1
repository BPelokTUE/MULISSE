$configsDir = "scripts/run_configs"

$configs = @()
foreach ($arg in $args) {
    if (-not (Test-Path "$configsDir/$arg")) {
        if ($arg.StartsWith("$configsDir/")) {
            $newArg = $arg.Substring("$configsDir/".Length)
            if (Test-Path "$configsDir/$newArg") {
                $configs += $newArg
                continue
            }
        }
        Write-Host "Config file $arg not found. Config files should be given relative to $configsDir"
        exit 1
    }
    $configs += $arg
}

docker build --rm -t mulisse .
foreach ($config in $configs) {
    # Fixed line with proper parentheses
    $logsDir = "EXPERIMENT_LOGS/$(Split-Path -Parent $config)/LOGS_$((Split-Path -Leaf $config) -Replace '\.json$','')"
    
    # Print the directory name for verification
    Write-Host "Creating logs directory: $logsDir"
    
    New-Item -Path $logsDir -ItemType Directory -Force
    docker run --rm -v "${PWD}/${logsDir}:/mulisse/LOGS" -v "${PWD}/mulisse_pack:/mulisse/mulisse_pack" mulisse -i "/mulisse/scripts/run_configs/$config"
}