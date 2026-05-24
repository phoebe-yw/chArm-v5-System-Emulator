# SE Lab - chArm-v5 System Emulator

A full system emulator for the **chArm-v5** instruction set (a 25-instruction subset of AArch64) built in C as part of CS 429 (Computer Architecture) at UT Austin. The emulator models a 5-stage pipelined processor with forwarding, hazard control, and an LRU write-back cache.

## What I Implemented

| Checkpoint | File(s) | Description |
|---|---|---|
| 1 - HW Elements | `src/base/hw_elts.c` | Ripple-carry adder, ALU (13+ operations + EC conditionals), register file with XZR/SP handling, NZCV flag computation, `cond_holds` for all 15 branch conditions |
| 2 - PIPE- | `src/pipe/instr_{Fetch,Decode,Execute,Memory,Writeback}.c` | Full 5-stage pipeline with PC selection, instruction decode, operand muxing, memory access, and register writeback |
| 3 - PIPE | `src/pipe/forward.c`, `src/pipe/hazard_control.c` | Value forwarding (X→M→W to D priority), load-use stalls, branch misprediction squashing, RET/BR bubbling |
| 4 - CSIM + PCSIM | `src/cache/cache.c` | LRU matrix write-back cache with arbitrary sets/associativity/block size; integrated into pipeline as memory-stage stalls on miss |

### Pipeline Design Notes

- **PC selection:** RET correction (X stage) > mispredicted B.COND (M stage) > predicted PC
- **Branch prediction:** B.COND predicted taken; target computed by shifting 19-bit immediate left 2
- **Forwarding priority:** X-stage result > M-stage result > W-stage result
- **Hazard priorities:** Error > in-flight memory > misprediction > load-use > RET/BR
- **Cache:** 8x8 LRU bit matrix; write-back with write-allocate; dirty/clean eviction tracking; data memory only (instruction memory never misses)

---

## Running the Emulator

```bash
# Build
make

# Run a test binary
bin/se -i testcases/alu/print_simple/add

# Run with verbose pipeline register output
bin/se -i testcases/alu/print_simple/add -v 1   # stage values
bin/se -i testcases/alu/print_simple/add -v 2   # + control signals

# Run with cache enabled
bin/se -i testcases/applications/hard/gemm_block \
    -l 40000000 -c checkpoint.out \
    -A 4 -B 32 -C 512 -d 100

# Run full test suite
bin/test-se -w 1   # Checkpoint 1 (hw_elts)
bin/test-se -w 2   # Checkpoint 2 (PIPE-)
bin/test-se -w 3   # Checkpoint 3 (PIPE + hazards)
bin/test-se -w 4   # Checkpoint 4 (PCSIM)

# Test cache standalone
bin/test-csim
```

### Key Flags

| Flag | Description |
|---|---|
| `-i <binary>` | Input binary to run |
| `-v <1\|2>` | Verbose pipeline state output |
| `-l <n>` | Cycle limit (default 500) |
| `-c <file>` | Write checkpoint (register + memory state) to file |
| `-A <n>` | Cache associativity |
| `-B <n>` | Cache line size in bytes |
| `-C <n>` | Cache capacity in bytes |
| `-d <n>` | Cache miss penalty in cycles |

---

## Repository Structure

```
src/
  base/
    hw_elts.c        # ALU, register file, ripple-carry adder
    proc.c           # Pipeline clock and stage orchestration (provided)
    mem.c            # Memory interface (provided)
  pipe/
    instr_Fetch.c    # Fetch stage: PC select, branch predict, alias fix
    instr_Decode.c   # Decode stage: reg extract, immediate, control signals
    instr_Execute.c  # Execute stage: operand mux, ALU call
    instr_Memory.c   # Memory stage: LDUR/STUR, error handling
    instr_Writeback.c# Writeback stage: dst/value mux, regfile write
    forward.c        # Value forwarding X/M/W → D
    hazard_control.c # Stall, bubble, and squash logic
  cache/
    cache.c          # LRU write-back cache controller and data store
include/             # Header files for all modules
testcases/           # AArch64 binaries organized by category
```

## Tools Used

- **GCC** (x86-64 Linux, UTCS machines)
- **GDB** with conditional breakpoints for pipeline debugging
- **Gradescope** autograder
- **`se-ref`** reference binary for output comparison