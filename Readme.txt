Command Lines:
  sender:
	gcc VideoSender.c -o send
	./send 1.mp4 15055
  receiver:
	gcc VideoReceiver.c -o recv
	./recv outfile.mp4 15055
