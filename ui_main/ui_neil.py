from multiprocessing import Process
import tkinter as tk
from tkinter import messagebox
from PIL import Image, ImageTk
import numpy as np
import ui_virtual_mouse 


def parse_input(input_str):
    iterations = []
    current_clusters = []

    for line in input_str.strip().splitlines():
        line = line.strip()
        if line.startswith("ITERATION"):
            current_clusters = []
        elif line.startswith("END"):
            iterations.append(current_clusters)
        elif line:
            parts = line.split(':')
            centroid_part = parts[0].strip()
            points_part = parts[1].strip() if len(parts) > 1 else ''

            centroid_name, centroid_coords = centroid_part.split(' ', 1)
            centroid_x, centroid_y = map(float, centroid_coords.strip('()').split(','))

            points = []
            if points_part:
                for point_str in points_part.replace('(', '').split(')'):
                    if point_str.strip():
                        x, y = map(float, point_str.strip().split(','))
                        points.append((x, y))

            current_clusters.append({
                'name': centroid_name,
                'centroid': (centroid_x, centroid_y),
                'points': points
            })
    return iterations


def generate_visualization(clusters, width=640, height=480):
    img = np.ones((height, width, 3), dtype=np.uint8) * 255
    plot_width, plot_height = 400, 300
    plot_x = 120
    plot_y = 70

    x_min, x_max = 0, 20
    y_min, y_max = 0, 20

    def scale(x, y):
        px = plot_x + int((x - x_min) / (x_max - x_min) * plot_width)
        py = plot_y + plot_height - int((y - y_min) / (y_max - y_min) * plot_height)
        return px, py

    # Fixed colors: Red, Green, Blue
    colors = [
        [255, 0, 0],   # Red
        [0, 255, 0],   # Green
        [0, 0, 255]    # Blue
    ]

    for i, cluster in enumerate(clusters):
        color = colors[i % 3]
        for x, y in cluster['points']:
            px, py = scale(x, y)
            img[py-2:py+3, px-2:px+3] = color  # Color the data point
        cx, cy = cluster['centroid']
        px, py = scale(cx, cy)
        img[py-4:py+5, px-4:px+5] = color  # Color the centroid with the same color

    return img


