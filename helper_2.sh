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


for i in /proc/irq/*/smp_affinity_list; do
    echo 0-1,4-5 | sudo tee "$i" > /dev/null
done

for cpu in /sys/devices/system/cpu/cpu[4-5]; do
    echo performance | sudo tee $cpu/cpufreq/scaling_governor
done

#!/bin/bash
# migrate all kernel threads away from CPUs 4 and 5

for pid in $(ps -eLo pid,psr,comm | awk '$3 ~ /\[.*\]/ { print $1 }'); do
    taskset -cp 0-1,4-5 "$pid" &> /dev/null
done


for irq in $(ls /proc/irq | grep -E '^[0-9]+$'); do
    echo 0-1,4-5 | sudo tee /proc/irq/$irq/smp_affinity_list > /dev/null
done
