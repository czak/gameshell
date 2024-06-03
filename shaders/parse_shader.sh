#!/bin/sh

if [ -z "$1" ]; then
  echo "Usage: $0 filename"
  exit 1
fi

filename=${1##*/}
basename=${filename%.glsl}

echo "static const char *${basename}_shader_source ="
sed 's/"/\\"/g; s/.*/"&\\n"/' "$1"
echo ";"
