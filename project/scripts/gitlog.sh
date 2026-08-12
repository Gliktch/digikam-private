#!/bin/bash

# ===========================================================
#
# This file is a part of digiKam project
# https://www.digikam.org
#
# Date:        2026-02-14
# Description: Script to generate a Markdown changelog for digiKam between two tags,
#              with categorization, commit hashes, dates, authors, and links to the web interface.
#              Commits are sorted by date (newest first) within each category.
#              Usage: ./gitlog.sh <START_TAG> <END_TAG> [OUTPUT_FILE] [BASE_URL]
#              Example for GitLab: ./gitlog.sh v9.0.0 v9.1.0 digikam_9.1.0_changelog.md "https://invent.kde.org/graphics/digikam/-/commit"
#
# Copyright (C) 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# ============================================================

# Check arguments

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <START_TAG> <END_TAG> [OUTPUT_FILE] [BASE_URL]"
    echo "Example (GitLab): $0 v9.0.0 v9.1.0 changelog.md 'https://invent.kde.org/graphics/digikam/-/commit'"
    exit 1
fi

START_TAG="$1"
END_TAG="$2"
OUTPUT_FILE="${3:-digikam_changelog.md}"
BASE_URL="${4:-https://invent.kde.org/graphics/digikam/-/commit}"  # Default: GitLab

# Repository directory

REPO_DIR="../../"

# Check if the repository exists

if [ ! -d "$REPO_DIR/.git" ]; then

    echo "Error: Git repository not found at $REPO_DIR"
    exit 1

fi

cd "$REPO_DIR" || exit 1

# Patterns to exclude (case-insensitive)

EXCLUDE_PATTERNS=("GIT_SILENT" "codespell" "cppcheck" "clazy" "krazy" "update" "Merge branch" "polish" "typo")

# Patterns for categorization (order matters: first match wins)

declare -A CATEGORIES=(
    ["feat:"]="Features"
    ["new:"]="Features"
    ["fix:"]="Bug Fixes"
    ["bugfix:"]="Bug Fixes"
    ["improve:"]="Improvements"
    ["perf:"]="Improvements"
    ["refactor:"]="Improvements"
)

# Initialize the Markdown file

echo "# Changelog for digiKam between $START_TAG and $END_TAG" > "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "Generated on $(date +'%Y-%m-%d %H:%M:%S')" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "---" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Temporary file to store unique contributors

CONTRIBUTORS_FILE="contributors.tmp"
touch "$CONTRIBUTORS_FILE"

# Temporary files for each category (to store commits before sorting)

for category in "${CATEGORIES[@]}"; do

    temp_file="${category// /_}.tmp"
    touch "$temp_file"

done

# Extract commits, filter them, and categorize

git log "$START_TAG".."$END_TAG" --pretty=format:"%h|%ad|%s|%an" --date=short | \
    grep -i -v -E "$(IFS="|"; echo "${EXCLUDE_PATTERNS[*]}")" | \
    while IFS="|" read -r hash date subject author; do
        # Determine the category
        category="Miscellaneous"
        for pattern in "${!CATEGORIES[@]}"; do
            if [[ "$subject" == *"$pattern"* ]]; then
                category="${CATEGORIES[$pattern]}"
                break
            fi
        done

        # Clean the subject (remove prefixes like "feat:")

        clean_subject=$(echo "$subject" | sed -E "s/^(feat|new|fix|bugfix|improve|perf|refactor): //i")

        # Write to the corresponding category's temp file (format: date|hash|subject|author)
        temp_file="${category// /_}.tmp"
        echo "$date|$hash|$clean_subject|$author" >> "$temp_file"

        # Add contributor to the contributors file (unique)
        echo "$author" >> "$CONTRIBUTORS_FILE"
    done

# Sort each category's commits by date (newest first) and write to the changelog

for category in "${CATEGORIES[@]}"; do
    temp_file="${category// /_}.tmp"

    if [ -s "$temp_file" ]; then

        echo "## $category" >> "$OUTPUT_FILE"
        echo "" >> "$OUTPUT_FILE"

        # Sort by date (descending), then format as Markdown

        sort -t '|' -k1 -r "$temp_file" | \
            while IFS="|" read -r date hash subject author; do
                echo "- [$subject]($BASE_URL/$hash) - $author ($date)" >> "$OUTPUT_FILE"
            done

        echo "" >> "$OUTPUT_FILE"
        rm "$temp_file"

    fi

done

# Handle Miscellaneous category (if it exists)

if [ -s "Miscellaneous.tmp" ]; then

    echo "## Miscellaneous" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"

    sort -t '|' -k1 -r "Miscellaneous.tmp" | \
        while IFS="|" read -r date hash subject author; do
            echo "- [$subject]($BASE_URL/$hash) - $author ($date)" >> "$OUTPUT_FILE"
        done

    echo "" >> "$OUTPUT_FILE"
    rm "Miscellaneous.tmp"
fi

# Add contributors section

echo "---" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "## Contributors" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Extract unique contributors, sort them, and add to the changelog

sort -u "$CONTRIBUTORS_FILE" | while read -r contributor; do
    echo "- $contributor" >> "$OUTPUT_FILE"
done

rm "$CONTRIBUTORS_FILE"

# Add a statistics section

echo "" >> "$OUTPUT_FILE"
echo "---" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "## Statistics" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Count commits per category

TOTAL_COMMITS=0
for category in "${CATEGORIES[@]}" "Miscellaneous"; do
    if [ "$category" == "Miscellaneous" ]; then
        count=$(grep -c "^- " "$OUTPUT_FILE" | grep -A 10 "## Miscellaneous" | head -n 1 || echo 0)
    else
        count=$(grep -c "^- " "$OUTPUT_FILE" | grep -A 10 "## $category" | head -n 1 || echo 0)
    fi
    if [ -n "$count" ] && [ "$count" -gt 0 ]; then
        entries=$(sed -n "/^## $category$/,/^## /p" "$OUTPUT_FILE" | grep -c "^-" || echo 0)
        echo "- **$category**: $entries commits" >> "$OUTPUT_FILE"
        TOTAL_COMMITS=$((TOTAL_COMMITS + entries))
    fi
done

# Count total unique contributors

TOTAL_CONTRIBUTORS=$(grep -c "^- " "$OUTPUT_FILE" | tail -n 1 | awk '{print $1}' || echo 0)

# Subtract 1 to exclude the "---" line if it was counted

TOTAL_CONTRIBUTORS=$((TOTAL_CONTRIBUTORS - 1))
echo "- **Total Commits**: $TOTAL_COMMITS" >> "$OUTPUT_FILE"
echo "- **Total Contributors**: $TOTAL_CONTRIBUTORS" >> "$OUTPUT_FILE"

echo ""
echo "Changelog generated in: $OUTPUT_FILE"
