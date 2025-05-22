# g++ -O2 -o latency_test latency_jitter_test.cpp -pthread
# g++ -o rt_test latency_jitter_test.cpp -pthread

# echo "---0"
# sudo ./rt_test 0
# echo "---1"
# sudo ./rt_test 1
# echo "---2"
# sudo ./rt_test 2
# echo "---3"
# sudo ./rt_test 3
# echo "---4"
# sudo ./rt_test 4
# echo "---5"
# sudo ./rt_test 5
# echo "---"

echo "---0"
sudo taskset -c 0 cyclictest -p 85 -t5 -i 1000 -l 10000
echo "---1"
sudo taskset -c 1 cyclictest -p 85 -t5 -i 1000 -l 10000
echo "---2"
sudo taskset -c 2 cyclictest -p 85 -t5 -i 1000 -l 10000
echo "---3"
sudo taskset -c 3 cyclictest -p 85 -t5 -i 1000 -l 10000
echo "---4"
sudo taskset -c 4 cyclictest -p 85 -t5 -i 1000 -l 10000
echo "---5"
sudo taskset -c 5 cyclictest -p 85 -t5 -i 1000 -l 10000
echo "---"

# clocksource=tsc tsc=reliable nmi_watchdog=0 nosoftlockup isolcpus=4,5 rcu_nocbs=4,5 nohz_full=4,5 irqaffinity=0,1,2,3

# GRUB_CMDLINE_LINUX_DEFAULT="quiet splash clocksource=tsc tsc=reliable nmi_watchdog=0 nosoftlockup isolcpus=4,5 rcu_nocbs=4,5 nohz_full=4,5 irqaffinity=0,1,2,3"