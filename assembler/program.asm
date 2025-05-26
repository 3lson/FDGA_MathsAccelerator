.text
.text
.globl main2
.align 2
.type main2, @function

main2:
addi sp, sp, -272
sw ra, 268(sp)
sw s0, 264(sp)
addi s0, sp, 272

li t0, 100
sw t0, -132(s0)
li t0, 9
sw t0, -128(s0)
li t0, 1
sw t0, -112(s0)

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

li t0, 0
sw t0, -116(s0)

for_start0:

lw t1, -116(s0)
li t2, 3
slt t0, t1, t2
beqz t0, for_end1

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -108
flw ft0, 0(t0)

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -264
fsw ft0, 0(t0)

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -72
flw ft0, 0(t0)

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -252
fsw ft0, 0(t0)

for_update2:
lw t0, -116(s0)
addi t1, t0, 1
sw t1, -116(s0)
j for_start0

for_end1:
li t0, 0
sw t0, -124(s0)
for_start3:

lw t2, -124(s0)
lw s1, -132(s0)
slt t0, t2, s1
beqz t0, for_end4

for_end4:

li t0, 0
sw t0, -120(s0)

for_start6:

lw t2, -120(s0)
li s1, 3
slt t0, t2, s1
beqz t0, for_end7

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -264
flw ft0, 0(t0)

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -240
fsw ft0, 0(t0)

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -252
flw ft0, 0(t0)

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -228
fsw ft0, 0(t0)

li t0, 0
lw t2, -120(s0)
slli t2, t2, 2
add t2, t2, s0
addi t2, t2, -144
sw t0, 0(t2)

for_update8:
lw t0, -120(s0)
addi t2, t0, 1
sw t2, -120(s0)
j for_start6

for_end7:
li t0, 0
sw t0, -116(s0)

for_start9:
lw s1, -116(s0)
lw a1, -128(s0)
slt t0, s1, a1
beqz t0, for_end10

li t0, 0
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -264
flw ft7, 0(t0)

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -108
flw fa2, 0(t0)

fsub.s ft2, ft7, fa2
fabs.s ft1, ft2

li t0, 0
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -252
flw fa2, 0(t0)

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -72
flw fa3, 0(t0)

fsub.s ft7, fa2, fa3
fabs.s ft2, ft7
fadd.s ft0, ft1, ft2

fsw ft0, -36(s0)
li t0, 0
sw t0, -32(s0)
li t0, 1
sw t0, -120(s0)

for_start12:
lw s1, -120(s0)
li a1, 3
slt t0, s1, a1
beqz t0, for_end13

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -264
flw ft7, 0(t0)

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -108
flw fa2, 0(t0)

fsub.s ft2, ft7, fa2
fabs.s ft1, ft2

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -252
flw fa2, 0(t0)

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -72
flw fa3, 0(t0)

fsub.s ft7, fa2, fa3
fabs.s ft2, ft7
fadd.s ft0, ft1, ft2

fsw ft0, -28(s0)
flw ft0, -28(s0)
flw ft1, -36(s0)
flt.s t0, ft0, ft1
beqz t0, else15

flw ft0, -28(s0)
fsw ft0, -36(s0)
lw s1, -120(s0)
sw s1, -32(s0)
j end_if16

else15:
end_if16:
for_update14:
lw t0, -120(s0)
addi s1, t0, 1
sw s1, -120(s0)
j for_start12

for_end13:
lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -108
flw ft0, 0(t0)

lw a1, -32(s0)
slli a1, a1, 2
add a1, a1, s0
addi a1, a1, -144
lw t0, 0(a1)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -216
fsw ft0, 0(t0)

