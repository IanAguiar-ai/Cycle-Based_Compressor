// Cycle-Based Compressor
// Ian dos Anjos Melo Aguiar
// First version: 9/12/2025
//
// Basic implementation of the cycle-based compressor (0^m 1^j)
// with header [K][s1]...[sK] + bit payload.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ------------------------------------------------------------
// Constants
#define MAX_SIZE_MESSAGE 1024
#define ALPHABET_SIZE 256   // ASCII

// ------------------------------------------------------------
// Structures

// Entry in the code table: symbol, frequency, pair (m, j)
typedef struct {
    unsigned char symbol;
    int freq;
    int m;
    int j;
} CodeEntry;

// Bit writer for the compressed payload
typedef struct {
    unsigned char *data;
    int capacity;   // capacity in bytes
    int size;       // bytes actually used
    int bit_pos;    // next bit position (0..7) inside the last byte
} BitWriter;

// ------------------------------------------------------------
// BitWriter helpers
void bw_init(BitWriter *bw, int capacity) {
    bw->data = (unsigned char *)malloc(capacity);
    bw->capacity = capacity;
    bw->size = 0;
    bw->bit_pos = 0;
    if (bw->data) {
        memset(bw->data, 0, capacity);
    }
}

void bw_free(BitWriter *bw) {
    if (bw->data) free(bw->data);
    bw->data = NULL;
    bw->capacity = 0;
    bw->size = 0;
    bw->bit_pos = 0;
}

// Ensure we have space for at least one more byte
static void bw_ensure_space(BitWriter *bw) {
    if (bw->size >= bw->capacity) {
        int new_cap = bw->capacity * 2;
        if (new_cap < 16) new_cap = 16;
        unsigned char *new_data = (unsigned char *)realloc(bw->data, new_cap);
        if (!new_data) {
            // simple error handling (example code)
            fprintf(stderr, "Error in realloc inside BitWriter\n");
            exit(1);
        }
        // zero newly allocated space
        memset(new_data + bw->capacity, 0, new_cap - bw->capacity);
        bw->data = new_data;
        bw->capacity = new_cap;
    }
}

void bw_put_bit(BitWriter *bw, int bit) {
    if (bw->size == 0 && bw->bit_pos == 0) {
        // start a new byte
        bw_ensure_space(bw);
        bw->size = 1;
        bw->data[0] = 0;
    }

    if (bw->bit_pos == 8) {
        // need a new byte
        bw_ensure_space(bw);
        bw->data[bw->size] = 0;
        bw->size++;
        bw->bit_pos = 0;
    }

    if (bit) {
        bw->data[bw->size - 1] |= (1 << (7 - bw->bit_pos));
    }
    bw->bit_pos++;
}

// Write the cycle 0^m 1^j
void bw_put_cycle(BitWriter *bw, int m, int j) {
    for (int i = 0; i < m; i++) {
        bw_put_bit(bw, 0);
    }
    for (int i = 0; i < j; i++) {
        bw_put_bit(bw, 1);
    }
}

// ------------------------------------------------------------
// Frequency counting
// ------------------------------------------------------------

void count_character_frequency(const char *text, int *freq_table) {
    int length = (int)strlen(text);
    for (int i = 0; i < length; i++) {
        unsigned char c = (unsigned char)text[i];
        freq_table[c]++;
        // optional debug
        // printf("%d: %c (freq: %d)\n", i, text[i], freq_table[c]);
    }
}

// ------------------------------------------------------------
// Sorting by frequency (desc) and symbol (asc)
// ------------------------------------------------------------

int compare_codeentry(const void *a, const void *b) {
    const CodeEntry *ca = (const CodeEntry *)a;
    const CodeEntry *cb = (const CodeEntry *)b;

    if (ca->freq > cb->freq) return -1;  // higher frequency first
    if (ca->freq < cb->freq) return 1;
    // if same frequency, order by symbol code
    if (ca->symbol < cb->symbol) return -1;
    if (ca->symbol > cb->symbol) return 1;
    return 0;
}

// ------------------------------------------------------------
// Cycle generation (m, j) for codes
//
// Order: lengths L = 2, 3, 4, ...
// For each length L, pairs (m, j) = (L-1,1), (L-2,2), ..., (1,L-1)
//
// Example: L=2 => (1,1)  -> "01"
//          L=3 => (2,1), (1,2)  -> "001", "011"
//          L=4 => (3,1), (2,2), (1,3) -> ...
// ------------------------------------------------------------

