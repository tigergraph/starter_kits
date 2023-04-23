#!/bin/bash

# define the root directory to start the search
ROOT_DIR="$(dirname "$(realpath "$0")")"

# find all tarball files under the root directory
find "$ROOT_DIR" -name "*.tar.gz" | while read tarball_file; do

  # extract the tarball to a temporary directory
  temp_dir=$(mktemp -d)
  tar -xzf "$tarball_file" -C "$temp_dir"
  cd "$temp_dir"
  # remove ExprFunctions.hpp from ExportedGraph.zip under /graph folder
  zip_file="$temp_dir/graph/ExportedGraph.zip"
  if [ -f "$zip_file" ]; then
    zip -d "$zip_file" ExprFunctions.hpp
    filename=$(basename -- "$tarball_file")
    dir="${tarball_file:0:${#tarball_file} - ${#filename}}"
    extension="tar.gz"
    filename="${filename%%.*}"
    tar -czf $dir"/"$filename"_NO_UDF."$extension -C "$temp_dir" .
  fi

  # remove unnecessary files and directories
  rm -rf "$temp_dir"

done