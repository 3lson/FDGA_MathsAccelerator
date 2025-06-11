v.li v5, 42
v.li v1, 10
v.sw v1, 0(v5)
v.lw v2, 0(v5)
v.add v3, v1, v2
v.sw v3, 1(v5)
v.sub v3, v1, v2
v.sw v3, 2(v5)
v.mul v3, v1, v2
v.sw v3, 3(v5)
v.div v3, v1, v2
v.sw v3, 4(v5)
v.slt v3, v1, v2
v.sw v3, 5(v5)
v.seq v3, v1, v2
v.sw v3, 6(v5)
v.snez v3, v1
v.sw v3, 7(v5)
v.li v4, -5
v.abs v4, v4
v.sw v4, 8(v5)
exit