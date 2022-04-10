SRC_FILE = "./data/pinyin.txt"

def get_pinyin_map():
    pinyin = {}
    with open(SRC_FILE) as file:
        for line in file:
            strs = line.split()
            pinyin[strs[0]] = strs[1:]

    return pinyin