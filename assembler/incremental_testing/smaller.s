s.li s0, 4144
s.li sp, 4144
s.addi sp, sp, -144
s.sw ra, 140(sp)
s.sw s0, 136(sp)
s.addi s0, sp, 144
s.li s3, 15496
s.li s2, 15512
s.sw s2, 0(s3)
s.li s3, 15240
s.li s2, 15256
s.sw s2, 0(s3)
s.li s3, 15384
s.li s2, 1
s.sw s2, 0(s3)
s.li s3, 15392
s.li s2, 0
s.sw s2, 0(s3)
s.li s3, 15128
s.li s2, 0
s.sw s2, 0(s3)
s.li s3, 15500
s.sw sp, 0(s3)
s.li s3, 15244
s.sw sp, 0(s3)
s.li s3, 15504
s.sw ra, 0(s3)
s.li s3, 15484
s.sw s0, 0(s3)
s.li s3, 15228
s.sw s0, 0(s3)
s.li s2, 15496
s.lw gp, 0(s2)
s.li s2, 15412
s.lw s24, 0(s2)
s.li s2, 15404
s.lw s26, 0(s2)
v.li v1, 15244
v.lw sp, 0(v1)
v.li v1, 15240
v.lw gp, 0(v1)
v.li v1, 15228
v.lw v0, 0(v1)
v.li v1, 15128
v.lw v26, 0(v1)
s.j kernel_start0
warp_switch1:
s.seqi s2, s24, 0
s.beqz s2, warp_check5
s.li s2, 15380
s.li s4, 1
s.sw s4, 0(s2)
s.li s2, 15508
s.sw zero, 0(s2)
s.li s2, 15504
s.sw ra, 0(s2)
s.li s2, 15500
s.sw sp, 0(s2)
s.li s2, 15496
s.sw gp, 0(s2)
s.li s2, 15492
s.sw tp, 0(s2)
s.li s2, 15488
s.sw s0, 0(s2)
s.li s2, 15484
s.sw s1, 0(s2)
s.li s3, 15480
s.sw s2, 0(s3)
s.li s2, 15476
s.sw s3, 0(s2)
s.li s2, 15388
s.sw s25, 0(s2)
v.li v2, 15252
v.sw zero, 0(v2)
v.li v2, 15248
v.sw ra, 0(v2)
v.li v2, 15244
v.sw sp, 0(v2)
v.li v2, 15240
v.sw gp, 0(v2)
v.li v2, 15236
v.sw tp, 0(v2)
v.li v2, 15232
v.sw v0, 0(v2)
v.li v2, 15228
v.sw v1, 0(v2)
v.li v2, 15224
v.sw v2, 0(v2)
v.li v2, 15220
v.sw v3, 0(v2)
v.li v2, 15216
v.sw v4, 0(v2)
v.li v2, 15212
v.sw v5, 0(v2)
v.li v2, 15208
v.sw v6, 0(v2)
v.li v2, 15204
v.sw v7, 0(v2)
v.li v2, 15200
v.sw v8, 0(v2)
v.li v2, 15196
v.sw v9, 0(v2)
v.li v2, 15192
v.sw v10, 0(v2)
v.li v2, 15188
v.sw v11, 0(v2)
v.li v2, 15184
v.sw v12, 0(v2)
v.li v2, 15180
v.sw v13, 0(v2)
v.li v2, 15176
v.sw v14, 0(v2)
v.li v2, 15172
v.sw v15, 0(v2)
v.li v2, 15168
v.sw v16, 0(v2)
v.li v2, 15164
v.sw v17, 0(v2)
v.li v2, 15160
v.sw v18, 0(v2)
v.li v2, 15156
v.sw v19, 0(v2)
v.li v2, 15152
v.sw v20, 0(v2)
v.li v2, 15148
v.sw v21, 0(v2)
v.li v2, 15144
v.sw v22, 0(v2)
v.li v2, 15140
v.sw v23, 0(v2)
v.li v2, 15124
v.fsw fv0, 0(v2)
v.li v2, 15120
v.fsw fv1, 0(v2)
v.li v2, 15116
v.fsw fv2, 0(v2)
v.li v2, 15112
v.fsw fv3, 0(v2)
v.li v2, 15108
v.fsw fv4, 0(v2)
v.li v2, 15104
v.fsw fv5, 0(v2)
v.li v2, 15100
v.fsw fv6, 0(v2)
v.li v2, 15096
v.fsw fv7, 0(v2)
v.li v2, 15092
v.fsw fv8, 0(v2)
v.li v2, 15088
v.fsw fv9, 0(v2)
v.li v2, 15084
v.fsw fv10, 0(v2)
v.li v2, 15080
v.fsw fv11, 0(v2)
v.li v2, 15076
v.fsw fv12, 0(v2)
v.li v2, 15072
v.fsw fv13, 0(v2)
v.li v2, 15068
v.fsw fv14, 0(v2)
v.li v2, 15064
v.fsw fv15, 0(v2)
v.li v2, 15060
v.fsw fv16, 0(v2)
v.li v2, 15056
v.fsw fv17, 0(v2)
v.li v2, 15052
v.fsw fv18, 0(v2)
v.li v2, 15048
v.fsw fv19, 0(v2)
v.li v2, 15044
v.fsw fv20, 0(v2)
v.li v2, 15040
v.fsw fv21, 0(v2)
v.li v2, 15036
v.fsw fv22, 0(v2)
v.li v2, 15032
v.fsw fv23, 0(v2)
v.li v2, 15028
v.fsw fv24, 0(v2)
v.li v2, 15024
v.fsw fv25, 0(v2)
v.li v2, 15020
v.fsw fv26, 0(v2)
v.li v2, 15016
v.fsw fv27, 0(v2)
v.li v2, 15012
v.fsw fv28, 0(v2)
v.li v2, 15008
v.fsw fv29, 0(v2)
v.li v2, 15004
v.fsw fv30, 0(v2)
v.li v2, 15000
v.fsw fv31, 0(v2)
s.j load_next_warp3
warp_check5:
load_next_warp3:
s.li s2, 15380
s.lw s2, 0(s2)
s.beqz s2, load_warp_07
s.j no_more_warps6
load_warp_07:
s.li s2, 15508
s.lw zero, 0(s2)
s.li s2, 15504
s.lw ra, 0(s2)
s.j kernel_start0
no_more_warps6:
s.j kernel_end2
kernel_start0:
v.li v2, 123
v.li v3, 1
v.slli v3, v3, 2
v.li v4, 136
v.sub v3, v4, v3
v.add v3, v3, sp
v.sw v2, 0(v3)
s.j warp_switch1
kernel_end2:
s.li s1, 5
s.j f_end
f_end:
s.lw s0, 136(sp)
s.lw ra, 140(sp)
s.addi sp, sp, 144
exit
.size f, .-f

	.section .rodata

	.data