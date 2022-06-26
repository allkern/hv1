<p align="center">
  <img width="70%" height="70%" src="https://user-images.githubusercontent.com/15825466/175829437-acfe7831-9647-46c3-8e51-1d58136ed3d0.png" alt="Hyrisc">
</p>

# Hyrisc
Hyrisc or HyRISC (pronounced "High-risk") is a simple 32-bit RISC architecture designed from the ground up for swift development for both compilers and humans

This repo contains a board-level emulator and assembler for Hyrisc

## Main features
- Expandable RISC ISA totalling 30 instructions
- 32 registers with template ABI
- 32 Floating Point registers
- 4-level Link registers
- ARM-like Condition Codes for Flow Control instructions
- Little Endian Architecture
- Integer and Floating Point Multiplier and Divider
- On-chip Bus Controller (BCI)
- On-chip PIC
- MAC Unit
- Multicore-capable

### Bus/Memory
- 32-bit Address and Data buses
- `BUSREQ`, `BUSACK` and `BUSIRQ` signals
- 8-bit Error bus (`BE` signals) with IRQs for each code
- Indexed mode `LOAD` and `STORE` for fast array and struct access
- X (eXecution) pin spec planned

### Interrupts
- Integrated PIC with 32-bit vector bus
- `IRQ` and `IRQACK` signals

## Emulator features
- Board-level with individual pin manipulation
- Simple API with easily serializable structs
- Multiple CPU support
- Planned support for user-defined machines (QEMU-like)
- Cross-platform
- PCIBus through emulated x86 IO bus support underway
- Complete access to CPU internals
