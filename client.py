from socket import socket
import os
import re


host = input("Give host IP: ")
port = 3990

mysock = socket()
mysock.connect((host,port))
recv = mysock.recv(1024)

print(f"receieved: {recv.decode()}")

fileName = input("Enter the name and path of the file you want to send, ex: C:\\Users\\test.txt: \n")

file_data = open(fileName, 'rb')
data = file_data.read()
fileNameExtracted = re.search(r'[\\/]?([^\\/]+)$',fileName)
#print(data)
trueFileName = fileNameExtracted.group(1)
mysock.send(str(trueFileName).encode())
file_size = os.path.getsize(fileName)
print(f"[+]Sending {fileName} with a total of {file_size} bytes\n")
mysock.send(str(file_size).encode())
mysock.send(str(data).encode())
socket.close(mysock)
