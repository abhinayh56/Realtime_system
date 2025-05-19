#!/bin/bash

echo "==================== SMT Siblings (Thread Pairs) ===================="
lscpu -p=cpu,node,socket,core | grep -v '^#' | sort -k4,4n | awk -F',' '
{
  coreid = $4;
  cpu[$4] = cpu[$4] ? cpu[$4] "," $1 : $1;
}
END {
  for (c in cpu) {
    split(cpu[c], arr, ",");
    if (length(arr) > 1) {
      print "Core " c ": CPUs " cpu[c] " (SMT siblings)";
    }
  }
}'

echo
echo "==================== Recommended CPUs for Real-Time ================="
echo "Physical (non-SMT sibling) cores are safest to use for RT threads:"
lscpu -p=cpu,node,socket,core | grep -v '^#' | sort -k4,4n | awk -F',' '
{
  coreid = $4;
  cpus[coreid]++;
  if (!seen_core[coreid]) {
    core_cpu[coreid] = $1;
    seen_core[coreid] = 1;
  }
}
END {
  for (c in core_cpu) {
    if (cpus[c] == 1) {
      print "Use CPU " core_cpu[c] " (sole thread on core " c ")";
    }
  }
}'

echo
echo "==================== IRQs per CPU (from /proc/interrupts) ============"
cpus=$(lscpu | grep '^CPU(s):' | awk '{print $2}')
cpulist=$(seq 0 $((cpus - 1)))
header="IRQ Source         "
for c in $cpulist; do
    header="$header CPU$c"
done
echo "$header"
echo "-----------------------------------------------------------------------"
awk -v cpus=$cpus '
{
  if ($1 ~ /^[0-9]+:/) {
    irq = substr($1, 1, length($1)-1)
    printf "%-18s", $NF
    for (i = 2; i <= cpus+1; i++) {
      printf " %5s", $i
    }
    printf "\n"
  }
}
' /proc/interrupts
