# DES cracker v5.0
## Product Guide

### General description
DES cracker IP that groups multiple parallel workers. 

56-bit keys are separated in two part: fixed and variable. Width of both parts can be set through `VARIABLE_PART_WIDTH` 
parameter. Fixed part must be set by the user before exhaust begins. Each worker is independent and have its own fixed
part of key. When exhausting, each worker tries all possible values for variable part of keys (from all 0s to all 1s).

Variable part width must be between 24 and 55 because fixed part must be at most 32-bit wide to fit in a single AXI 
register.

2 references and 2 masks are defined globally. This lets use the bruteforce optimisation (see later).
The plaintext is also global.

When a worker finds a result that match the reference and the mask, it is stopped until the result is read.
The result consists in the 56-bit key and a bit that indicates against which reference it matches.
Communication is done through AXI bus.

In this version, enable signal is fixed because there is no need to stop a worker and reset signals are separated for 
each worker to be able to manage them separately.

### Notes registers
Some registers relate to all workers but some of them are linked to a single worker. In this case, the register "WORKER"
is used to determine the worker to get/set parameters of. The next table lists registers and whether they are global or
depend on the "WORKER" one.

| Register            | Global / worker-dependent |
|:--------------------|:--------------------------|
| version             | global                    |
| resets              | global                    |
| plaintext           | global                    |
| mask1               | global                    |
| mask2               | global                    |
| ref1                | global                    |
| ref2                | global                    |
| ended_outs          | global                    |
| results_available   | global                    |
| results_full        | global                    |
| worker              | global                    |
| fixed_key           | dependent                 |
| match_out / key_out | dependent                 |

For example: to set the start key of a worker, these steps must be followed:
- write in "WORKER" register the number of desired worker
- write in "START_KEY" register the key

This structure limits the number of registers needed to communicate with all workers.

Available registers are 32 bits wide. Thus, for the sake of simplicity, the number of workers is limited to 32. By this
way, "RESETS", "ENDED_OUTS", "RESULTS_AVAILABLE" and "RESULTS_FULL" are contained in a single 32-bit register. If this 
limit is reached, tests would be done to determine if modifying the IP to extend the limit to 64 workers is the best 
strategy to instantiate more of them.


### Inputs/Outputs
| Signal    | Type         | Comment                                   |
|-----------|--------------|-------------------------------------------|
| S_AXI_... | Input/Output | AXI bus to communicate with other modules |
| des_clk   | Input        | Clock of workers                          |


### IP parameters
#### NBR_WORKERS
Number of workers to generate.

#### VARIABLE_PART_WIDTH
Width of variable part of the key to exhaust. Fixed part is then the bits [55; 56 - VARIABLE_PART_WIDTH] of the key.

### Registers
#### Version

| Offset | Reset value |
|:------:|:-----------:|
|  0x00  | 0x0500_0000 |

Fields:

| Bits | 31 - 24 | 23 - 16 | 15 - 0 |
|------|:-------:|:-------:|:------:|
| Mode |    R    |    R    |        |
| Name |  major  |  minor  |  RFU   |

- major: major version number
- minor: minor version number

#### Resets

| Offset | Reset value |
|:------:|:-----------:|
|  0x04  | 0x0000_0000 |

Fields:

| Bits | 31 - NBR_WORKERS | NBR_WORKERS-1 - 0 |
|------|:----------------:|:-----------------:|
| Mode |                  |      resets       |
| Name |       RFU        |        R/W        |

- reset: set to 1 to reset corresponding worker

#### Plaintext high

| Offset | Reset value |
|:------:|:-----------:|
|  0x08  | 0x0000_0000 |

Fields:

| Bits |      31 - 0      |
|:----:|:----------------:|
| Mode |       R/W        |
| Name | plaintext[63-32] |

- MSBs of plaintext.

#### Plaintext low

| Offset | Reset value |
|:------:|:-----------:|
|  0x0C  | 0x0000_0000 |

Fields:

| Bits |     31 - 0      |
|:----:|:---------------:|
| Mode |       R/W       |
| Name | plaintext[31-0] |

- LSBs of plaintext.

#### Mask1 high

| Offset | Reset value |
|:------:|:-----------:|
|  0x10  | 0x0000_0000 |

Fields:

| Bits |    31 - 0    |
|:----:|:------------:|
| Mode |     R/W      |
| Name | mask1[63-32] |

- MSBs of first mask.

#### Mask1 low

