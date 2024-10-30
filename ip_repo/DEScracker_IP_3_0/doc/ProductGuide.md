# DES cracker v1.0
## Product Guide

### General description
DES cracker IP that groups multiple parallel workers. Each worker is independent and have its own start key and end key.
It exhausts all keys between them.

2 references and 2 masks are defined globally. This lets use the bruteforce optimisation (see later).
The plaintext is also global.

When a worker finds a result that match the reference and the mask, it is stopped until the result is read.
The result consists in the 56-bit key and a bit that indicates against which reference it matches.
Communication is done through AXI bus.

### Notes registers
Available registers are 32 bits wide.

| Signals               | Registers            |
|-----------------------|----------------------|
| reset / enable        | 1                    |
| plaintext             | 2                    |
| start_keys            | 2 * nbr_workers      |
| end_keys              | 2 * nbr_workers      |
| mask1                 | 2                    |
| mask2                 | 2                    |
| ref1                  | 2                    |
| ref2                  | 2                    |
| read_results          | 0                    |
| current_keys          | 2 * nbr_workers      |
| ended_outs            | nbr_worker // 32 + 1 |
| results_available     | nbr_worker // 32 + 1 |
| results_full          | nbr_worker // 32 + 1 |
| match_outs / key_outs | 2 * nbr_workers      |

