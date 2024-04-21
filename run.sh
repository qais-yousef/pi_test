#!/bin/bash
set -eux

sa=$(which sched-analyzer)
if [ -n $sa ]; then
	sudo $sa &
fi

sudo prlimit -e=40 --pid=$$

./pi_test --lp-nice ${1:-10} --hp-1-nice ${2:-0} --hp-2-nice ${3-0} --affine-cpu ${4:-0} &

sleep 10

pkill -SIGKILL pi_test

if [ -n $sa ]; then
	sudo pkill -SIGINT sched-analyzer
fi

sa_pp=$(which sched-analyzer-pp)
if [ -n $sa_pp ]; then
	$sa_pp --sched-states pi_test sched-analyzer.perfetto-trace
fi
