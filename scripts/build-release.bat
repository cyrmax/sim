cmake --preset windows-msvc-x64-release -DSIM_VERSION="%GITHUB_REF_NAME%" -DSIM_BUILD_NUMBER="%GITHUB_RUN_NUMBER%" && cmake --build --preset release-x64
