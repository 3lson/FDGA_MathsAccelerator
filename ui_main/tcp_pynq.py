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
        #wait for START command
        cmd = recv_line(conn)
        if cmd != "START":
            print("Expected START")
            return

        #expected files to receive
        save_targets = ["out/clicked_points.txt", "out/points.txt"]
        for expected_name in save_targets:
            received_filename = recv_line(conn)
            size = int(recv_line(conn))
            data = recv_exact(conn, size)

            os.makedirs(os.path.dirname(expected_name), exist_ok=True)
            with open(expected_name, "wb") as f:
                f.write(data)

            print(f"Saved {received_filename} as {expected_name} ({size} bytes)")

        #wait for END
        end_cmd = recv_line(conn)
        if end_cmd != "END":
            print("Expected END")
            return

        subprocess.run(['gcc', 'k_means3.c', '-o', 'k_means3'], capture_output=True, text=True)
        subprocess.run(['./k_means3'], capture_output=True, text=True)

        output_path = "out/output.txt"
        if not os.path.exists(output_path):
            print("output.txt not generated.")
            return

        with open(output_path, "rb") as f:
            output_data = f.read()

        conn.sendall(b"output.txt\n")
        conn.sendall(f"{len(output_data)}\n".encode())
        conn.sendall(output_data)
        print("Sent output.txt")

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
