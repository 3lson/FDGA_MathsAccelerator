v.li v5, 42
v.li v1, 10
v.sw v1, 0(v5)
v.lw v2, 0(v5)
v.addi v3, v2, 10
v.sw v3, 2(v5)
v.muli v3, v2, 10
v.sw v3, 3(v5)
v.divi v3, v2, 10
v.sw v3, 4(v5)
v.slli v3, v2, 2
v.sw v3, 5(v5)
exit