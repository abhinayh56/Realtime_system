#!/bin/bash
for cpu in 4 5; do
  for pid in $(ps -eLo pid,psr | awk -v c=$cpu '$2 == c {print $1}' | sort -u); do
    taskset -cp 0-3 $pid >/dev/null 2>&1
  done
done


grep -E 'CPU4|CPU5' /proc/interrupts
