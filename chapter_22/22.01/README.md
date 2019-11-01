Scenario 1:  
- open a terminal and execute: `./sigcont 0`. Stop the process: press Ctrl-z.  
- open another terminal and find out the pid of the previous command. 
Check that the process `STAT`'s is `T` using ps.  
- execute `kill -s SIGCONT $pid`. 
Check that the process prints `sighandler 18`. 
Check that the process `STAT`'s is `R` using ps.  
- execute `kill -s SIGCONT $pid`. Check that the process prints `sighandler 18`.  

Scenario 2:  
- open a terminal and execute: `./sigcont 1`. Stop the process: press Ctrl-z.  
- open another terminal and find out the pid of the previous command. 
Check that the process `STAT`'s is `T` using ps.  
- execute `kill -s SIGCONT $pid`. Check that the process `STAT`'s is `R` using ps. 
- execute `kill -s SIGCONT $pid`. Check that the process prints nothing.  
- execute `kill -s SIGCONT $pid`. Check that the process prints nothing.  
