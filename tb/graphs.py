import numpy as np
import plotly.graph_objects as go
from plotly.subplots import make_subplots

# Generate datasets
# Gaussian data
gaussian_data = np.random.normal(loc=125, scale=25, size=10000)
gaussian_hist, gaussian_bins = np.histogram(gaussian_data, bins=100)
gaussian_x = (gaussian_bins[:-1] + gaussian_bins[1:]) / 2

# Sine data
x = np.linspace(0, 250, 1000)
sine_data = (np.sin(2 * np.pi * x / 50) + 1) * 100

# Triangle data
triangle_data = np.abs(np.arange(0, 250, 1) % 100 - 50)

# Noisy data
noisy_data = sine_data + np.random.normal(loc=0, scale=20, size=len(sine_data))
noisy_hist, noisy_bins = np.histogram(noisy_data, bins=100)
noisy_x = (noisy_bins[:-1] + noisy_bins[1:]) / 2

# Create subplots
fig = make_subplots(
    rows=2, cols=2,
    subplot_titles=["Gaussian", "Sine", "Triangle", "Noisy"],
    horizontal_spacing=0.2, vertical_spacing=0.3
)

# Add Gaussian line plot
fig.add_trace(
    go.Scatter(x=gaussian_x, y=gaussian_hist, mode='lines', name='Gaussian'),
    row=1, col=1
)

# Add Sine line plot
fig.add_trace(
    go.Scatter(x=np.arange(len(sine_data)), y=sine_data, mode='lines', name='Sine'),
    row=1, col=2
)

# Add Triangle line plot
fig.add_trace(
    go.Scatter(x=np.arange(len(triangle_data)), y=triangle_data, mode='lines', name='Triangle'),
    row=2, col=1
)

# Add Noisy line plot
fig.add_trace(
    go.Scatter(x=noisy_x, y=noisy_hist, mode='lines', name='Noisy'),
    row=2, col=2
)

# Update layout
fig.update_layout(
    title="Line Graphs for Different Datasets",
    height=800, width=900,
    showlegend=False
)

# Show figure
fig.show()
