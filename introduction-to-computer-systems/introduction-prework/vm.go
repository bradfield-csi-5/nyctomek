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

// Type of read operation.
const (
	Data   = 1
	OpCode = 2
)

// Address space segments.
const (
	AddrStoreLocation = 0
	AddrDataStart     = 0
	AddrDataEnd       = 7
	AddrOpCodeStart   = 8
)

// Standard OpCode encoding size.
// Doesn't apply to Halt or Jump, but those
// instructions are special in that their
// encoding size isn't used to advance the PC.
const (
	StandardOpCodeSize = 3
)

const (
	IllegalMemoryAccess  = -1
	IllegalOpCode        = -2
)

func readMemory(memory[] byte, address byte, accessType int) byte {

	validAddress := true

	if accessType == OpCode {
		if address < AddrOpCodeStart {
			validAddress = false
		}
	} else if accessType == Load {
		if address < AddrDataStart || address > AddrDataEnd {
			validAddress = false
		}
	} else {
		fmt.Fprintf(os.Stderr, "Illegal access type %d.\n", accessType)
		os.Exit(IllegalMemoryAccess)
	}

	if !validAddress {

		var accessTypeStr string
		if accessType == Load {
			accessTypeStr = "Load"
		} else {
			accessTypeStr = "OpCode"
		}
	
		fmt.Fprintf(os.Stderr,
			"Invalid address %d for access type %s.\n",
			address, accessTypeStr)

		os.Exit(IllegalMemoryAccess)
	}

	return memory[address]
}

func writeMemory(memory[] byte, address byte, value byte) {

	if address != AddrStoreLocation {
		fmt.Fprintf(os.Stderr, "Invalid address %d for write.\n", address)
		os.Exit(IllegalMemoryAccess)
	}

	memory[address] = value
}

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

	registers := [3]byte{8, 0, 0} // PC, R1 and R2

	// Keep looping, like a physical computer's clock
	for {

		pc := registers[0]

		// fetch the opcode
		op := readMemory(memory, pc, OpCode)

		// decode and execute
		switch op {

		case Load:
			register := readMemory(memory, pc + 1, OpCode)
			address  := readMemory(memory, pc + 2, OpCode)
			registers[register] = readMemory(memory, address, Data)

		case Store:
			register := readMemory(memory, pc + 1, OpCode)
			address  := readMemory(memory, pc + 2, OpCode)
			writeMemory(memory, address, registers[register])

		case Add:
			arg1 := readMemory(memory, pc + 1, OpCode)
			arg2 := readMemory(memory, pc + 2, OpCode)
			registers[arg1] = registers[arg1] + registers[arg2]

		case Sub:
			arg1 := readMemory(memory, pc + 1, OpCode)
			arg2 := readMemory(memory, pc + 2, OpCode)
			registers[arg1] = registers[arg1] - registers[arg2]
	
		case Addi:
			arg1 := readMemory(memory, pc + 1, OpCode)
			arg2 := readMemory(memory, pc + 2, OpCode)
			registers[arg1] = registers[arg1] + arg2

		case Subi:
			arg1 := readMemory(memory, pc + 1, OpCode)
			arg2 := readMemory(memory, pc + 2, OpCode)
			registers[arg1] = registers[arg1] - arg2

		case Jump:
			address := readMemory(memory, pc + 1, OpCode)
			registers[0] = address
			continue

		case Beqz:
			register := readMemory(memory, pc + 1, OpCode)
			offset   := readMemory(memory, pc + 2, OpCode)
			if registers[register] == 0 {
				registers[0] += offset
			}

		case Halt:
			return
		
		default:
			fmt.Fprintf(os.Stderr, "Illegal opcode: %d.\n", op)
			os.Exit(IllegalOpCode)
		}
	
		registers[0] += StandardOpCodeSize
	}
}