| Offset | Reset value |
|:------:|:-----------:|
|  0x14  | 0x0000_0000 |

Fields:

| Bits |   31 - 0    |
|:----:|:-----------:|
| Mode |     R/W     |
| Name | mask1[31-0] |

- LSBs of first mask.

#### Mask2 high

| Offset | Reset value |
|:------:|:-----------:|
|  0x18  | 0x0000_0000 |

Fields:

| Bits |    31 - 0    |
|:----:|:------------:|
| Mode |     R/W      |
| Name | mask2[63-32] |

- MSBs of second mask.

#### Mask2 low

| Offset | Reset value |
|:------:|:-----------:|
|  0x1C  | 0x0000_0000 |

Fields:

| Bits |   31 - 0    |
|:----:|:-----------:|
| Mode |     R/W     |
| Name | mask2[31-0] |

- LSBs of second mask.

#### Ref1 high

| Offset | Reset value |
|:------:|:-----------:|
|  0x20  | 0xFFFF_FFFF |

Fields:

| Bits |   31 - 0    |
|:----:|:-----------:|
| Mode |     R/W     |
| Name | ref1[63-32] |

- MSBs of first reference.

#### Ref1 low

| Offset | Reset value |
|:------:|:-----------:|
|  0x24  | 0xFFFF_FFFF |

Fields:

| Bits |   31 - 0   |
|:----:|:----------:|
| Mode |    R/W     |
| Name | ref1[31-0] |

- LSBs of first reference.

#### Ref2 high

| Offset | Reset value |
|:------:|:-----------:|
|  0x28  | 0xFFFF_FFFF |

Fields:

| Bits |   31 - 0    |
|:----:|:-----------:|
| Mode |     R/W     |
| Name | ref2[63-32] |

- MSBs of second reference.

#### Ref2 low

| Offset | Reset value |
|:------:|:-----------:|
|  0x2C  | 0xFFFF_FFFF |

Fields:

| Bits |   31 - 0   |
|:----:|:----------:|
| Mode |    R/W     |
| Name | ref2[31-0] |

- LSBs of second reference.

#### Ended status

| Offset | Reset value |
|:------:|:-----------:|
|  0x30  | 0x0000_0000 |

Fields:

| Bits |      31 - 0      |
|:----:|:----------------:|
| Mode |        R         |
| Name | ended_outs[31-0] |

- all ended status bits.

#### Result available status

| Offset | Reset value |
|:------:|:-----------:|
|  0x34  | 0x0000_0000 |

Fields:

| Bits |         31 - 0          |
|:----:|:-----------------------:|
| Mode |            R            |
| Name | results_available[31-0] |

- all result available status bits.

#### Result full status

| Offset | Reset value |
|:------:|:-----------:|
|  0x38  | 0x0000_0000 |

Fields:

| Bits |       31 - 0       |
|:----:|:------------------:|
| Mode |         R          |
| Name | results_full[31-0] |

- all result full status bits.

#### Worker

| Offset | Reset value |
|:------:|:-----------:|
|  0x3C  | 0x0000_0000 |

Fields:

| Bits | 31 - 5 | 4 - 0  |
|:----:|:------:|:------:|
| Mode |        |  R/W   |
| Name |  RFU   | worker |

- worker to work on with worker-dependent registers.

#### Fixed key

| Offset | Reset value |
|:------:|:-----------:|
|  0x40  | 0x0000_0000 |

Fields:

| Bits | 31 - 56-VARIABLE_PART_WIDTH | 56-VARIABLE_PART_WIDTH-1 - 0 |
|:----:|:---------------------------:|:----------------------------:|
| Mode |                             |             R/W              |
| Name |             RFU             |          fixed_key           |

- fixed part of the key.

#### Match result high

| Offset | Reset value |
|:------:|:-----------:|
|  0x44  | 0x0000_0000 |

Fields:

| Bits |      31      | 30 - 24 |       23 - 0       |
|:----:|:------------:|:-------:|:------------------:|
| Mode |      R       |         |         R          |
| Name | match_number |   RFU   | matched_key[55-32] |

- match_number: 0 if key matched with mask1/ref1, 1 if it matched with mask2/ref2
- matched_keyN: MSBs of matched key.

#### Match result low

| Offset | Reset value |
|:------:|:-----------:|
|  0x48  | 0x0000_0000 |

Fields:

| Bits |      31 - 0       |
|:----:|:-----------------:|
| Mode |         R         |
| Name | matched_key[31-0] |

- matched_keyN: LSBs of matched key.