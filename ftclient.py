#!/bin/python


####################################################################
# Alieta Train 
# CS 372 
# Oregon State Univeristy 
# November 2017 
# ftclient written in python
# starts on Host B and validates any pertinent command line parameters
# <SERVER_HOST> <SERVER_PORT> <COMMAND> <FILENAME> <DATA_PORT>
#
# establish TCP control connection on <SERVER_PORT> 'connection P'
# sends a command (-1(list) or -g <FILENAME> (get)) on connection p 
#
# resources: https://docs.python.org/2/library/socket.html
# resources:  
####################################################################


from socket import *
import sys

# check args, make sure we are on appropriate flip server, 
# ensure valid port, and validate option selection and valid port
def validate_inputs():
	if len(sys.argv) > 6 or len(sys.argv) < 5:
		print "Usage: check readme for usage."
		exit(1)
	elif (sys.argv[1] != "flip1" and sys.argv[1] != "flip2" and sys.argv[1] != "flip3"):
		print "Must use either flip1, flip2, or flip3."
		exit(1)
	elif (sys.argv[3] == "-l" and (int(sys.argv[4]) > 65000 or int(sys.argv[4]) < 40000)):
		print "Invalid control port number (usage: 40000 - 65000)"
		exit(1)
	elif (sys.argv[3] == "-g" and (int(sys.argv[5]) > 65000 or int(sys.argv[5]) < 40000)):
		print "Invalid control port number (usage: 40000 - 65000)"
		exit(1)
	elif (int(sys.argv[2]) > 65000 or int(sys.argv[2]) < 40000):
		print "Please choose a port between 40000 - 65000."
		exit(1)
	elif (sys.argv[3] != "-l" and sys.argv[3] != "-g"):
		print "Usage: refer to readme for usage."
		exit(1)

# returns connects to a socket 
# https://pymotw.com/2/socket/tcp.html
def connection():
	serv_addr = sys.argv[1]+".engr.oregonstate.edu"
	serv_port = int(sys.argv[2])
	sock_c = socket(AF_INET,SOCK_STREAM)
	sock_c.connect((serv_addr, serv_port))
	return sock_c

# Port arg is five unless there are 5 args. 
# make socket for server and start accepting connections on socket
def make_data_socket():
	if sys.argv[3] == "-g":
		portarg = 5
	elif sys.argv[3] == "-l":
		portarg = 4
	serv_port = int(sys.argv[portarg])
	sock_s = socket(AF_INET, SOCK_STREAM)
	sock_s.bind(('', serv_port))
	sock_s.listen(1)
	sock_d, addr = sock_s.accept()	
	return sock_d

# Grab file list and go through filenames
def get_file_list(sock_d):
	filename = sock_d.recv(100)
	while filename != "done":
		print filename
		filename = sock_d.recv(100)

# opens files for writing, receives buffered input until runs out of files
def get_file(sock_d):
	f = open(sys.argv[4],"w")
	buff = sock_d.recv(1000)
	while "__done__" not in buff:
		f.write(buff)
		buff = sock_d.recv(1000)

# helper function, gets ip address 
# https://docs.python.org/2/library/socket.html#socket.getsockname
def self_ip():
	s = socket(AF_INET, SOCK_DGRAM)
	s.connect(("8.8.8.8", 80))	
	return s.getsockname()[0]

# port argument is 5 argument if there are 5 arguments 
# and port argument is 4th argument if there are 4 arguments
# program execution statements
# https://docs.python.org/2/library/socket.html
def exchange_requests(sock_c):
	if sys.argv[3] == "-g":
		print "Info: reqesting file {}".format(sys.argv[4])
		portarg = 5
	elif sys.argv[3] == "-l":
		print "Info: requesting server file list"
		portarg = 4
	sock_c.send(sys.argv[portarg])
	sock_c.recv(1024)
	if sys.argv[3] == "-l":
		sock_c.send("l")
	else:
		sock_c.send("g")
	sock_c.recv(1024)
	sock_c.send(self_ip())
	response = sock_c.recv(1024)
	if response == "bad":
		print "Error: server received a bad command"
		exit(1)
	if sys.argv[3] == "-g":
		sock_c.send(sys.argv[4])
		response = sock_c.recv(1024)
		if response != "File found":
			print "Error: client response 'File not found message'"
			return
	sock_d = make_data_socket()
	if sys.argv[3] == "-l":
		get_file_list(sock_d)
	elif(sys.argv[3] == "-g"):
		get_file(sock_d)
	sock_d.close()


# main 
# checks arguments, calls functions to create socket 
# talk to server 
if __name__ == "__main__":
    validate_inputs()
    sock_c = connection()
    exchange_requests(sock_c)