def run_gui():
    root = tk.Tk()
    root.title("K-Means Visualization")
    root.geometry("640x480")
    root.resizable(False, False)

    # Use parameterized dimensions
    canvas = tk.Canvas(root, width=640, height=480)
    canvas.pack()

    # State variables
    num_centroids = 0 
    points = []  
    points_generated = False
    centroids_gen = False
    drawing = False
    last_x, last_y = None, None
    drawn_lines = []

    # Plot boundaries
    x_min, x_max = 0, 20
    y_min, y_max = 0, 20
    plot_x = 120
    plot_y = 70
    plot_width = 400
    plot_height = 300

    # Initialize background
    img = np.ones((480, 640, 3), dtype=np.uint8) * 255
    tk_img = ImageTk.PhotoImage(Image.fromarray(img))

    canvas.create_image(0, 0, anchor='nw', image=tk_img, tags='plot')
    canvas.image = tk_img

    def draw_axes():
        # X-axis
        canvas.create_line(120, 370, 520, 370, fill='black', tags='plot')
        # Y-axis
        canvas.create_line(120, 370, 120, 70, fill='black', tags='plot')
        #arrows
        canvas.create_line(520, 370, 515, 365, fill='black', tags='plot')
        canvas.create_line(520, 370, 515, 375, fill='black', tags='plot')
        canvas.create_line(120, 70, 115, 75, fill='black', tags='plot')
        canvas.create_line(120, 70, 125, 75, fill='black', tags='plot')

    # draws the clustersand points on canvas
    def draw_image():
        nonlocal img, tk_img, points_generated, centroids_gen
        if (not points_generated) or (not centroids_gen):
            messagebox.showwarning("Warning", "Please generate points and select initial centroids before starting.")
            return

        try:
            import socket
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect(('127.0.0.1', 9090))  # IP and port of PYNQ server
                s.sendall(b"START\n")

                for fname in ["points/clicked_points.txt", 'points/points.txt']:
                    with open(fname, 'rb') as f:
                        data = f.read()
                    s.sendall(f"{fname}\n{len(data)}\n".encode())
                    s.sendall(data)

                s.sendall(b"END\n")

                #wait for output.txt
                def recv_until(sock, terminator=b'\n'):
                    data = b''
                    while not data.endswith(terminator):
                        part = sock.recv(1)
                        if not part:
                            break
                        data += part
                    return data.strip()

                #receive confirmation
                header = recv_until(s).decode()
                if header != "output.txt":
                    raise ValueError("Unexpected file received")

                size = int(recv_until(s))
                output_data = b''
                while len(output_data) < size:
                    chunk = s.recv(min(4096, size - len(output_data)))
                    if not chunk:
                        break
                    output_data += chunk

                with open('output.txt', 'wb') as f:
                    f.write(output_data)

        except Exception as e:
            messagebox.showerror("Error", f"Failed to communicate with board:\n{e}")
            return

        with open('output.txt', 'r') as f:
            input_data = f.read()
        all_iterations = parse_input(input_data)

        animate(0, all_iterations)

   #animate iterations one by one
    def animate(i, all_iterations):
        nonlocal img, tk_img
        if i >= len(all_iterations):
            return
        pixel_array = generate_visualization(all_iterations[i])
        img = pixel_array.copy()
        tk_img = ImageTk.PhotoImage(Image.fromarray(img))
        canvas.delete('plot')
        canvas.create_image(0, 0, anchor='nw', image=tk_img, tags='plot')
        canvas.image = tk_img
        draw_axes()
        redraw_lines()
        root.after(0, lambda: animate(i + 1, all_iterations))  #wait 1 second between frames

    def reset_canvas():
        nonlocal img, tk_img, points_generated, num_centroids, centroids_gen, drawn_lines
        num_centroids = 0
        points_generated = False
        centroids_gen = False
        points.clear()
        open("points/points.txt", "w").close()
        img = np.ones((480, 640, 3), dtype=np.uint8) * 255
        tk_img = ImageTk.PhotoImage(Image.fromarray(img))
        open("points/clicked_points.txt", "w").close()
        canvas.delete('plot')
        canvas.delete('drawings')
        canvas.create_image(0, 0, anchor='nw', image=tk_img, tags='plot')
        canvas.image = tk_img
        draw_axes()
        drawn_lines.clear()


    # coordinate labels
    coord_label = tk.Label(root, text="Coordinates: ", font=("Arial", 10))
    coord_label.place(x=260, y=450)

    def get_centroids(event):
        nonlocal img, tk_img, points_generated, num_centroids, points, centroids_gen

        if 120 <= event.x <= 520 and 70 <= event.y <= 360:
            x = x_min + (event.x - plot_x) * (x_max - x_min) / plot_width
            y = y_min + (plot_y + plot_height - event.y) * (y_max - y_min) / plot_height
            px = int(plot_x + (x - x_min) / (x_max - x_min) * plot_width)
            py = int(plot_y + plot_height - (y - y_min) / (y_max - y_min) * plot_height)

            img[py - 4:py + 4, px - 4:px + 4] = [100, 100, 100]  # gray box
            num_centroids += 1
            if num_centroids == 3:
                centroids_gen = True

            if num_centroids > 3:
                with open("points/clicked_points.txt", "r") as file:
                    lines = file.readlines()

                if lines:
                    x_str, y_str = lines[0].strip().split()
                    x1 = float(x_str)
                    y1 = float(y_str)
                    px1 = int(plot_x + (x1 - x_min) / (x_max - x_min) * plot_width)
                    py1 = int(plot_y + plot_height - (y1 - y_min) / (y_max - y_min) * plot_height)
                    for i in range(5):
                        for j in range(5):
                            if np.all(img[py1 - i, px1 - j] == [100, 100, 100]):
                                img[py1 - i, px1 - j] = [255, 255, 255]
                            if np.all(img[py1 + i, px1 + j] == [100, 100, 100]):
                                img[py1 + i, px1 + j] = [255, 255, 255]
                            if np.all(img[py1 - i, px1 + j] == [100, 100, 100]):
                                img[py1 - i, px1 + j] = [255, 255, 255]
                            if np.all(img[py1 + i, px1 - j] == [100, 100, 100]):
                                img[py1 + i, px1 - j] = [255, 255, 255]
                    with open("points/clicked_points.txt", "w") as file:
                        file.writelines(lines[1:])  # remove first
                    with open("points/clicked_points.txt", "a") as file:
                        file.write(f"{x:.2f} {y:.2f}\n")  # append new
            else:
                with open("points/clicked_points.txt", "a") as file:
                    file.write(f"{x:.2f} {y:.2f}\n")

            with open("points/points.txt", "r") as file:
                lines2 = file.readlines()
            for i in range(100):
                x2_str, y2_str = lines2[i].strip().split()
                x2 = float(x2_str)
                y2 = float(y2_str)
                px2 = int(plot_x + (x2 - x_min) / (x_max - x_min) * plot_width)
                py2 = int(plot_y + plot_height - (y2 - y_min) / (y_max - y_min) * plot_height)
                points.append((x2, y2))
                # 5x5 black squares as points
                img[py2 - 2:py2 + 3, px2 - 2:px2 + 3] = [0,0,0]

            # Refresh canvas once
            tk_img = ImageTk.PhotoImage(Image.fromarray(img))
            canvas.create_image(0, 0, anchor='nw', image=tk_img, tags='plot')
            canvas.image = tk_img
            draw_axes()
            redraw_lines()


    def show_coordinates(event):
        if 120 <= event.x <= 520 and 70 <= event.y <= 370:
            x = x_min + (event.x - plot_x) * (x_max - x_min) / plot_width
            y = y_min + (plot_y + plot_height - event.y) * (y_max - y_min) / plot_height
            coord_label.config(text=f"Coordinates: ({x:.2f}, {y:.2f})")
        else:
            coord_label.config(text="Coordinates: ")

    canvas.bind('<Motion>', show_coordinates)

    def gen_points():
        nonlocal img, tk_img, points_generated, points, centroids_gen, num_centroids
        points_generated = True
        points.clear()
        open("points/clicked_points.txt", "w").close()
        centroids_gen = False
        num_centroids = 0

        img = np.ones((480, 640, 3), dtype=np.uint8) * 255  #white background

        num_clusters = 3
        points_per_cluster = 30  #3 clusters × 30 = 90
        num_outliers = 10
        std_dev = 1.5  #cluster spread
        min_centroid_distance = 7.0  #minimum distance between centroids

        #generate centroids
        centroids = []
        attempts = 0
        # attempts ensure that we dont get stuck in a loop
        # this while loop generates the random centres for data that are at least 8 distance apart
        while len(centroids) < num_clusters and attempts < 100:
            attempts += 1
            cx = np.random.uniform(x_min + 3, x_max - 3)
            cy = np.random.uniform(y_min + 3, y_max - 3)
            if all((cx - x)**2 + (cy - y)**2 >= min_centroid_distance**2 for x, y in centroids):
                centroids.append((cx, cy))

        # had to add this to ensure we always have 3 centroids
        if len(centroids) < num_clusters:
            while len(centroids) < num_clusters:
                cx = np.random.uniform(x_min + 3, x_max - 3)
                cy = np.random.uniform(y_min + 3, y_max - 3)
                centroids.append((cx, cy))

        #generate points
        for cx, cy in centroids:
            for _ in range(points_per_cluster):
                x = np.random.normal(cx, std_dev)
                y = np.random.normal(cy, std_dev)
                x = max(min(x, x_max), x_min)
                y = max(min(y, y_max), y_min)

                px = int(plot_x + (x - x_min) / (x_max - x_min) * plot_width)
                py = int(plot_y + plot_height - (y - y_min) / (y_max - y_min) * plot_height)

                points.append((x, y))
                img[py - 2:py + 3, px - 2:px + 3] = [0, 0, 0]  # black points

        #generate 10 random outliers
        for _ in range(num_outliers):
            x = np.random.uniform(x_min, x_max)
            y = np.random.uniform(y_min, y_max)

            px = int(plot_x + (x - x_min) / (x_max - x_min) * plot_width)
            py = int(plot_y + plot_height - (y - y_min) / (y_max - y_min) * plot_height)

            points.append((x, y))
            img[py - 2:py + 3, px - 2:px + 3] = [0,0,0] 

        #save to points
        with open('points/points.txt', 'w') as f:
            for point in points:
                f.write(f"{point[0]:.2f} {point[1]:.2f}\n")

        tk_img = ImageTk.PhotoImage(Image.fromarray(img))
        canvas.create_image(0, 0, anchor='nw', image=tk_img, tags='plot')
        canvas.image = tk_img
        draw_axes()
        redraw_lines()

    def start_drawing(event):
        global drawing, last_x, last_y
        if 120 <= event.x <= 520 and 70 <= event.y <= 370:
            drawing = True
            last_x, last_y = event.x, event.y

    def draw(event):
        global drawing, last_x, last_y
        if drawing and 120 <= event.x <= 520 and 70 <= event.y <= 370:
            # Draw on canvas
            canvas.create_line(last_x, last_y, event.x, event.y, fill="orange", width=2, tags='drawings')
            # Save the line segment
            drawn_lines.append((last_x, last_y, event.x, event.y))
            # Update last position
            last_x, last_y = event.x, event.y

    def redraw_lines():
        for x1, y1, x2, y2 in drawn_lines:
            canvas.create_line(x1, y1, x2, y2, fill="orange", width=2, tags='drawings')

    def stop_drawing(event):
        global drawing
        drawing = False

    def on_exit():
        open("points/clicked_points.txt", "w").close()
        open("points/points.txt", "w").close()
        open("output.txt", "w").close()
        root.destroy()  # This closes the window


    # Register the custom exit function
    root.protocol("WM_DELETE_WINDOW", on_exit)

    # buttons
    button_start = tk.Button(root, text="Start",width=15,height=1, command=draw_image)
    canvas.create_window(600, 440, anchor='se', window=button_start)

    button_reset = tk.Button(root, text="Reset",width=15,height=1, command=reset_canvas)
    canvas.create_window(40, 440, anchor='sw', window=button_reset)

    button_gen_points = tk.Button(root, text="Generate Points",width=15,height=1, command=gen_points)
    canvas.create_window(320, 440, anchor='s', window=button_gen_points)
    
    canvas.bind('<Button-1>', get_centroids)
    # Bind right-click events
    canvas.bind('<ButtonPress-3>', start_drawing)   # Right mouse button press
    canvas.bind('<B3-Motion>', draw)                # Mouse movement with right button held
    canvas.bind('<ButtonRelease-3>', stop_drawing)  # Right mouse button release


    draw_axes()
    root.mainloop()


