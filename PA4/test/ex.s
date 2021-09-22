.data

.text

_f_output:
  lw $a0, 4($sp)
  li $v0, 1
  syscall
  li $v0, 11
  li $a0, 0x0a
  syscall
  addiu $sp, $sp, 4
  li $a0, 0
  j $ra


_f_main:
  sw $ra, 0($sp)
  addiu $sp, $sp, -4
  li $a0, 4
  sw $a0, 0($sp)
  addiu $sp, $sp, -4
  li $a0, 5
  lw $t1, 4($sp)
  addiu $sp, $sp, 4
  add $a0, $t1, $a0
  sw $a0, 0($sp)
  addiu $sp, $sp, -4
  jal _f_output
  lw $ra, 4($sp)
  addiu $sp, $sp, 4
  j $ra


main:
  jal _f_main
  li $v0, 10
  syscall