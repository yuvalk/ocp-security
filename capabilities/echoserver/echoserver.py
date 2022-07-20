#!/usr/bin/env python3

# Echo server 
import socket

HOST = ''    # all available interfaces
PORT = 5005  # any port > 1023
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)  # 1 means the number of accepted connections
while True:
    conn, addr = s.accept()  # waits for a new connection
    print('Client connected: ', addr)
    while True:
        data = conn.recv(1024)
        if not data:
            print('disconnecting')
            break
        print('Client sent: ', data)
        conn.sendall(data)
    conn.close()
