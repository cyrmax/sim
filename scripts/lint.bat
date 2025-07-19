@echo off
setlocal enabledelayedexpansion

:: ============================================================================
:: This script runs clang-tidy to find lint issues and potential bugs.
::
:: It requires a compilation database generated with a specific 'lint' preset.
:: ============================================================================

set "LINT_PRESET_NAME=windows-msvc-x64-debug"
set "BUILD_DIR=build/!LINT_PRESET_NAME!"

if not exist "!BUILD_DIR!/compile_commands.json" (
    echo [INFO] Compilation database not found. Configuring project with preset '!LINT_PRESET_NAME!'...
    cmake --preset=!LINT_PRESET_NAME!
    if errorlevel 1 (
        echo [ERROR] CMake configuration failed.
        exit /b 1
    )
)

echo [INFO] Using compilation database from: !BUILD_DIR!
echo [INFO] Running clang-tidy on the project...
echo ---

set "SOURCE_FILES="
for /r "src" %%f in (*.cpp) do (
    set "SOURCE_FILES=!SOURCE_FILES! "%%f""
)

clang-tidy -p "!BUILD_DIR!" !SOURCE_FILES! ^
    --header-filter="src/.*" ^
    --warnings-as-errors="*" ^
    --extra-arg="-Wno-unknown-warning-option" ^
    --extra-arg="-Wno-unused-command-line-argument"


if %errorlevel% neq 0 (
    echo ---
    echo [FAILURE] clang-tidy found issues
    echo Please fix the reported errors and warnings.
    exit /b %errorlevel%
) else (
    echo ---
    echo [SUCCESS] clang-tidy found no issues. Great work!
    exit /b 0
)

endlocal