lw t0, -116(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -72
flw ft0, 0(t0)

lw a1, -32(s0)
slli a1, a1, 2
add a1, a1, s0
addi a1, a1, -144
lw t0, 0(a1)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -180
fsw ft0, 0(t0)

lw a1, -32(s0)
slli a1, a1, 2
add a1, a1, s0
addi a1, a1, -144
lw t0, 0(a1)
addi a1, t0, 1

for_update11:
lw t0, -116(s0)
addi a2, t0, 1
sw a2, -116(s0)
j for_start9

for_end10:
li t0, 0
sw t0, -116(s0)

for_start17:
lw a3, -116(s0)
li a4, 3
slt t0, a3, a4
beqz t0, for_end18
li t0, 0

fcvt.s.w ft0, t0
fsw ft0, -36(s0)
li a3, 0

fcvt.s.w ft0, a3
fsw ft0, -32(s0)
lw a5, -116(s0)
slli a5, a5, 2
add a5, a5, s0
addi a5, a5, -144
lw a4, 0(a5)
sw a4, -28(s0)
li a4, 0
sw a4, -120(s0)

for_start20:
lw a5, -120(s0)
lw a6, -28(s0)
slt a4, a5, a6
beqz a4, for_end21
flw ft1, -36(s0)

lw a4, -120(s0)
slli a4, a4, 2
add a4, a4, s0
addi a4, a4, -216
flw ft2, 0(a4)

fadd.s ft0, ft1, ft2
fsw ft0, -36(s0)
flw ft1, -32(s0)

lw a4, -120(s0)
slli a4, a4, 2
add a4, a4, s0
addi a4, a4, -180
flw ft2, 0(a4)
fadd.s ft0, ft1, ft2
fsw ft0, -32(s0)

for_update22:
lw a4, -120(s0)
addi a5, a4, 1
sw a5, -120(s0)
j for_start20

for_end21:
lw a6, -28(s0)
li a7, 0

sgt a4, a6, a7
beqz a4, else23
flw ft1, -36(s0)
lw a6, -28(s0)
fcvt.s.w ft2, a6
fdiv.s ft0, ft1, ft2
lw a7, -116(s0)
slli a7, a7, 2
add a7, a7, s0
addi a7, a7, -264
fsw ft0, 0(a7)
flw ft1, -32(s0)

lw a7, -28(s0)
fcvt.s.w ft2, a7
fdiv.s ft0, ft1, ft2
lw s2, -116(s0)
slli s2, s2, 2
add s2, s2, s0
addi s2, s2, -252
fsw ft0, 0(s2)

j end_if24
else23:
end_if24:
for_update19:
lw a4, -116(s0)
addi s2, a4, 1
sw s2, -116(s0)
j for_start17

for_end18:
li a4, 0
sw a4, -116(s0)
for_start25:

lw s3, -116(s0)
li s4, 3
slt a4, s3, s4
beqz a4, for_end26
lw a4, -116(s0)
slli a4, a4, 2
add a4, a4, s0
addi a4, a4, -264
flw ft2, 0(a4)

lw a4, -116(s0)
slli a4, a4, 2
add a4, a4, s0
addi a4, a4, -240
flw ft7, 0(a4)

fsub.s ft1, ft2, ft7
fabs.s ft0, ft1

fsw ft0, -36(s0)
lw a4, -116(s0)
slli a4, a4, 2
add a4, a4, s0
addi a4, a4, -252
flw ft2, 0(a4)

lw a4, -116(s0)
slli a4, a4, 2
add a4, a4, s0
addi a4, a4, -228
flw ft7, 0(a4)

fsub.s ft1, ft2, ft7
fabs.s ft0, ft1
fsw ft0, -32(s0)
flw ft2, -36(s0)

lui s3, %hi(.LC18)
flw ft7, %lo(.LC18)(s3)
flt.s ft1, ft2, ft7
flw ft7, -32(s0)

lui s3, %hi(.LC19)
flw fa2, %lo(.LC19)(s3)
flt.s ft2, ft7, fa2

beqz ft1, false_label
snez ft0, ft2
j end_label
false_label:

li s3, 0
fcvt.s.w ft0, s3

end_label:
li s4, 0
fcvt.s.w ft1, s4
feq.s a4, ft0, ft1
beqz a4, else28
li s5, 0
sw s5, -112(s0)
j for_end26
j end_if29
else28:
end_if29:
for_update27:

lw a4, -116(s0)
addi s5, a4, 1
sw s5, -116(s0)
j for_start25
for_end26:
lw a4, -112(s0)
beqz a4, else30
j for_end4
j end_if31
else30:
end_if31:
for_update5:

lw a4, -124(s0)
addi s6, a4, 1
sw s6, -124(s0)
j for_start3
for_end4:
li a0, 0
j main2_end
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
	.align 2
.LC18:
	.word 897988541
	.align 2
.LC19:
	.word 897988541

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


