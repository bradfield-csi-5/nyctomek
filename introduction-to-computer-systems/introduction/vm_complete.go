package vm

import (
	"fmt"
	"os"
)

const (
	Load  = 0x01
	Store = 0x02
	Add   = 0x03
	Sub   = 0x04
	Halt  = 0xff
)

// Stretch goals
const (
	Addi = 0x05
	Subi = 0x06
	Jump = 0x07
	Beqz = 0x08
)

// Given a 256 byte array of "memory", run the stored program
// to completion, modifying the data in place to reflect the result
//
// The memory format is:
//
// 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f ... ff
// __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ ... __
// ^==DATA===============^ ^==INSTRUCTIONS==============^
//
func compute(memory []byte) {

	const MemorySize = 256
	const InstructionSize = 3
	const InstructionSegmentBoundary = 8
	const SIGSEGV = 11

	registers := [3]byte{8, 0, 0} // PC, R1 and R2

	// Keep looping, like a physical computer's clock
	for {

		pc := registers[0]
		op := memory[pc] // fetch the opcode

		var param1, param2 byte
		if op != Halt && pc < MemorySize - 2 {
			param1 = memory[pc + 1]
			param2 = memory[pc + 2]
		}
		// decode and execute
		switch op {
		case Load:
			registers[param1] = memory[param2]
		case Store:
			if param2 >= InstructionSegmentBoundary {
				fmt.Fprintf(os.Stderr, "Invalid memory access at address %d.\n", param2)
				os.Exit(SIGSEGV)
			}
			memory[param2] = registers[param1]
		case Add:
			registers[param1] = registers[param1] + registers[param2]
		case Sub:
			registers[param1] = registers[param1] - registers[param2]
		case Addi:
			registers[param1] = registers[param1] + param2
		case Subi:
			registers[param1] = registers[param1] - param2
		case Jump:
			registers[0] = param1
			continue
		case Beqz:
			if registers[param1] == 0 {
				registers[0] += param2
			}
		case Halt:
			return
		}
		registers[0] += InstructionSize
	}
}
