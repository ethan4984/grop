test1:
	riscv32-elf-as test1.s -o test1.o
	riscv32-elf-ld -T linker.ld test1.o -o test1.elf
	riscv32-elf-objcopy -O binary test1.elf
	rm test1.o
