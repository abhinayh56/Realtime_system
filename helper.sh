#!/bin/bash
# Move all IRQs off CPUs 4 and 5

for irq in $(ls /proc/irq/*/smp_affinity_list | grep -oP '\d+'); do
    echo 0-3,6-7 | sudo tee /proc/irq/$irq/smp_affinity_list > /dev/null
done
