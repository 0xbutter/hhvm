#! /usr/bin/env bash

# Usage: First do 'make -C ../src test' to generate the .out files. If there are
# failures, their test filenames will be printed to stdout. Pass these as
# arguments to this script.

# note: we don't use [ -z $FOO ] to check if FOO is unset because that is also
# true if FOO=""
if [ -z "${OUT_EXT+x}" ]; then
  OUT_EXT=".out"
fi

if [ -z "${EXP_EXT+x}" ]; then
  EXP_EXT=".exp"
fi

if [ -z "${NO_COPY+x}" ]; then
  NO_COPY=false
fi

ARROW="$(tput bold)$(tput setaf 6)==>$(tput setaf 7)"

for f in "$@"; do
  echo "$ARROW $f $(tput sgr0)"
  nl --body-numbering=a "$f"
  echo
  if [ -e "$f$EXP_EXT" ]; then
    EXP="$f$EXP_EXT"
  else
    EXP=/dev/null
  fi

  echo "$ARROW Diff between $EXP and $(basename "$f$OUT_EXT") $(tput sgr0)"

  # Use git diff to give us color and word diffs. The patience algorithm
  # produces more readable diffs in some situations.
  git --no-pager diff --diff-algorithm=histogram --color=always \
    --word-diff=color --word-diff-regex='[a-zA-Z0-9_:;-]+' \
    $EXP "$f$OUT_EXT" | tail -n +5
  echo
  if [ "$NO_COPY" = true ]; then
    if [ "$TERM" = "dumb" ]; then
      read -r -p "$(tput bold)View next file? [Y/q] $(tput sgr0)"
    else
      read -p "$(tput bold)View next file? [Y/q] $(tput sgr0)" -n 1 -r
    fi
  elif [ "$TERM" = "dumb" ]; then
      read -r -p "$(tput bold)Copy output to expected output? [y/q/N] $(tput sgr0)"
  else
      read -p "$(tput bold)Copy output to expected output? [y/q/N] $(tput sgr0)" -n 1 -r
  fi
  echo ""
  if [ "$REPLY" = "y" ] && [ "$NO_COPY" = false ]; then
    cp "$f$OUT_EXT" "$f$EXP_EXT"
  elif [ "$REPLY" = "q" ]; then
    exit 0
  fi

  # A single blank line between loop iterations, even if the user hit enter.
  if [[ "$REPLY" =~ [a-zA-Z] ]]; then
    echo
  fi
done
