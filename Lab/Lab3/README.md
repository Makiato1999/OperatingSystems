
# Lab3
- student name:Xiaoran Xie
- student number:7884702
- how to compile all files: 'make'
- how to clean all obj files: 'make clean'
## spin-lock.c
- how to run: './spin-lock'
## atomic-lock.c
- how to run: './atomic-lock'
## lock-really.c
- how to run: './lock-really' (please be patient, this may take some time...)
## Questions
### how do the system calls change when you switch from flag-based spin locks to atomic variables to atomic instructions?
- Flag-based spin lock looks have same system calls as atomic variables. Also, atomic variables looks have same system calls to atomic instructions except system call 'futex' may have different times for each run. 
### Does pthread_mutex_lock use any system calls? 
- Yes, such as 'futex'.
### Before you start, make a prediction: will your spin lock be faster, or pthread_mutex_lock?
- I think that pthread_mutex_lock will be faster.
#### compare the time(for one experiment):
- lock-really: 0:17.79
- pthread_mutex_lock: 0:09.60
- So pthread_mutex_lock looks faster than lock-really in this experiment.
## condition-vars.c
- how to run: './condition-vars'
## Questions
### In this design, we don’t use signals to notify that the data has been used, only that it is available. What may happen if lots of data is given quickly?
- If we don't use signals, main thread will not notify lowercase thread when the input has been accepted. And when lots of data is given quickly, there will be too much data in buffer. Since we don't use signals, so lowercase thread will not be waked up as soon as possible when the input has been acccepted, and It may lose some data which in buffer.
