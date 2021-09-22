_f_output:

  # peek stack and print
  lw $a0, 4($sp)
  li $v0, 1
  syscall

  # print newline
  li $v0, 11
  li $a0, 0x0a
  syscall

  # pop stack
  addiu $sp, $sp, 4

  # clear accumulator
  li $a0, 0
  j $ra

_f_input:
  
  # read int
  li $v0, 5
  syscall

  # move to accumulator
  move $a0, $v0

  j $ra