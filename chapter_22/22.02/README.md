- open a terminal and execute: `./signal_order`.  
- open another terminal and find out the pid of the previous command.  
- execute:  
kill -2 $pid  
kill -2 $pid  
kill -60 $pid  
kill -60 $pid  
kill -60 $pid  
- Check that the program prints:  
before sleep  
realtime sighandler 60  
realtime sighandler 60  
realtime sighandler 60  
default sighandler 2  
- realtime signal handlers are always executed before standard signal handlers  
