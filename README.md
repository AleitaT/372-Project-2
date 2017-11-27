# 372-Project-2

This is a file transfer program which is able to excahnge files between two directories in seperate servers.  

## Before you begin

You must run the client and server on a different flip server, for example, one on flip 2 and one on flip 3 would be fine.  

The program only accepts port numbers within the range of 40000 to 65000

If you are experiencing difficulties,  you should also place the running programs into different directories within the different servers. 

## SERVER

### build server file first  

		make server 

### run server 

		./ftserver <portnum>

## CLIENT

### build client files first 

		make client

### run client 

		python ftclient.py <serverhost> <server_port> -g <filetoget> <data_port>
		python ftclient.py <serverhost> <server_port> -l <data_port>

Example: 

		$ python ftclient.py flip3 43678 -g filename.txt 43679
		$ python ftclient.py flip2 49765 -l 49766