Thus, in total, number of mandatory registers is computed as follow: 11 + 8 * nbr_workers + 3 * (nbr_workers // 32 + 1).
Then, we have this table:

| nbr_workers | Registres |
|-------------|-----------|
| 1           | 22        |
| 2           | 30        |
| 4           | 46        |
| 8           | 78        |
| 16          | 142       |
| 32          | 273       |
| 61          | 505       |
| 62          | 513       |
| 63          | 521       |
| 64          | 532       |

Max number of registers in an AXI4 IP is 512. Thus, nbr_worker is limited to 61.


### Inputs/Outputs
| Signal     | Type         | Comment                                   |
|------------|--------------|-------------------------------------------|
| S_AXI_...  | Input/Output | AXI bus to communicate with other modules |


### IP parameters
#### NBR_WORKERS
Number of workers to generate.

### Registers
#### Status

| Offset | Reset value |
|:------:|:-----------:|
|  0x00  | 0x0100_0000 |

Fields:

| Bits | 31 - 24 | 23 - 16 | 15 - 2 |   1    |   0   |
|------|:-------:|:-------:|:------:|:------:|:-----:|
| Mode |    R    |    R    |        |  R/W   |  R/W  |
| Name |  major  |  minor  |  RFU   | enable | reset |

- major: major version number
- minor: minor version number
- enable: 1 if workers are enabled, 0 if disabled
- reset: set to 1 to reset workers

#### Plaintext high

| Offset | Reset value |
|:------:|:-----------:|
|  0x04  | 0x0000_0000 |

Fields:

| Bits |      31 - 0      |
|:----:|:----------------:|
| Mode |       R/W        |
| Name | plaintext[63-32] |

- MSBs of plaintext.

#### Plaintext low

| Offset | Reset value |
|:------:|:-----------:|
|  0x08  | 0x0000_0000 |

Fields:

| Bits |     31 - 0      |
|:----:|:---------------:|
| Mode |       R/W       |
| Name | plaintext[31-0] |

- LSBs of plaintext.

#### Mask1 high

| Offset | Reset value |
|:------:|:-----------:|
|  0x0C  | 0xFFFF_FFFF |

Fields:

| Bits |    31 - 0    |
|:----:|:------------:|
| Mode |     R/W      |
| Name | mask1[63-32] |

- MSBs of first mask.

#### Mask1 low

| Offset | Reset value |
|:------:|:-----------:|
|  0x10  | 0xFFFF_FFFF |

Fields:

| Bits |   31 - 0    |
|:----:|:-----------:|
| Mode |     R/W     |
| Name | mask1[31-0] |

- LSBs of first mask.

#### Mask2 high

| Offset | Reset value |
|:------:|:-----------:|
|  0x14  | 0xFFFF_FFFF |

Fields:

| Bits |    31 - 0    |
|:----:|:------------:|
| Mode |     R/W      |
| Name | mask2[63-32] |

- MSBs of second mask.

#### Mask2 low

| Offset | Reset value |
|:------:|:-----------:|
|  0x18  | 0xFFFF_FFFF |

Fields:

| Bits |   31 - 0    |
|:----:|:-----------:|
| Mode |     R/W     |
| Name | mask2[31-0] |

- LSBs of second mask.

#### Ref1 high

| Offset | Reset value |
|:------:|:-----------:|
|  0x1C  | 0xFFFF_FFFF |

Fields:

| Bits |   31 - 0    |
|:----:|:-----------:|
| Mode |     R/W     |
| Name | ref1[63-32] |

- MSBs of first reference.

#### Ref1 low

| Offset | Reset value |
|:------:|:-----------:|
|  0x20  | 0xFFFF_FFFF |

Fields:

| Bits |   31 - 0   |
|:----:|:----------:|
| Mode |    R/W     |
| Name | ref1[31-0] |

- LSBs of first reference.

#### Ref2 high

| Offset | Reset value |
|:------:|:-----------:|
|  0x24  | 0xFFFF_FFFF |

Fields:

| Bits |   31 - 0    |
|:----:|:-----------:|
| Mode |     R/W     |
| Name | ref2[63-32] |

- MSBs of second reference.

#### Ref2 low

| Offset | Reset value |
|:------:|:-----------:|
|  0x28  | 0xFFFF_FFFF |

Fields:

| Bits |   31 - 0   |
|:----:|:----------:|
| Mode |    R/W     |
| Name | ref2[31-0] |

- LSBs of second reference.

#### Start key N high

|    Offset    | Reset value |
|:------------:|:-----------:|
| 0x2C + N * 8 | 0x0000_0000 |

Fields:

| Bits | 31-24 |      23 - 0       |
|:----:|:-----:|:-----------------:|
| Mode |       |        R/W        |
| Name |  RFU  | start_keyN[55-32] |

- MSBs of start key in Nth worker.

#### Start key N low

|    Offset    | Reset value |
|:------------:|:-----------:|
| 0x30 + N * 8 | 0x0000_0000 |

Fields:

| Bits |      31 - 0      |
|:----:|:----------------:|
| Mode |       R/W        |
| Name | start_keyN[31-0] |

- LSBs of start key in Nth worker.

#### End key N high

|    Offset     | Reset value |
|:-------------:|:-----------:|
| 0x214 + N * 8 | 0x0000_0000 |

Fields:

| Bits | 31-24 |     23 - 0      |
|:----:|:-----:|:---------------:|
| Mode |       |       R/W       |
| Name |  RFU  | end_keyN[55-32] |

- MSBs of end key in Nth worker.

#### End key N low

|    Offset     | Reset value |
|:-------------:|:-----------:|
| 0x218 + N * 8 | 0x0000_0000 |

Fields:

| Bits |     31 - 0     |
|:----:|:--------------:|
| Mode |      R/W       |
| Name | end_keyN[31-0] |

- LSBs of end key in Nth worker.

#### Current key N high

|    Offset     | Reset value |
|:-------------:|:-----------:|
| 0x3FC + N * 8 | 0x0000_0000 |

Fields:

| Bits | 31-24 |       23 - 0        |
|:----:|:-----:|:-------------------:|
| Mode |       |          R          |
| Name |  RFU  | current_keyN[55-32] |

- MSBs of current key in Nth worker.

#### Current key N low

|    Offset     | Reset value |
|:-------------:|:-----------:|
| 0x400 + N * 8 | 0x0000_0000 |

Fields:

| Bits |       31 - 0       |
|:----:|:------------------:|
| Mode |         R          |
| Name | current_keyN[31-0] |

- LSBs of current key in Nth worker.

#### Ended status high

|    Offset     | Reset value |
|:-------------:|:-----------:|
| 0x5E4 + N * 8 | 0x0000_0000 |

Fields:

| Bits | 31-29 |      28 - 0       |
|:----:|:-----:|:-----------------:|
| Mode |       |         R         |
| Name |  RFU  | ended_outs[60-32] |

- MSBs of all ended status bits.

#### Ended status low

|    Offset     | Reset value |
|:-------------:|:-----------:|
| 0x5E8 + N * 8 | 0x0000_0000 |

Fields:

| Bits |      31 - 0      |
|:----:|:----------------:|
| Mode |        R         |
| Name | ended_outs[31-0] |

- LSBs of all ended status bits.

#### Result available status high

|    Offset     | Reset value |
|:-------------:|:-----------:|
| 0x5EC + N * 8 | 0x0000_0000 |

Fields:

| Bits | 31-29 |          28 - 0          |
|:----:|:-----:|:------------------------:|
| Mode |       |            R             |
| Name |  RFU  | results_available[60-32] |

- MSBs of all result available status bits.

#### Result available status low

|    Offset     | Reset value |
|:-------------:|:-----------:|
| 0x5F0 + N * 8 | 0x0000_0000 |

Fields:

| Bits |         31 - 0          |
|:----:|:-----------------------:|
| Mode |            R            |
| Name | results_available[31-0] |

- LSBs of all result available status bits.

#### Result full status high

|    Offset     | Reset value |
|:-------------:|:-----------:|
| 0x5F4 + N * 8 | 0x0000_0000 |

Fields:

| Bits | 31-29 |       28 - 0        |
|:----:|:-----:|:-------------------:|
| Mode |       |          R          |
| Name |  RFU  | results_full[60-32] |

- MSBs of all result full status bits.

#### Result full status low

|    Offset     | Reset value |
|:-------------:|:-----------:|
| 0x5F8 + N * 8 | 0x0000_0000 |

Fields:

| Bits |       31 - 0       |
|:----:|:------------------:|
| Mode |         R          |
| Name | results_full[31-0] |

- LSBs of all result full status bits.

#### Match result N high

|    Offset     | Reset value |
|:-------------:|:-----------:|
| 0x5FC + N * 8 | 0x0000_0000 |

Fields:

| Bits |      31      | 30-24 |       23 - 0        |
|:----:|:------------:|:-----:|:-------------------:|
| Mode |      R       |       |          R          |
| Name | match_number |  RFU  | matched_keyN[55-32] |

- match_number: 0 if key matched with mask1/ref1, 1 if it matched with mask2/ref2
- matched_keyN: MSBs of matched key in Nth worker.

#### Match result N low

|    Offset     | Reset value |
|:-------------:|:-----------:|
| 0x600 + N * 8 | 0x0000_0000 |

Fields:

| Bits |       31 - 0       |
|:----:|:------------------:|
| Mode |         R          |
| Name | matched_keyN[31-0] |

- matched_keyN: LSBs of matched key in Nth worker.