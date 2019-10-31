Scenario 1:  
- open a terminal and execute: `./sighandler 2 0`.  
- open another terminal and find out the pid of the previous command.  
- execute `kill -2 $pid`. Check that `sighandler 2` is printed and program continues 
execution, i.e., read is restarted.  

Scenario 2:  
- open a terminal and execute: `./sighandler 2 1`.  
- open another terminal and find out the pid of the previous command.  
- execute `kill -2 $pid`. Check that `sighandler 2` is printed and program ends with 
`read: Interrupted system call` error.  
