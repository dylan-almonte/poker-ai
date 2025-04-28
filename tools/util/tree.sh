#!/bin/bash

# Usage: ./tree.sh [directory]
# If no directory is given, use current directory

dir="${1:-.}"

print_tree() {
    local dir="$1"
    local prefix="$2"
    local entries=("$dir"/*)
    local count=${#entries[@]}
    local i=0

    for entry in "${entries[@]}"; do
        ((i++))
        local connector="├──"
        [ $i -eq $count ] && connector="└──"
        echo "${prefix}${connector} $(basename "$entry")"
        if [ -d "$entry" ]; then
            local new_prefix="$prefix"
            [ $i -eq $count ] && new_prefix+="    " || new_prefix+="│   "
            print_tree "$entry" "$new_prefix"
        fi
    done
}

echo "$(basename "$dir")"
print_tree "$dir" ""