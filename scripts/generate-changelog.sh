#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Configuration ---
CHANGELOG_FILE="CHANGELOG.md"

# --- Get Git Tags ---
# Get the current tag that triggered the workflow
CURRENT_TAG=$(git tag --points-at HEAD)

if [ -z "$CURRENT_TAG" ]; then
  echo "Error: No tag found at the current commit. This script should be run on a tagged commit."
  exit 1
fi

# Get the previous tag
PREVIOUS_TAG=$(git describe --tags --abbrev=0 HEAD~1 2>/dev/null || git rev-list --max-parents=0 HEAD)

echo "Current tag: $CURRENT_TAG"
echo "Previous tag: $PREVIOUS_TAG"

# --- Get Commit Information ---
# Get the list of commit subjects between the two tags
COMMIT_LOG=$(git log --pretty=format:%s "$PREVIOUS_TAG..$CURRENT_TAG")

# Count the total number of commits
COMMIT_COUNT=$(echo "$COMMIT_LOG" | wc -l)

# Filter commits for features and bug fixes
FEATURES=$(echo "$COMMIT_LOG" | grep -i '^feature: ' | sed -e 's/^feature: //I' -e 's/^/* /')
BUG_FIXES=$(echo "$COMMIT_LOG" | grep -i '^\(fix\|bugfix\): ' | sed -e 's/^\(fix\|bugfix\): //I' -e 's/^/* /')

# --- Generate Changelog File ---
# Get current date and time
RELEASE_DATE=$(date +'%Y-%m-%d')
RELEASE_TIME=$(date +'%H:%M:%S')

# Create the changelog file
{
  echo "# Release $CURRENT_TAG changes"
  echo ""
  echo "Released on $RELEASE_DATE at $RELEASE_TIME UTC."
  echo ""
  echo "$COMMIT_COUNT commits since previous release."
  echo ""

  if [ -n "$FEATURES" ]; then
    echo "## Features"
    echo ""
    echo "$FEATURES"
    echo ""
  fi

  if [ -n "$BUG_FIXES" ]; then
    echo "## Bug fixes"
    echo ""
    echo "$BUG_FIXES"
  fi
} > "$CHANGELOG_FILE"

echo "Changelog generated successfully in $CHANGELOG_FILE"
cat "$CHANGELOG_FILE"
