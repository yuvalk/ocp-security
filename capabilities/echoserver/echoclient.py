#!/usr/bin/env python3

# Echo client
import socket

HOST = ''    # all available interfaces
PORT = 5005  # any port > 1023
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
print('Client connected.')
while True:
    user_input = str(input())  # convert input to string
    if not user_input:  # i.e. enter key pressed
        break
    s.sendall(user_input.encode())  # encode needed to transform string to bytes
    data = s.recv(1024)
    if not data:
        break
    print("Data received from server: ", data.decode('utf-8'))  # decode needed to convert data to string

