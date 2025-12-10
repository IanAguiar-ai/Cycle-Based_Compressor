# Cycle-Based Compressor – Reference Implementations

This repository provides C and Python reference implementations of the cycle-based compressor, a deterministic, prefix-free coding scheme designed for short IoT payloads and resource-constrained devices. The code accompanies the paper:

*Cycle-Based Compressor* (link in future)

The compressor assigns each symbol to a bit pattern of the form $0^m 1^j$ (with $m ≥ 1$, $j ≥ 1$), ordered by empirical frequency. This structure yields:

- Prefix-free codes with no explicit tree;
- Very small, deterministic headers;
- Constant-time decoding;
- Strong performance for payloads between 32 and 512 bytes, typical of IoT telemetry.

## Repository Structure

- ```cycle_based_compressor.c```:
  C implementation with a demo program that compresses and decompresses example messages.

- ```cycle_based_compressor.py```:
  Python implementation with compress and decompress functions, plus an educational verbose mode.

- *Cycle-Based Compressor* (link in future):
  Full paper describing the algorithm, theoretical properties, and experiments.

## Algorithm Summary

1. Count symbol frequencies.
2. Sort symbols by decreasing frequency.
3. Assign cycle codes in the deterministic sequence:
   ```01, 001, 011, 0001, 0011, 0111, 00001, 00011, ...```
4. Construct the header with the ordered list of symbols.
5. Encode the payload by concatenating the corresponding bit patterns.

Python format:
[symbols][§ separator][bit payload]

C format:
[K][K symbols][bit payload]
(K = number of distinct symbols)

## Using the C Implementation

### Build

```
gcc cycle_based_compressor.c -o cbc_demo
```

### Run

```
./cbc_demo
```

The demo prints the original text, compressed size, and decompressed output for different truncation lengths.

### API (C)

```c
void compress_cycle_based(const char *text,
                          unsigned char **out_data,
                          int *out_size);

void decompress_cycle_based(const unsigned char *data,
                            int data_size,
                            int original_len,
                            char *out_text);
```

Example:

```c
#include <stdio.h>
#include <stdlib.h>
#include "cycle_based_compressor.c"

int main(void) {
    const char *msg = "Hello, cycle-based compressor!";
    unsigned char *compressed = NULL;
    int comp_size = 0;

    compress_cycle_based(msg, &compressed, &comp_size);

    char recovered[1024];
    decompress_cycle_based(compressed, comp_size,
                           (int)strlen(msg), recovered);

    printf("Original:  %s\n", msg);
    printf("Recovered: %s\n", recovered);
    printf("Compressed size: %d bytes\n", comp_size);

    free(compressed);
    return 0;
}
```

## Using the Python Implementation

### Example

```
python3 cycle_based_compressor.py
```

### API (Python)

```python
from cycle_based_compressor import compress, decompress

compress(text: str, save: str = None, verbose: bool = False) -> bytes  
decompress(text: bytes = None, read: str = None, verbose: bool = False) -> str
```

Usage:

```
from cycle_based_compressor import compress, decompress

msg = "In wireless sensor networks, energy cost matters."

compressed = compress(msg, save="example.cbc")
print(len(compressed))

recovered = decompress(read="example.cbc")
print(recovered)
```

Other Example:

```python
from cycle_based_compressor import compress, decompress

text = """In wireless sensor networks, the energy cost of transmitting a single byte is often far higher than the cost of executing hundreds or even thousands of local instructions. As a consequence, lightweight compression techniques are essential for extending device lifetime and reducing network congestion."""
    
c = compress(text[:256], save = f"test", verbose = True)
d = decompress(read = f"test", verbose = True)
```

Output:

```
----------(Compression Cycled-Based Compressor)----------

text = 'In wireless sensor networks, the energy cost of transmitting a single byte is often far higher than the cost of executing hundreds or even thousands of local instructions. As a consequence, lightweight compression techniques are essential for extending dev'

freq_tab = [(' ', 38), ('e', 30), ('n', 22), ('s', 22), ('t', 20), ('o', 16), ('i', 15), ('r', 13), ('h', 10), ('c', 9), ('a', 9), ('g', 8), ('l', 6), ('f', 6), ('u', 6), ('d', 5), ('w', 3), (',', 2), ('y', 2), ('m', 2), ('x', 2), ('v', 2), ('q', 2), ('I', 1), ('k', 1), ('b', 1), ('.', 1), ('A', 1), ('p', 1)]

symb_tab = {' ': '01', 'e': '001', 'n': '011', 's': '0001', 't': '0011', 'o': '0111', 'i': '00001', 'r': '00011', 'h': '00111', 'c': '01111', 'a': '000001', 'g': '000011', 'l': '000111', 'f': '001111', 'u': '011111', 'd': '0000001', 'w': '0000011', ',': '0000111', 'y': '0001111', 'm': '0011111', 'x': '0111111', 'v': '00000001', 'q': '00000011', 'I': '00000111', 'k': '00001111', 'b': '00011111', '.': '00111111', 'A': '01111111', 'p': '000000001'}

compress_symbolic = '000001110110100000110000100011001000111001000100010100010010110001011100011010110010011000001101110001100001111000100001110100110011100101001011001000110000110001111010111101110001001101011100111101001100011000001011000100111110000100110011000010110000110100000101000100001011000011000111001010001111100011110011001010000100010101110011110011001011010011110000010001101001110000100001100111001000110100110011100000101101001100111001010111101110001001101011100111101001011111100101111011111001100001011000011010011101111101100000010001100100000010001010111000110100100000001001011010011001110111011111000100000101100000010001010111001111010001110111011110000010001110100001011000100110001101111101111001100001011101100010011111101011111110001010000010101111011101100010010000001101111100101101111001000011101000111000010000110011100110000011001000010000110011100110101111011100111110000000010001100100010001000010111011010011001011110011101100001000000110111110010001010000010001100101001000100010010110011000010000010001110100111101110001101001011111100110010110000001000010110000110100000010010000000100'

header = ' enstoirhcaglfudw,ymxvqIkb.Ap§'

compress_message = bytearray(b' enstoirhcaglfudw,ymxvqIkb.Ap\xc2\xa7\x07h0\x8c\x8eDQ,\\k&\r\xc6\x1e!\xd39K#\x0cz\xf7\x13\\\xf4\xc6\x0b\x13\xe13\x0b\r\x05\x10\xb0\xc7(\xf8\xf3(Es\xcc\xb4\xf0F\x9c!\x9c\x8d3\x82\xd39^\xe2k\x9e\x97\xe5\xef\x98Xi\xdf`F@\x8a\xe3H\ti\x9d\xdf\x10X\x11\\\xf4wx#\xa1bc}\xe6\x17b~\xbf\x8a\n\xf7b@\xdf-\xe4:8C9\x83!\x0c\xe6\xbd\xcf\x80FDB\xed2\xf3\xb0\x81\xbeE\x04e"%\x98A\x1d=\xc6\x97\xe6X\x10\xb0\xd0$\x04')

30(header) + 138(text compressed) = 169 Bytes

----------(Decompression Cycled-Based Compressor)----------
Read file: test

symb_tab = {' ': '01', 'e': '001', 'n': '011', 's': '0001', 't': '0011', 'o': '0111', 'i': '00001', 'r': '00011', 'h': '00111', 'c': '01111', 'a': '000001', 'g': '000011', 'l': '000111', 'f': '001111', 'u': '011111', 'd': '0000001', 'w': '0000011', ',': '0000111', 'y': '0001111', 'm': '0011111', 'x': '0111111', 'v': '00000001', 'q': '00000011', 'I': '00000111', 'k': '00001111', 'b': '00011111', '.': '00111111', 'A': '01111111', 'p': '000000001'}

compress_symbolic = '000001110110100000110000100011001000111001000100010100010010110001011100011010110010011000001101110001100001111000100001110100110011100101001011001000110000110001111010111101110001001101011100111101001100011000001011000100111110000100110011000010110000110100000101000100001011000011000111001010001111100011110011001010000100010101110011110011001011010011110000010001101001110000100001100111001000110100110011100000101101001100111001010111101110001001101011100111101001011111100101111011111001100001011000011010011101111101100000010001100100000010001010111000110100100000001001011010011001110111011111000100000101100000010001010111001111010001110111011110000010001110100001011000100110001101111101111001100001011101100010011111101011111110001010000010101111011101100010010000001101111100101101111001000011101000111000010000110011100110000011001000010000110011100110101111011100111110000000010001100100010001000010111011010011001011110011101100001000000110111110010001010000010001100101001000100010010110011000010000010001110100111101110001101001011111100110010110000001000010110000110100000010010000000100'

