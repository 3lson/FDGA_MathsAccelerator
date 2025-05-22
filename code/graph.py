import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("kmeans_out.csv")

# scatter all points, coloring by cluster
pts = df[df['type']=="point"]
plt.scatter(pts.x, pts.y, c=pts.cluster, cmap='tab10', s=50, alpha=0.6)

# overlay centroids
ct = df[df['type']=="centroid"]
ct = df[df['type']=="centroid"]
plt.scatter(
    ct.x, ct.y,
    c=ct.cluster,        # ← use the cluster column
    cmap='tab10',        # ← same colormap
    marker='X',
    s=100,
    alpha=0.3,           # ← a little less opaque
    edgecolors='k',      # ← black outline so they stand out
    linewidths=1,
    label='centroids'
)

plt.legend()
plt.axis('equal')
plt.title("K-Means Result")
plt.show()
