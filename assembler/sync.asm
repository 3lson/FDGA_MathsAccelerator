v.li v2, 1
sx.slt s0, v30, v2  
s.beqz s0, warp1_code 
v.li v5, 123
v.li v6, 42
v.sw v5, 0(v6)  
j barrier           
warp1_code:
v.li v5, 999
v.li v6, 46
v.sw v5, 0(v6)  
barrier:
sync
sx.slt s4, v30, v2  
s.beqz s4, finish    
v.li s3, 1
sx.seq s4, v30, s3  
s.beqz s4, finish
v.li v6, 42 
v.lw v7, 0(v6)     
v.li v8, 50
v.sw v7, 0(v8)    
finish: 
exit