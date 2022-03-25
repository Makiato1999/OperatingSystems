
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
- how to run: './lock-really'
## Questions
### how do the system calls change when you switch from flag-based spin locks to atomic variables to atomic instructions?
- Flag-based spin lock looks have same system calls as atomic variables. Also, atomic variables looks have same system calls to atomic instructions except system call 'futex' may have different times for each run. 
### Does pthread_mutex_lock use any system calls? 
- Yes, such as 'futex'.
### Before you start, make a prediction: will your spin lock be faster, or pthread_mutex_lock?
- I think that spin lock will be faster.
#### compare the time(for one experiment):
- spin-lock: 0:01.41
- pthread_mutex_lock: 0:09.60
- So spin-lock looks faster than pthread_mutex_lock in this experiment.
## condition-vars.c
- how to run: './condition-vars'
## Questions
### In this design, we don’t use signals to notify that the data has been used, only that it is available. What may happen if lots of data is given quickly?
- We don’t use signals to notify that the data has been used, so that the uppercase thread does not know whether data is used in lowercase thread. So in lowercase thread, it may accepts data failed or processes data failed. However, if pthread_mutex_signal works successed, uppercase thread can still be waked up and works. There is no error will be thrown, so that this is not safe.