void generate_cycles_for_codes(CodeEntry *codes, int K) {
    int count = 0;
    int L = 2;

    while (count < K) {
        for (int m = L - 1; m >= 1 && count < K; m--) {
            int j = L - m;
            codes[count].m = m;
            codes[count].j = j;
            count++;
        }
        L++;
    }
}

// ------------------------------------------------------------
// Compression
//
// Input:
//   text      - C string (null terminated)
// Output:
//   out_data  - pointer to allocated compressed buffer (malloc)
//   out_size  - size in bytes of the compressed buffer
//
// Compressed format:
//   [1 byte: K] [K bytes: ordered symbols] [bit payload]
//
// Header cost is K + 1 bytes.
// ------------------------------------------------------------

void compress_cycle_based(const char *text,
                          unsigned char **out_data,
                          int *out_size) {
    int freq_table[ALPHABET_SIZE] = {0};
    count_character_frequency(text, freq_table);

    // Build the table of symbols with freq > 0
    CodeEntry codes[ALPHABET_SIZE];
    int K = 0;
    for (int c = 0; c < ALPHABET_SIZE; c++) {
        if (freq_table[c] > 0) {
            codes[K].symbol = (unsigned char)c;
            codes[K].freq = freq_table[c];
            codes[K].m = 0;
            codes[K].j = 0;
            K++;
        }
    }

    if (K == 0) {
        // empty text
        *out_data = NULL;
        *out_size = 0;
        return;
    }

    if (K > 255) {
        // for this example, we limit K to 255
        fprintf(stderr, "Error: more than 255 distinct symbols\n");
        exit(1);
    }

    // Sort by decreasing frequency
    qsort(codes, K, sizeof(CodeEntry), compare_codeentry);

    // Generate pairs (m, j) for each symbol in the given order
    generate_cycles_for_codes(codes, K);

    // Build LUT: symbol -> index in codes
    int lut[ALPHABET_SIZE];
    for (int i = 0; i < ALPHABET_SIZE; i++) lut[i] = -1;
    for (int i = 0; i < K; i++) {
        lut[codes[i].symbol] = i;
    }

    // Write bit payload
    BitWriter bw;
    bw_init(&bw, 64);  // small initial capacity, grows dynamically

    int len = (int)strlen(text);
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)text[i];
        int idx = lut[c];
        if (idx < 0) {
            fprintf(stderr, "Internal error: symbol not found in LUT\n");
            exit(1);
        }
        bw_put_cycle(&bw, codes[idx].m, codes[idx].j);
    }

    // Build final buffer: [K][symbols][payload bits]
    int header_size = 1 + K;  // 1 byte K + K symbol bytes
    *out_size = header_size + bw.size;
    *out_data = (unsigned char *)malloc(*out_size);

    if (!*out_data) {
        fprintf(stderr, "Error allocating output buffer\n");
        exit(1);
    }

    // Fill header
    (*out_data)[0] = (unsigned char)K;
    for (int i = 0; i < K; i++) {
        (*out_data)[1 + i] = codes[i].symbol;
    }

    // Copy payload
    memcpy(*out_data + header_size, bw.data, bw.size);

    bw_free(&bw);

    // Optional debug
    printf("K = %d (header = %d bytes, payload = %d bytes, total = %d bytes)\n",
           K, header_size, *out_size - header_size, *out_size);
}

// ------------------------------------------------------------
// Decompression (simple version)
// ------------------------------------------------------------
//
// Input:
//   comp_data    - compressed buffer
//   comp_size    - compressed buffer size in bytes
//   original_len - original text length in characters
//
// Output:
//   out_text     - output buffer (must have space >= original_len+1)
//
// Note: here we assume the receiver knows the original length (it could be
//       transmitted in another protocol field, for example).
// ------------------------------------------------------------

