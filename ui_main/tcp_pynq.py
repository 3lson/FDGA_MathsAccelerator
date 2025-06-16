import socket
import threading
import os
import subprocess
from pynq import Overlay, MMIO
#change that part
overlay = Overlay("")

# You can confirm with: print(overlay.ip_dict)
#chnage that part
BRAM_BASE_ADDR = 0x40000000  
BRAM_SIZE = 0x10000  

# Create MMIO object
mmio = MMIO(BRAM_BASE_ADDR, BRAM_SIZE)



def mmio_write(offset, value):
    mmio.write(offset, value)

def mmio_read(offset):
    return mmio.read(offset)


def handle_client(conn, addr):
    print(f'Connected by {addr}')
    try:
        cmd = recv_line(conn)
        if cmd != "START":
            print("Expected START")
            return

        save_targets = ["clicked_points.txt", "points.txt"]
        base_offsets = [0x8000, 0x16000]  # split memory regions for the two files

        for idx, expected_name in enumerate(save_targets):
            received_filename = recv_line(conn)
            size = int(recv_line(conn))
            data = recv_exact(conn, size)

            with open(expected_name, "wb") as f:
                f.write(data)
            print(f"Saved {received_filename} as {expected_name} ({size} bytes)")

            #decode and parse the file into integers for MMIO
            lines = data.decode().splitlines()
            offset = base_offsets[idx]
            for line in lines:
                parts = line.strip().split()
                for part in parts:
                    value = int(float(part))
                    mmio_write(offset, value)
                    offset += 4  # next 4-byte slot

        end_cmd = recv_line(conn)
        if end_cmd != "END":
            print("Expected END")
            return

        #read output from BRAM starting at address 0x0000
        output_lines = []
        offset = 0x0000
        max_lines = 1000 #adjust later

        for _ in range(max_lines):
            value = mmio_read(offset)
            if value == 0xFFFFFFFF:
                break
            output_lines.append(str(value))
            offset += 4

        output_str = "\n".join(output_lines) + "\n"
        output_bytes = output_str.encode()

        conn.sendall(b"output.txt\n")
        conn.sendall(f"{len(output_bytes)}\n".encode())
        conn.sendall(output_bytes)
        print("Sent output.txt from BRAM")

    except Exception as e:
        print(f"Error: {e}")
    finally:
        conn.close()



def recv_line(sock):
    data = b""
    while not data.endswith(b"\n"):
        part = sock.recv(1)
        if not part:
            break
        data += part
    return data.decode().strip()

def recv_exact(sock, size):
    data = b""
    while len(data) < size:
        part = sock.recv(min(4096, size - len(data)))
        if not part:
            break
        data += part
    return data


def run_server():
    HOST = '192.168.2.2' 
    PORT = 9090

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen(1)
        print(f"Server listening on {HOST}:{PORT}")
        while True:
            conn, addr = s.accept()
            thread = threading.Thread(target=handle_client, args=(conn, addr))
            thread.start()


threading.Thread(target=run_server, daemon=True).start()
print("TCP server started and listening...")