def start_screen():
    start_root = tk.Tk()
    start_root.title("K-Means Clustering - Introduction")
    start_root.geometry("800x800")
    start_root.resizable(False, False)
    # Intro text
    intro_text = (
        "Welcome to the K-Means Clustering Visualizer!\n\n"
        "This tool helps you understand how the K-Means clustering algorithm works.\n\n"
        "To use the virtual mouse controller:\n\n"
        "1. Raise your hand to camera level.\n"
        "2. Point your palm to the screen and lift your thumb, index, and middle finger to use the cursor.\n"
        "3. Quickly move your index finger down and then up as if clicking a mouse to left click.\n"
        "4. Activate draw mode by bending your middle finger, then begin drawing.\n\n"
        "To run the K-Means clustering algorithm, follow these steps:\n\n"
        "1. Generate random points using the 'Generate Points' button.\n"
        "2. Click to place initial centroids on the graph (max 3 at a time).\n"
        "3. You can draw lines on the graph if you want to guess clusters.\n"
        "4. Click 'Start' to run the clustering algorithm implemented in C.\n"
        "5. Watch as the algorithm updates cluster assignments and centroid positions over iterations.\n\n"
        "Click 'Continue' to proceed to the interactive interface."
    )

    text_label = tk.Label(start_root, text=intro_text, wraplength=600, justify="left", font=("Arial", 11))
    text_label.pack(padx=10, pady=20)

    # Continue button
    continue_btn = tk.Button(start_root, text="Continue →", font=("Arial", 12),width=20,height=2,bg="white",command=lambda: [start_root.destroy(), run_gui()])
    continue_btn.pack(side='right', padx=20, pady=20)

    start_root.mainloop()




def main():
    try:
        # Start K-means GUI in a separate process
        kmeans_proc = Process(target=start_screen)
        kmeans_proc.start()

        controller = ui_virtual_mouse.VirtualMouse()
        # Start the virtual mouse gesture controller
        controller.run()

        # Wait for the K-means GUI to finish
        kmeans_proc.join()

    except KeyboardInterrupt:
        print("\nApplication interrupted by user")
    finally:
        print("Application closed")


if __name__ == "__main__":
    main()
