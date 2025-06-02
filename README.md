# FDGA

## UI must-dos
- Read from a .mem file for visualisation
- Interactive buttons to run/stop/reset
- Proper graph layout

## UI implementation idea
- allow the user to select 3 cluster points in a fixed graph consisting of a fixed number of points
- The user should be able to select the points using computer vision via hand signals tracking the movement of their hand to a cursor which by pressing the thumb and index finger results in the cluster points being placed
- The algorithm wont start running until 3 cluster points are placed and then it will be accelerated on the fpga
- if a 4th point is placed in real-time the 1st point is replaced and the new clusters are recalcualted by the fpga in real time implemened via an interrupt system
- if a 5th point is placed in real-time the 2nd point is replaced and the new clusters are recalculated again via interrupts
- This can go on and on until the user chooses to end the live program
- EXTRA: overlay can show performance of clustering via calculating variances of clusters and distribution which can show the user how cluster position impacts clustering performance
