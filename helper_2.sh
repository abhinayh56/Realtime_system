#!/bin/bash

echo -e "PID\tCPU\tCOMMAND\t\t\tTYPE"
echo "------------------------------------------------------------"

# Get processes on CPU 4 or 5
ps -eo pid,psr,comm --no-headers | grep -E "\s(4|5)$|\s(4|5)\s" | while read pid cpu comm; do
    # Check if the /proc/[pid]/exe symlink exists
    if [ -L /proc/$pid/exe ]; then
        # User-space process
        type="User"
    else
        # Kernel thread
        type="Kernel"
    fi

    printf "%-8s %-4s %-24s %s\n" "$pid" "$cpu" "$comm" "$type"
done
