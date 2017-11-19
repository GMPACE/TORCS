# TCP server example
import socket
import time

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(("", 9998))
server_socket.listen(5)

print("TCPServer Waiting for client on port 9998")

while 1:
    client_socket, address = server_socket.accept()
    print("I got a connection from ", address)
    while 1:
        time.sleep(0.05)
        data = "3/"
        client_socket.send(data.encode())

        if (data == 'q' or data == 'Q'):
            client_socket.close()
            break
    break
server_socket.close()
print("SOCKET closed... END")
