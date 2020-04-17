# How to run
  
## sender:
./obj_file [ file_name ] [ port_no ]
### used commands
  	gcc VideoSender.c -o send
	./send 1.mp4 15055

## receiver: 
./obj_file [ file_name ] [ port_no ]
### used command
   gcc VideoReceiver.c -o recv
   ./recv outfile.mp4 15055
