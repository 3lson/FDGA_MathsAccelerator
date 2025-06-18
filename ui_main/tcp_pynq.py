import socket
import threading
import os
import struct
from pynq import Overlay, MMIO

# Load bitstream
overlay = Overlay("/home/xilinx/jupyter_notebooks/design_1_wrapper.bit")

# MMIO setup
BRAM_BASE_ADDR = 0x40000000  
BRAM_SIZE = 0x10000  
mmio = MMIO(BRAM_BASE_ADDR, BRAM_SIZE)

def mmio_write(offset, value):
    mmio.write(offset, value)

def mmio_read(offset):
    return mmio.read(offset)

def read_float(offset):
    raw = mmio_read(offset)
    return struct.unpack('f', struct.pack('I', raw))[0]

# Constants
NUM_CENTROIDS = 3
NUM_POINTS = 100

def read_bram_data():
    offset = 1000

    centroids_x = [read_float(offset + i * 4) for i in range(NUM_CENTROIDS)]
    offset -= NUM_CENTROIDS * 4

    centroids_y = [read_float(offset + i * 4) for i in range(NUM_CENTROIDS)]
    offset -= NUM_CENTROIDS * 4

    cluster_labels = [int(read_float(offset + i * 4)) for i in range(NUM_POINTS)]
    offset -= NUM_POINTS * 4

    clusters_x = [read_float(offset + i * 4) for i in range(NUM_POINTS)]
    offset -= NUM_POINTS * 4

    clusters_y = [read_float(offset + i * 4) for i in range(NUM_POINTS)]
    offset -= NUM_POINTS * 4

    return centroids_x, centroids_y, cluster_labels, clusters_x, clusters_y

def write_clusters_to_txt(centroids_x, centroids_y, cluster_labels, clusters_x, clusters_y, filename="clusters.txt"):
    clusters = {0: [], 1: [], 2: []}
    for i in range(NUM_POINTS):
        cid = cluster_labels[i]
        clusters[cid].append((clusters_x[i], clusters_y[i]))

    with open(filename, "w") as f:
        for i in range(NUM_CENTROIDS):
            f.write(f"C{i} ({centroids_x[i]:.2f}, {centroids_y[i]:.2f}):")
            for pt in clusters[i]:
                f.write(f" ({pt[0]:.2f}, {pt[1]:.2f})")
            f.write("\n")

def handle_client(conn, addr):
    print(f'Connected by {addr}')
    try:
        cmd = recv_line(conn)
        if cmd != "START":
            print("Expected START")
            return

        save_targets = ["clicked_points.txt", "points.txt"]
        base_offsets = [0x3268, 0x16000]
        y_val_points = []
        x_val_points = [] 
        y_val_centroids = []
        x_val_centroids = [] 

        for idx, expected_name in enumerate(save_targets):
            received_filename = recv_line(conn)
            size = int(recv_line(conn))
            data = recv_exact(conn, size)

            with open(expected_name, "wb") as f:
                f.write(data)
            print(f"Saved {received_filename} as {expected_name} ({size} bytes)")

            # Decode and write to MMIO
            lines = data.decode().splitlines()
            offset = base_offsets[idx]
            for line in lines:
                parts = line.strip().split()
                if len(parts) != 2:
                    continue  # skip bad lines
                x = float(parts[0])
                y = float(parts[1])

                # Store coordinates
                if expected_name == "clicked_points.txt":
                    x_val_centroids.append(x)
                    y_val_centroids.append(y)
                elif expected_name == "points.txt":
                    x_val_points.append(x)
                    y_val_points.append(y)

            for i in x_val_centroids:
                x_raw = struct.unpack("I", struct.pack("f", i))[0]
                mmio_write(offset, x_raw)
                offset -= 4
            for i in y_val_centroids:
                y_raw = struct.unpack("I", struct.pack("f", i))[0]
                mmio_write(offset, y_raw)
                offset -= 4
            for i in x_val_points:
                x_raw = struct.unpack("I", struct.pack("f", i))[0]
                mmio_write(offset, x_raw)
                offset -= 4
            for i in y_val_points:
                y_raw = struct.unpack("I", struct.pack("f", i))[0]
                mmio_write(offset, y_raw)
                offset -= 4
            
        end_cmd = recv_line(conn)
        if end_cmd != "END":
            print("Expected END")
            return

        # Read output from BRAM and write to file
        centroids_x, centroids_y, cluster_labels, clusters_x, clusters_y = read_bram_data()
        write_clusters_to_txt(centroids_x, centroids_y, cluster_labels, clusters_x, clusters_y)

        # Send file back to client
        with open("output.txt", "rb") as f:
            output_bytes = f.read()

        conn.sendall(b"output.txt\n")
        conn.sendall(f"{len(output_bytes)}\n".encode())
        conn.sendall(output_bytes)
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

# Start the server
threading.Thread(target=run_server, daemon=True).start()
print("TCP server started and listening...")
