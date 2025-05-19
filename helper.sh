for irq in $(ls -1 /proc/irq | grep -E '^[0-9]+$'); do
    echo 0-3,6-7 | sudo tee /proc/irq/$irq/smp_affinity_list > /dev/null 2>&1
done
