"""
Cycled-Based Compressor
Ian dos Anjos Melo Aguiar
"""

# Functions:

def compress(text:str, *, save:str = None, verbose:bool = False) -> str:
    print(f"\n{'-'*10}(Compression Cycled-Based Compressor){'-'*10}") if verbose else None
    print(f"\n{text = }") if verbose else None
    
    freq_tab = frequenci_table(text)
    print(f"\n{freq_tab = }") if verbose else None
    symb_tab = symbol_table(freq_tab)
    print(f"\n{symb_tab = }") if verbose else None

    compress_symbolic:str = ""
    for t in text:
        compress_symbolic += symb_tab[t]

    while len(compress_symbolic)%8 != 0:
        compress_symbolic += "0"
    print(f"\n{compress_symbolic = }") if verbose else None

    header = ''.join(list(symb_tab.keys())) + "§"
    print(f"\n{header = }") if verbose else None

    compress_message:str = bytearray(header.encode("utf-8"))
    for i in range(0, len(compress_symbolic), 8):
        byte_val = int(compress_symbolic[i:i+8], 2)
        compress_message.append(byte_val)

    print(f"\n{compress_message = }") if verbose else None
    print(f"\n{len(header)}(header) + {len(compress_symbolic)//8}(text compressed) = {len(compress_message)}") if verbose else None

    if save != None:
        with open(save, "wb") as arq:
            arq.write(compress_message)
    return compress_message
    

def decompress(text:str = None, *, read:str = None, verbose:bool = False) -> str:
    print(f"\n{'-'*10}(Decompression Cycled-Based Compressor){'-'*10}") if verbose else None

    if text == None and read != None:
        print(f"Read file: {read}") if verbose else None
        with open(read, "rb") as arq:
            text = arq.read()
    
    for sep, b in enumerate(text):
        if b == ord("§"):
            break
    header, text = text[:sep - 1], text[sep+1:]

    freq_tab:dict = [(chr(key), None) for key in header]
    symb_tab:dict = symbol_table(freq_tab)
    print(f"\n{symb_tab = }") if verbose else None
    inv_symb_tab:dict = {value:key for key, value in symb_tab.items()}

    compress_symbolic:str = ""
    for t in text:
        bits_temp:str = str(bin(t))[2:]
        if len(bits_temp) < 8:
            bits_temp = "0"*(8 - len(bits_temp)) + bits_temp
        compress_symbolic += bits_temp
    print(f"\n{compress_symbolic = }") if verbose else None

    real_text = ""
    print(f"\nRead: ", end = "") if verbose else None
    while True:
        sep:int = compress_symbolic.find("10")
        if len(compress_symbolic) == 0 or ((sep == -1) and (compress_symbolic[-1] != "1")): # The text end
            break
        elif (sep == -1) and compress_symbolic[-1] == "1": # When the text end with not change to 10
            sep:int = len(compress_symbolic)
        bit_part:str = compress_symbolic[:sep + 1]
        compress_symbolic = compress_symbolic[sep+1:]
        real_text += inv_symb_tab[bit_part]
        print(f"{bit_part}({inv_symb_tab[bit_part]})", end = " ") if verbose else None
    print(f"\n\n{real_text = }") if verbose else None
    return real_text


def bitpattern(m:int, j:int) -> str:
    """
    bitparttern = 0^m 1^j where m >= 1 and j >= 1
    """
    return "0"*m + "1"*j

def frequenci_table(text:str) -> dict:
    freq_tab:dict = {}
    for t in text:
        if not t in freq_tab:
            freq_tab[t] = 0
        freq_tab[t] += 1
    return sorted(freq_tab.items(), key = lambda x:x[1], reverse = True)

def symbol_table(frequenci_table:list[tuple]) -> dict:
    symb_tab:dict = {}
    j, max_ = 1, 2
    for key, _ in frequenci_table:
        symb_tab[key] = bitpattern(m = max_-j, j = j)
        j += 1
        if j >= max_:
            j, max_ = 1, max_ + 1
    return symb_tab


if __name__ == "__main__":
    texto = """In wireless sensor networks, the energy cost of transmitting a single byte is often far higher than the cost of executing hundreds or even thousands of local instructions. As a consequence, lightweight compression techniques are essential for extending device lifetime and reducing network congestion. A deterministic low-overhead compressor allows embedded devices to reduce traffic without adding excessive computational complexity to firmware. Modern IoT systems often operate under strict limitations: restricted memory, low clock frequencies, intermittent connectivity, and energy budgets that must last months or years. Under these conditions, traditional compression algorithms may introduce too much overhead or require dynamic structures that are unsuitable for constrained nodes. A predictable, prefix-free, cycle-based scheme provides a promising alternative by minimizing header cost and avoiding the reconstruction of probability models during decoding."""
    
    for n in [32, 64, 128, 256, 512]:
        c = compress(texto[:n], save = f"teste_{n:03}", verbose = True)
        d = decompress(read = f"teste_{n:03}", verbose = True)

        if texto[:n] == d:
            print(f"\n\tSucess Compression\n\n\n")
