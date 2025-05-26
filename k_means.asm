# main2.c (K-Means core function)
# ------ Stack frame setup -----
.text
.text
.globl main2
.align 2
.type main2, @function
main2:

addi sp, sp, -272 # Allocate stack space
sw ra, 268(sp) # Save return address
sw s0, 264(sp) # Save frame pointer
addi s0, sp, 272 # Set new frame pointer

li t0, 100 # int max_iter = 100;
sw t0, -132(s0)

li t0, 9 # int num_points = 9;
sw t0, -128(s0)

li t0, 1 # int done = 1;
sw t0, -112(s0)

# Load points_x[9] and points_y[9] into stack memory
lui t0, %hi(.LC0)  # Load address of float 1.0
flw ft1, %lo(.LC0)(t0)
fsw ft1, -108(s0) # points_x[0] = 1.0

# Similar pattern
lui t0, %hi(.LC1)
flw ft1, %lo(.LC1)(t0)
fsw ft1, -104(s0)

lui t0, %hi(.LC2)
flw ft1, %lo(.LC2)(t0)
fsw ft1, -100(s0)

lui t0, %hi(.LC3)
flw ft1, %lo(.LC3)(t0)
fsw ft1, -96(s0)

lui t0, %hi(.LC4)
flw ft1, %lo(.LC4)(t0)
fsw ft1, -92(s0)

lui t0, %hi(.LC5)
flw ft1, %lo(.LC5)(t0)
fsw ft1, -88(s0)

lui t0, %hi(.LC6)
flw ft2, %lo(.LC6)(t0)
fneg.s ft1, ft2 # For the negative points
fsw ft1, -84(s0)

lui t0, %hi(.LC7)
flw ft2, %lo(.LC7)(t0)
fneg.s ft1, ft2
fsw ft1, -80(s0)

lui t0, %hi(.LC8)
flw ft2, %lo(.LC8)(t0)
fneg.s ft1, ft2
fsw ft1, -76(s0)

lui t0, %hi(.LC9)
flw ft1, %lo(.LC9)(t0)
fsw ft1, -72(s0)

lui t0, %hi(.LC10)
flw ft1, %lo(.LC10)(t0)
fsw ft1, -68(s0)

lui t0, %hi(.LC11)
flw ft1, %lo(.LC11)(t0)
fsw ft1, -64(s0)

lui t0, %hi(.LC12)
flw ft1, %lo(.LC12)(t0)
fsw ft1, -60(s0)

lui t0, %hi(.LC13)
flw ft1, %lo(.LC13)(t0)
fsw ft1, -56(s0)

lui t0, %hi(.LC14)
flw ft1, %lo(.LC14)(t0)
fsw ft1, -52(s0)

lui t0, %hi(.LC15)
flw ft2, %lo(.LC15)(t0)
fneg.s ft1, ft2
fsw ft1, -48(s0)

lui t0, %hi(.LC16)
flw ft2, %lo(.LC16)(t0)
fneg.s ft1, ft2
fsw ft1, -44(s0)

lui t0, %hi(.LC17)
flw ft2, %lo(.LC17)(t0)
fneg.s ft1, ft2
fsw ft1, -40(s0)

# for (i = 0; i < 3; i++) { centroids_x[i] = points_x[i]; centroids_y[i] = points_y[i]; }
li t0, 0 
sw t0, -116(s0) # i = 0
for_start0:
lw t1, -116(s0)
li t2, 3 
slt t0, t1, t2 
beqz t0, for_end1 # if (i>=3) break

# centr1oids_x[i] = points_x[i]
# centroids_y[i] = points_y[i]
lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -108
flw ft0, 0(t0) # load points_x[i]

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -264
fsw ft0, 0(t0) # store centroids_x[i]

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -72
flw ft0, 0(t0) # load points_y[i]

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -252
fsw ft0, 0(t0) # store centroids_y[i]

for_update2:
lw t0, -116(s0) # i++
addi t1, t0, 1
sw t1, -116(s0)
j for_start0
for_end1:

# for (cycle = 0; cycle < max_iter; cycle++)
li t0, 0
sw t0, -124(s0) # cycle = 0
for_start3:
lw t2, -124(s0)
lw s2, -132(s0) # max_iter
slt t0, t2, s2
beqz t0, for_end4 # if cycle < max_iter

li t0, 0
sw t0, -120(s0) # j =0
for_start6:
lw t2, -120(s0)
li s2, 3
slt t0, t2, s2
beqz t0, for_end7 # j < 3

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -264
flw ft0, 0(t0) # load centroids_x[j] 

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -240
fsw ft0, 0(t0) # store old_centroids_x[j]

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -252
flw ft0, 0(t0) # load centroids_y[j]

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -228
fsw ft0, 0(t0) # store old_centroids_y[j]

li t0, 0
lw t2, -120(s0)
slli t2, t2, 2
add t2, t2, s0
addi t2, t2, -144
sw t0, 0(t2) # cluster_sizes[j] = 0;

