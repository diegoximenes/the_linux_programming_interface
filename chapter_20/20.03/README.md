Testing sa_resethand scenario 1:  
- open a terminal and execute: `./sa_resethand 0`.  
- open another terminal and find out that pid of the previous command.  
- execute `kill -s SIGINT $pid`. Check that `sighandler 2` is printed and program continues execution.  
- execute `kill -s SIGINT $pid`. Check that `sighandler 2` is printed and program continues execution.  

Testing sa_resethand scenario 2:  
- open a terminal and execute: `./sa_resethand 1`.  
- open another terminal and find out that pid of the previous command.  
- execute `kill -s SIGINT $pid`. Check that `sighandler 2` is printed and program continues execution.  
- execute `kill -s SIGINT $pid`. Check that program ends.  

Testing sa_nodefer scenario 1:  
- open a terminal and execute: `./sa_nodefer 0`.  
- open another terminal and find out the pid of the previous command.  
- execute `kill -s SIGINT $pid`. Check that `sighandler 2` is printed and program continues execution.  
- execute `kill -s SIGINT $pid`. Check that nothing is printed and program continues execution.  

Testing sa_nodefer scenario 2:  
- open a terminal and execute: `./sa_nodefer 1`.  
- open another terminal and find out the pid of the previous command.  
- execute `kill -s SIGINT $pid`. Check that `sighandler 2` is printed and program continues execution.  
- execute `kill -s SIGINT $pid`. Check that `sighandler 2` is printed and program continues execution.  
