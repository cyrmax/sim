cmake --preset windows-msvc-x86-release -DSIM_VERSION="%GITHUB_REF_NAME%" -DSIM_BUILD_NUMBER="%GITHUB_RUN_NUMBER%" && cmake --build --preset release-x86
