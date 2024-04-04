```
Usage: pi_test [OPTION...]
Test PTHREAD_PRIO_INHERIT for fair tasks by spawning a thread at low nice value
blocking another thread at higher nice value.

      --affine-cpu=NICE      CPU to affine the tasks to. Default 0.
      --hp-nice=NICE         Nice value of high priority thread to run at.
                             Default 0.
      --lp-nice=NICE         Nice value of low priority thread to run at.
                             Default 10.
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```
