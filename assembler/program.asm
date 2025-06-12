.text
.globl main
main:
    s.addi s1, zero, 10
    s.addi s2, zero, 0

loop:
    s.addi s2, s2, 1
    s.slt s3, s2, s1
    s.beq s3, zero, loop  

    sync

    s.add s4, s1, s2
    s.mul s5, s4, s2
    j end_program

ENDSYNC:
    s.add s6, s6, s1

end_program:
    exit