void decompress_cycle_based(const unsigned char *comp_data,
                            int comp_size,
                            int original_len,
                            char *out_text) {
    if (comp_size <= 0) {
        out_text[0] = '\0';
        return;
    }

    int K = comp_data[0];
    if (K <= 0 || comp_size < 1 + K) {
        fprintf(stderr, "Invalid compressed data\n");
        out_text[0] = '\0';
        return;
    }

    // Recover symbols
    CodeEntry codes[ALPHABET_SIZE];
    for (int i = 0; i < K; i++) {
        codes[i].symbol = comp_data[1 + i];
        codes[i].m = 0;
        codes[i].j = 0;
        codes[i].freq = 0;  // not needed here
    }

    // Generate the same cycles (m, j) in the same order
    generate_cycles_for_codes(codes, K);

    // Pointer to the bit payload
    const unsigned char *payload = comp_data + 1 + K;
    int payload_bytes = comp_size - (1 + K);

    int bit_index = 0;  // global bit index
    int out_pos = 0;

    while (out_pos < original_len && bit_index < payload_bytes * 8) {
        // count zeros until the first 1
        int m = 0;
        int j = 0;

        // count zeros
        while (bit_index < payload_bytes * 8) {
            int byte_idx = bit_index / 8;
            int bit_in_byte = bit_index % 8;
            int bit = (payload[byte_idx] >> (7 - bit_in_byte)) & 1;
            bit_index++;

            if (bit == 0) {
                m++;
            } else {
                // found first 1
                j = 1;
                break;
            }
        }

        if (j == 0) {
            // did not find a 1, truncated stream
            break;
        }

        // count consecutive 1s
        while (bit_index < payload_bytes * 8) {
            int byte_idx = bit_index / 8;
            int bit_in_byte = bit_index % 8;
            int bit = (payload[byte_idx] >> (7 - bit_in_byte)) & 1;

            if (bit == 1) {
                j++;
                bit_index++;
            } else {
                // found 0, end of cycle
                break;
            }
        }

        // now we have (m, j) -> search symbol
        int found = -1;
        for (int i = 0; i < K; i++) {
            if (codes[i].m == m && codes[i].j == j) {
                found = i;
                break;
            }
        }

        if (found < 0) {
            fprintf(stderr, "Pair (m=%d, j=%d) not found in code table\n", m, j);
            break;
        }

        out_text[out_pos++] = (char)codes[found].symbol;
    }

    out_text[out_pos] = '\0';
}

// ------------------------------------------------------------
// Simple test
// ------------------------------------------------------------

int main(void) {
    // Full original text (em ingles, como no artigo)
    const char *full_text = "In wireless sensor networks, the energy cost of transmitting a single byte is often far higher than the cost of executing hundreds or even thousands of local instructions. As a consequence, lightweight compression techniques are essential for extending device lifetime and reducing network congestion. A deterministic low-overhead compressor allows embedded devices to reduce traffic without adding excessive computational complexity to firmware. Modern IoT systems often operate under strict limitations: restricted memory, low clock frequencies, intermittent connectivity, and energy budgets that must last months or years. Under these conditions, traditional compression algorithms may introduce too much overhead or require dynamic structures that are unsuitable for constrained nodes. A predictable, prefix-free, cycle-based scheme provides a promising alternative by minimizing header cost and avoiding the reconstruction of probability models during decoding.";

    size_t full_len = strlen(full_text);
    int target_sizes[] = {32, 64, 128, 256, 512};
    int num_sizes = sizeof(target_sizes) / sizeof(target_sizes[0]);

    printf("Full original length: %zu bytes\n\n", full_len);

    for (int t = 0; t < num_sizes; t++) {
        int S = target_sizes[t];

        // Skip if the full text is smaller than the target size
        if (full_len < (size_t)S) {
            continue;
        }

        // Build truncated message
        char example[MAX_SIZE_MESSAGE];
        if (S >= MAX_SIZE_MESSAGE) {
            fprintf(stderr, "Target size %d exceeds MAX_SIZE_MESSAGE=%d\n",
                    S, MAX_SIZE_MESSAGE);
            continue;
        }

        memcpy(example, full_text, S);
        example[S] = '\0';

        unsigned char *compressed = NULL;
        int comp_size = 0;

        printf("=== Truncated to %d bytes ===\n", S);
        printf("Original (first %d bytes): \"%s\"\n", S, example);

        // Compress
        compress_cycle_based(example, &compressed, &comp_size);
        printf("Compressed size: %d bytes\n", comp_size);

        // Decompress (we pass S as the original length)
        char recovered[MAX_SIZE_MESSAGE];
        decompress_cycle_based(compressed, comp_size, S, recovered);

        printf("Recovered: \"%s\"\n\n", recovered);

        free(compressed);
    }

    return 0;
}

