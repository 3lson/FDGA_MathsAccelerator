import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider

# Parameters
width, height = 400, 400
x_range = (-2, 2)
y_range = (-2, 2)

# Grid setup
x = np.linspace(*x_range, width)
y = np.linspace(*y_range, height)
xx, yy = np.meshgrid(x, y)
grid_points = np.stack([xx.ravel(), yy.ravel()], axis=-1)

# Compute cluster assignment
def assign_clusters(points, centroids):
    dists = np.linalg.norm(points[:, None, :] - centroids[None, :, :], axis=2)
    return np.argmin(dists, axis=1)

# Initial centroids
init_centroids = np.array([
    [-1.0, -1.0],
    [1.0, -1.0],
    [0.0, 1.5]
])

fig, ax = plt.subplots()
plt.subplots_adjust(left=0.25, bottom=0.35)
fig.canvas.manager.set_window_title('K-means Cluster Assignment Visualisation')
ax.set_title('Interactive K-means Clustering')
ax.set_xlabel('X-axis')
ax.set_ylabel('Y-axis')

labels = assign_clusters(grid_points, init_centroids).reshape((height, width))
im = ax.imshow(labels, origin='lower', extent=(*x_range, *y_range), cmap='viridis', alpha=0.8)
sc = ax.scatter(init_centroids[:, 0], init_centroids[:, 1], c='red', s=100, marker='x')

# Sliders
axcolor = 'lightgoldenrodyellow'
slider_axes = []
sliders = []
for i in range(3):
    for j, axis_label in enumerate(["X", "Y"]):
        ax_slider = plt.axes([0.25, 0.25 - 0.05 * (i * 2 + j), 0.65, 0.03], facecolor=axcolor)
        slider = Slider(
            ax=ax_slider,
            label=f'C{i+1}_{axis_label}',
            valmin=-2,
            valmax=2,
            valinit=init_centroids[i, j]
        )
        sliders.append(slider)

# Update function
def update(val):
    new_centroids = np.array([
        [sliders[0].val, sliders[1].val],
        [sliders[2].val, sliders[3].val],
        [sliders[4].val, sliders[5].val]
    ])
    labels = assign_clusters(grid_points, new_centroids).reshape((height, width))
    im.set_data(labels)
    sc.set_offsets(new_centroids)
    fig.canvas.draw_idle()

for slider in sliders:
    slider.on_changed(update)

plt.show()