for_update8:
lw t0, -120(s0)
addi t2, t0, 1
sw t2, -120(s0) # cycle ++
j for_start6
for_end7:

li t0, 0
sw t0, -116(s0) # i = -
for_start9:
lw s2, -116(s0)
lw s3, -128(s0)
slt t0, s2, s3
beqz t0, for_end10 # i < num_points

li t0, 0
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -264
flw ft7, 0(t0) # centroids_x[0]

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -108
flw fa2, 0(t0) # points_x[i]

fsub.s ft2, ft7, fa2 # sub operations
abs ft1, ft2 # abs value 

li t0, 0
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -252
flw fa2, 0(t0) # centroids_y[0]

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -72
flw fa3, 0(t0) # points_y[i]

fsub.s ft7, fa2, fa3 # sub operations
abs ft2, ft7 # abs value

fadd.s ft0, ft1, ft2 # add the abs value together

fsw ft0, -36(s0) # store into min_dist

li t0, 0 # int best = 0

sw t0, -32(s0)
li t0, 1
sw t0, -120(s0) # j = 1

for_start12:
lw s2, -120(s0)
li s3, 3
slt t0, s2, s3
beqz t0, for_end13 # j < 3

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -264
flw ft7, 0(t0) # centroids_x[j]

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -108
flw fa2, 0(t0) # points_x[i]

fsub.s ft2, ft7, fa2 # sub operation
abs ft1, ft2 # abs value

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -252
flw fa2, 0(t0) # centroids_y[j]

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -72
flw fa3, 0(t0) # points_y[i]

fsub.s ft7, fa2, fa3 # sub operation
abs ft2, ft7 # abs value

fadd.s ft0, ft1, ft2 # add the abs value together
fsw ft0, -28(s0) # store into d 

flw ft0, -28(s0) # load d
flw ft1, -36(s0) # load min_dist
flt.s t0, ft0, ft1 # d < min_dist
beqz t0, else15
flw ft0, -28(s0) 
fsw ft0, -36(s0) # min_dist = d;
lw s2, -120(s0)
sw s2, -32(s0) # best = j;
j end_if16

else15:
end_if16:

for_update14:
lw t0, -120(s0)
addi s2, t0, 1 # j++
sw s2, -120(s0)

j for_start12
for_end13:
lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -108
flw ft0, 0(t0) # point_x[i]

lw s3, -32(s0)
slli s3, s3, 2
add s3, s3, s0
addi s3, s3, -144
lw t0, 0(s3) # load in cluster_sizes[best]
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -216
fsw ft0, 0(t0) # clusters_x[best][cluster_sizes[best]]

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -72
flw ft0, 0(t0) # points_y[i]

lw s3, -32(s0)
slli s3, s3, 2
add s3, s3, s0
addi s3, s3, -144
lw t0, 0(s3) # load in cluster_sizes[best]
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -180
fsw ft0, 0(t0) clusters_x[best][cluster_sizes[best]]

lw s3, -32(s0)
slli s3, s3, 2
add s3, s3, s0
addi s3, s3, -144
lw t0, 0(s3)
addi s3, t0, 1 # cluster_sizes[best]++

for_update11:
lw t0, -116(s0)
addi s4, t0, 1
sw s4, -116(s0) # i++

j for_start9
for_end10:

li t0, 0
sw t0, -116(s0) # i = 0

for_start17:
lw s5, -116(s0)
li s6, 3
slt t0, s5, s6
beqz t0, for_end18 # i<3

li t0, 0
fcvt.s.w ft0, t0
fsw ft0, -36(s0) # float sum_x
li s5, 0
fcvt.s.w ft0, s5
fsw ft0, -32(s0) # float sum_y

lw s5, -116(s0)
slli s5, s5, 2
add s5, s5, s0
addi s5, s5, -144
lw t0, 0(s5)
sw t0, -28(s0) # int size = cluster_sizes[i]

li t0, 0
sw t0, -120(s0) # j =0
for_start20:
lw s5, -120(s0)
lw s6, -28(s0)
slt t0, s5, s6
beqz t0, for_end21 # j < size

flw ft1, -36(s0)
lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -216
flw ft2, 0(t0)
fadd.s ft0, ft1, ft2
fsw ft0, -36(s0) # sum_x += clusters_x[i][j]

flw ft1, -32(s0)
lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -180
flw ft2, 0(t0)
fadd.s ft0, ft1, ft2
fsw ft0, -32(s0) # sum_y += clusters_y[i][j]

for_update22:
lw t0, -120(s0)
addi s5, t0, 1
sw s5, -120(s0) # j++
j for_start20
for_end21:

lw s6, -28(s0)
li s7, 0
sgt t0, s6, s7
beqz t0, else23 # size > 0

flw ft1, -36(s0)
lw ft2, -28(s0)
fcvt.s.w ft7, ft2
fdiv.s ft0, ft1, ft7 # sum_x / size
lw s6, -116(s0)
slli s6, s6, 2
add s6, s6, s0
addi s6, s6, -264
fsw ft0, 0(s6) # store in centroids_x[i]

