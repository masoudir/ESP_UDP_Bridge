import socket

HOST = ''
PORT = 4585     # The Value is not important
BUFSIZE = 1024  # Maximum buffer size for receiving
ADDR = (HOST, PORT)

ESP_IP = "192.168.1.1"
ESP_PORT = 13585
ESP_ADDR = (ESP_IP, ESP_PORT)

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
client.bind(ADDR)


def receive():
    print("waiting for receiving from UDP port...")
    data, addr = client.recvfrom(BUFSIZE)
    if data != b"":
        print(data, addr)


def send():
    print("sending data to UDP port...")
    data = b"Hello, I am UDP message..."
    client.sendto(data, ESP_ADDR)
    
    
while True:
    send()
    receive()
    