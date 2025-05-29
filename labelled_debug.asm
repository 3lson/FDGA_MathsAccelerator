.text
.text
.globl main2
.align 2
.type main2, @function
main2:

# ===== Stack Space Assignment ======
li s0, 1472
li sp, 1472
addi sp, sp, -272
sw ra, 268(sp)
sw s0, 264(sp)
addi s0, sp, 272

# max_iter = 10
li t0, 10
sw t0, -132(s0)

# num_points = 9
li t0, 9
sw t0, -128(s0)

# done = 1
li t0, 1
sw t0, -112(s0)

# points_x[9]
lui t0, %hi(.LC0)
flw ft1, %lo(.LC0)(t0)
fsw ft1, -108(s0)
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
fneg.s ft1, ft2
fsw ft1, -84(s0)
lui t0, %hi(.LC7)
flw ft2, %lo(.LC7)(t0)
fneg.s ft1, ft2
fsw ft1, -80(s0)
lui t0, %hi(.LC8)
flw ft2, %lo(.LC8)(t0)
fneg.s ft1, ft2
fsw ft1, -76(s0)

# points_y[9]
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

# int i = 0
li t0, 0
sw t0, -116(s0)

for_start0:
lw t1, -116(s0)
li t2, 3
slt t0, t1, t2 # i<3
beqz t0, for_end1

# points_x[i]
lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -108
flw ft0, 0(t0)

# centroids_x[i]
lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -264
fsw ft0, 0(t0)

# points_y[i]
lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -72
flw ft0, 0(t0)

# centroids_y[i]
lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -252
fsw ft0, 0(t0)

for_update2:
lw t0, -116(s0)
addi t1, t0, 1 # i++
sw t1, -116(s0)
j for_start0

for_end1:
li t0, 0
sw t0, -124(s0) # cycle =0

for_start3:
lw t2, -124(s0)
lw s1, -132(s0)
slt t0, t2, s1 # cycle < max_iter
beqz t0, for_end4

li t0, 1
sw t0, -112(s0) # done =1

li t0, 0
sw t0, -120(s0) # j = 0

for_start6:
lw t2, -120(s0)
li s1, 3
slt t0, t2, s1 # j < 3
beqz t0, for_end7

# centroids_x[j]
lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -264
flw ft0, 0(t0)

# old_centroids_x[j]
lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -240
fsw ft0, 0(t0)

# centroids_y[j]
lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -252
flw ft0, 0(t0)

# old_centroids_y[i]
lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -228
fsw ft0, 0(t0)

for_update8:
lw t0, -120(s0)
addi t2, t0, 1 # j ++
sw t2, -120(s0)
j for_start6

for_end7:
for_update5:
lw t0, -124(s0)
addi s1, t0, 1 # cycle++
sw s1, -124(s0)
j for_start3

for_end4:
li a0, 5 # return 5
j main2_end

# End of stack allocation
main2_end:
lw s0, 264(sp)
lw ra, 268(sp)
addi sp, sp, 272
ret

.size main2, .-main2

	.section .rodata
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

	.data
	.align 2
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

