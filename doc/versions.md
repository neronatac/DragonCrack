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