flw ft1, -32(s0)
lw ft7, -28(s0)
fcvt.s.w fa2, ft7
fdiv.s ft0, ft1, fa2 # sum_y / size
lw s6, -116(s0)
slli s6, s6, 2
add s6, s6, s0
addi s6, s6, -252
fsw ft0, 0(s6) # store in centroids_y[i]
j end_if24
else23:
end_if24:

for_update19:
lw t0, -116(s0)
addi s6, t0, 1 # i++
sw s6, -116(s0)

j for_start17
for_end18:
li t0, 0
sw t0, -116(s0) # i =0

for_start25:
lw s7, -116(s0)
li s8, 3
slt t0, s7, s8
beqz t0, for_end26 # i <3

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -264
flw fa2, 0(t0) # centroids_x[i]

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -240
flw fa3, 0(t0) # old_centroids_x[o]

fsub.s ft1, fa2, fa3
abs ft0, ft1

fsw ft0, -36(s0) # store into side1

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -252
flw fa2, 0(t0) # centroids_y[i]

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -228
flw fa3, 0(t0) # old_centroids_y[i]

fsub.s ft1, fa2, fa3
abs ft0, ft1

fsw ft0, -32(s0) # store into side2

flw fa2, -36(s0)
lui s7, %hi(.LC18)
flw fa3, %lo(.LC18)(s7) # Loading 0.000001
flt.s ft1, fa2, fa3  # side1 < 0.000001

flw fa3, -32(s0)
lui s7, %hi(.LC19)
flw fa4, %lo(.LC19)(s7)
flt.s fa2, fa3, fa4 # side2 < 0.000001

beqz ft1, false_label # && evaluation
snez ft0, fa2
j end_label
false_label:
li ft0, 0
end_label:
li ft1, 0

feq.s t0, ft0, ft1
beqz t0, else28

li s7, 0 # done = 0
sw s7, -112(s0)
j for_end26 # break
j end_if29

else28:
end_if29:
for_update27:
lw t0, -116(s0)
addi s7, t0, 1 # i++
sw s7, -116(s0)
j for_start25
for_end26:

lw t0, -112(s0)
beqz t0, else30 # if (done)
j for_end4 # break
j end_if31
else30:
end_if31:

for_update5:
lw t0, -124(s0)
addi s8, t0, 1 # cycle++
sw s8, -124(s0)
j for_start3
for_end4:

li a0, 0 # return 0

# End of function
j main2_end
main2_end:
lw s0, 264(sp)
lw ra, 268(sp)
addi sp, sp, 272
ret

.size main2, .-main2

	.section .rodata # Data stored in points arrays
	.align 2
.LC0:
	.word 1065353216
	.align 2
.LC1:
	.word 1073741824
	.align 2
.LC2:
	.word 1065353216
	.align 2
.LC3:
	.word 1090519040
	.align 2
.LC4:
	.word 1091567616
	.align 2
.LC5:
	.word 1090519040
	.align 2
.LC6:
	.word 1065353216
	.align 2
.LC7:
	.word 1073741824
	.align 2
.LC8:
	.word 1065353216
	.align 2
.LC9:
	.word 1065353216
	.align 2
.LC10:
	.word 1065353216
	.align 2
.LC11:
	.word 1073741824
	.align 2
.LC12:
	.word 1090519040
	.align 2
.LC13:
	.word 1090519040
	.align 2
.LC14:
	.word 1091567616
	.align 2
.LC15:
	.word 1065353216
	.align 2
.LC16:
	.word 1065353216
	.align 2
.LC17:
	.word 1073741824
	.align 2
.LC18:
	.word 897988541
	.align 2
.LC19:
	.word 897988541

	.data 
	.align 2 # Initialize global arrays
	.type global_OUT_cluster_sizes, @object
	.globl global_OUT_cluster_sizes
global_OUT_cluster_sizes: 
	.zero 4
	.zero 4
	.zero 4

	.align 2
	.type global_OUT_clusters_y, @object
	.globl global_OUT_clusters_y
global_OUT_clusters_y:
	.zero 4
	.zero 4
	.zero 4
	.zero 4
	.zero 4
	.zero 4
	.zero 4
	.zero 4
	.zero 4

	.align 2
	.type global_OUT_clusters_x, @object
	.globl global_OUT_clusters_x
global_OUT_clusters_x:
	.zero 4
	.zero 4
	.zero 4
	.zero 4
	.zero 4
	.zero 4
	.zero 4
	.zero 4
	.zero 4

	.align 2
	.type global_OUT_centroids_y, @object
	.globl global_OUT_centroids_y
global_OUT_centroids_y:
	.zero 4
	.zero 4
	.zero 4

	.align 2
	.type global_OUT_centroids_x, @object
	.globl global_OUT_centroids_x
global_OUT_centroids_x:
	.zero 4
	.zero 4
	.zero 4

