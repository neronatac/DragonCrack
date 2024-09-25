# Notes on versions of DragonCrack

## V0 - Proof of concept

Based on DEScracker IP v1.0.

A single DES worker is implemented. Implementation is as naive as possible (even dumb on certain aspects).

Most important facts about the IP:
- Plaintext, 2 references and 2 masks are defined globally.
- It groups multiple parallel workers. Each worker is independent and have its own start key and end key. It exhausts 
all keys between them one by one.
- Each worker relies on its own registers to set parameters (start/end key, etc.) and output results. 505 registers are
defined in the AXI registers model. It takes a lot of resources but this version is meant to be naive.
- Workers work using the same clock as the one used for AXI bus. It is set to 8MHz in this version to avoid timing 
problems.
- When a worker finds a result that matches the reference and the mask, it is stopped until the result is read. The 
result consists in the 56-bit key and a bit that indicates against which reference it matches.
- In workers, DES is done in 1 clock cycle: all combinational, no pipeline
- Python driver polls continuously result registers to get pending results.
- In Vivado, default synthesis and implementation are used.
- In Vitis, the program just execute simple commands received from Python driver. Commands just consist in reading or
writing a single register.

Notes on resources usage (after implementation) of IP:

| Block  | Number of LUTs | Number of FlipFlops |
|:------:|:--------------:|:-------------------:|
| Total  |      8116      |        16550        |
|  AXI   |      6582      |        16375        |
| worker |      961       |         175         |


## V1 - First real implementation

The goal is to set the working frequency of workers at its maximum value with no negative slack and also to instantiate
as many workers as possible. Indeed, when usage of FPGA resources will rise, timing will be harder to maintain.

First results:

| Working frequency (MHz) | Number of workers | Theoretical keys/s | Worst negative slack (ns) |   LUT usage   |   FF usage    |
|:-----------------------:|:-----------------:|:------------------:|:-------------------------:|:-------------:|:-------------:|
|           25            |         3         |        75 M        |           1.064           | 23.9% (12721) | 16.7% (17819) |
|           20            |        28         |       560 M        |           1.43            | 96.7% (51482) | 20.9% (22192) |

By doing some tests, it appeared that on a single worker, the maximum frequency is around 27 MHz. With 28 workers,
frequency could probably be optimised but compilation take 30 min and I did not have enough patience to do multiple
compilations.

**Note:** for some reason, a bug in IP customisation tool blocked the setting of FCLK_CLKx frequency... From now on, 
advanced setting ("override clocks" mode) is used.

Notes on resources usage (after implementation) of IP:

| Block  |    Number of LUTs     | Number of FlipFlops |
|:------:|:---------------------:|:-------------------:|
| Total  |         50802         |        21283        |
|  AXI   |         6879          |        16393        |
| worker | between 1300 and 2300 |         175         |

In this configuration, each worker is implemented using a different number of LUTs.