from socket import socket
import os
import re

mysock = socket()
mysock.connect((input("Give host IP: "),3990))
recv = mysock.recv(1024)

print(f"receieved: {recv.decode()}")

fileName = input("Enter the name and path of the file you want to send, ex: C:\\Users\\test.txt: \n")
file_data = open(fileName, 'rb')
data = file_data.read()
fileNameExtracted = re.search(r'[\\/]?([^\\/]+)$',fileName)
trueFileName = fileNameExtracted.group(1)
mysock.send(str(trueFileName).encode())
file_size = os.path.getsize(fileName)
print(f"[+]Sending {fileName} with a total of {file_size} bytes\n")
mysock.send(str(file_size).encode())
mysock.send(data)
socket.close(mysock)
