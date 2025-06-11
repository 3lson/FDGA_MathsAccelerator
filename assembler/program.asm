v.add v1, x29, zero
v.li v2, 8
sx.slt s1, v1, v2
s.li s10, 42
s.sw s1, 0(s10)
exit