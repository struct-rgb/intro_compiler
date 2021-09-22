.text
_f_output:
  lw $a0, 4($sp)
  li $v0, 1
  syscall
  li $v0, 11
  li $a0, 0x0a
  syscall
  li $a0, 0
  jr $ra

_f_input:
  li $v0, 5
  syscall
  move $a0, $v0
  jr $ra

_f_main:
  sw $ra, 0($sp)
  move $fp, $sp
  addiu $sp, $sp, -4
  addiu $sp, $sp, -4
  addi $a0, $fp, -4
  sw $a0, 0($sp)
  addiu $sp, $sp, -4
  sw $fp, 0($sp)
  addiu $sp, $sp, -4
  jal _f_input
  addiu $sp, $sp, 4
  lw $fp, 0($sp)
  lw $t1, 4($sp)
  sw $a0, 0($t1)
  addiu $sp, $sp, 4
  sw $fp, 0($sp)
  addiu $sp, $sp, -4
  addi $a0, $fp, -4
  lw $a0, 0($a0)
  sw $a0, 0($sp)
  addiu $sp, $sp, -4
  sw $fp, 0($sp)
  addiu $sp, $sp, -4
  jal _f_input
  addiu $sp, $sp, 4
  lw $fp, 0($sp)
  lw $t1, 4($sp)
  add $a0, $t1, $a0
  addiu $sp, $sp, 4
  sw $a0, 0($sp)
  addiu $sp, $sp, -4
  jal _f_output
  addiu $sp, $sp, 8
  lw $fp, 0($sp)
  addiu $sp, $sp, 4
_f_main_exit:
  move $sp, $fp
  lw $ra, 0($sp)
  jr $ra

main:
  jal _f_main
  li $v0, 10
  syscall
