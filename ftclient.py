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
####################################################################


from socket import *
import sys

# check args, make sure we are on appropriate flip server, 
# ensure valid port, and validate option selection and valid port
def check_args():
    if len(sys.argv) < 5 or len(sys.argv) > 6:
        print "Invalid number of args"
        exit(1)
    elif (sys.argv[1] != "flip1" and sys.argv[1] != "flip2" and sys.argv[1] != "flip3"):
        print "Invalid server name"
        exit(1)
    elif (int(sys.argv[2]) > 65535 or int(sys.argv[2]) < 1024):
        print "Invalid control port number"
        exit(1)
    elif (sys.argv[3] != "-l" and sys.argv[3] != "-g"):
        print "Invalid option"
        exit(1)
    elif (sys.argv[3] == "-l" and (int(sys.argv[4]) > 65535 or int(sys.argv[4]) < 1024)):
        print "Invalid control port number"
        exit(1)
    elif (sys.argv[3] == "-g" and (int(sys.argv[5]) > 65535 or int(sys.argv[5]) < 1024)):
        print "Invalid control port number"
        exit(1)

# Port arg is five unless there are 5 args. 
# make socket for server and start accepting connections on socket
# 
def setup_data_socket():
    if sys.argv[3] == "-l":
        portarg = 4
    elif sys.argv[3] == "-g":
        portarg = 5
    serverport = int(sys.argv[portarg])
    serversocket = socket(AF_INET, SOCK_STREAM)
    serversocket.bind(('', serverport))
    serversocket.listen(1)
    data_socket, addr = serversocket.accept()
    return data_socket

# Grab file list and go through filenames
def get_file_list(data_socket):
    filename = data_socket.recv(100)
    while filename != "done":
        print filename
        filename = data_socket.recv(100)
# opens files for writing, receives buffered input until runs out of files
def get_file(data_socket):
    f = open(sys.argv[4],"w")
    buff = data_socket.recv(1000)
    while "__done__" not in buff:
        f.write(buff)
        buff = data_socket.recv(1000)

# helper function, gets ip address 
# http://stackoverflow.com/questions/24196932/how-can-i-get-the-ip-address-of-eth0-in-python
def get_my_ip():
    s = socket(AF_INET, SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    return s.getsockname()[0]

# portnum is 5 unless there are five arguments, then it's 4. 
# program execution statements
def exchange_information(clientsocket):
    if sys.argv[3] == "-l":
        print "Requesting file list"
        portnum = 4
    elif sys.argv[3] == "-g":
        print "Reqesting file {}".format(sys.argv[4])
        portnum = 5
    clientsocket.send(sys.argv[portnum])
    clientsocket.recv(1024)
    if sys.argv[3] == "-l":
        clientsocket.send("l")
    else:
        clientsocket.send("g")
    clientsocket.recv(1024)
    clientsocket.send(get_my_ip())
    response = clientsocket.recv(1024)
    if response == "bad":
        print "Server received an invalid command"
        exit(1)
    if sys.argv[3] == "-g":
        clientsocket.send(sys.argv[4])
        response = clientsocket.recv(1024)
        if response != "File found":
            print "Client responded with 'File not found message'"
            return
    data_socket = setup_data_socket()
    if sys.argv[3] == "-l":
        get_file_list(data_socket)
    elif(sys.argv[3] == "-g"):
        get_file(data_socket)
    data_socket.close()


# connects to a socket 
def connect_to_server():
    servername = sys.argv[1]+".engr.oregonstate.edu"
    serverport = int(sys.argv[2])
    clientsocket = socket(AF_INET,SOCK_STREAM)
    clientsocket.connect((servername, serverport))
    return clientsocket

# main 
# checks arguments, calls functions to create socket 
# talk to server 
if __name__ == "__main__":
    check_args()
    clientsocket = connect_to_server()
    exchange_information(clientsocket)