Read: 00000111(I) 011(n) 01( ) 0000011(w) 00001(i) 00011(r) 001(e) 000111(l) 001(e) 0001(s) 0001(s) 01( ) 0001(s) 001(e) 011(n) 0001(s) 0111(o) 00011(r) 01( ) 011(n) 001(e) 0011(t) 0000011(w) 0111(o) 00011(r) 00001111(k) 0001(s) 0000111(,) 01( ) 0011(t) 00111(h) 001(e) 01( ) 001(e) 011(n) 001(e) 00011(r) 000011(g) 0001111(y) 01( ) 01111(c) 0111(o) 0001(s) 0011(t) 01( ) 0111(o) 001111(f) 01( ) 0011(t) 00011(r) 000001(a) 011(n) 0001(s) 0011111(m) 00001(i) 0011(t) 0011(t) 00001(i) 011(n) 000011(g) 01( ) 000001(a) 01( ) 0001(s) 00001(i) 011(n) 000011(g) 000111(l) 001(e) 01( ) 00011111(b) 0001111(y) 0011(t) 001(e) 01( ) 00001(i) 0001(s) 01( ) 0111(o) 001111(f) 0011(t) 001(e) 011(n) 01( ) 001111(f) 000001(a) 00011(r) 01( ) 00111(h) 00001(i) 000011(g) 00111(h) 001(e) 00011(r) 01( ) 0011(t) 00111(h) 000001(a) 011(n) 01( ) 0011(t) 00111(h) 001(e) 01( ) 01111(c) 0111(o) 0001(s) 0011(t) 01( ) 0111(o) 001111(f) 01( ) 001(e) 0111111(x) 001(e) 01111(c) 011111(u) 0011(t) 00001(i) 011(n) 000011(g) 01( ) 00111(h) 011111(u) 011(n) 0000001(d) 00011(r) 001(e) 0000001(d) 0001(s) 01( ) 0111(o) 00011(r) 01( ) 001(e) 00000001(v) 001(e) 011(n) 01( ) 0011(t) 00111(h) 0111(o) 011111(u) 0001(s) 000001(a) 011(n) 0000001(d) 0001(s) 01( ) 0111(o) 001111(f) 01( ) 000111(l) 0111(o) 01111(c) 000001(a) 000111(l) 01( ) 00001(i) 011(n) 0001(s) 0011(t) 00011(r) 011111(u) 01111(c) 0011(t) 00001(i) 0111(o) 011(n) 0001(s) 00111111(.) 01( ) 01111111(A) 0001(s) 01( ) 000001(a) 01( ) 01111(c) 0111(o) 011(n) 0001(s) 001(e) 00000011(q) 011111(u) 001(e) 011(n) 01111(c) 001(e) 0000111(,) 01( ) 000111(l) 00001(i) 000011(g) 00111(h) 0011(t) 0000011(w) 001(e) 00001(i) 000011(g) 00111(h) 0011(t) 01( ) 01111(c) 0111(o) 0011111(m) 000000001(p) 00011(r) 001(e) 0001(s) 0001(s) 00001(i) 0111(o) 011(n) 01( ) 0011(t) 001(e) 01111(c) 00111(h) 011(n) 00001(i) 00000011(q) 011111(u) 001(e) 0001(s) 01( ) 000001(a) 00011(r) 001(e) 01( ) 001(e) 0001(s) 0001(s) 001(e) 011(n) 0011(t) 00001(i) 000001(a) 000111(l) 01( ) 001111(f) 0111(o) 00011(r) 01( ) 001(e) 0111111(x) 0011(t) 001(e) 011(n) 0000001(d) 00001(i) 011(n) 000011(g) 01( ) 0000001(d) 001(e) 00000001(v) 

real_text = 'In wireless sensor networks, the energy cost of transmitting a single byte is often far higher than the cost of executing hundreds or even thousands of local instructions. As a consequence, lightweight compression techniques are essential for extending dev'
```

### Debug Tools

```
from cycle_based_compressor import frequenci_table, symbol_table, bitpattern
```

## Reproducibility

These implementations are minimal and aligned with the paper so researchers can reproduce experiments, test payload sizes (32–512 bytes), and integrate the method into IoT systems.
