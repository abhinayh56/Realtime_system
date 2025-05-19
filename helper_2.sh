#!/bin/bash

echo -e "PID\tCPU\tCOMMAND\t\t\tTYPE"
echo "------------------------------------------------------------"

ps -eo pid,psr,comm --no-headers | grep -E "\s(4|5)$|\s(4|5)\s" | while read pid cpu comm; do
    if [ -e "/proc/$pid/exe" ] && readlink "/proc/$pid/exe" &> /dev/null; then
        type="User"
    else
        type="Kernel"
    fi

    printf "%-8s %-4s %-24s %s\n" "$pid" "$cpu" "$comm" "$type"
done
