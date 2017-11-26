
.PHONY: client server

server: 
	cc -o ftserver ftserver.c
	gcc ftserver.c
 
client: 
	chmod +x ftclient.py
	
