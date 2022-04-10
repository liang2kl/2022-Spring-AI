from pinyin import get_pinyin_map
from stats import get_stats_map
from utils import get_prob, parse_args
import math

INPUT_FILE = "./data/input.txt"
LAMBDA = 0.9

if __name__ == "__main__":
    args = parse_args(False)
    LAMBDA = args._lambda if args._lambda != None else LAMBDA

    pinyin_map = get_pinyin_map()
    stats_map = get_stats_map()[0]

    result = ""

    with open(args.input_file, "r") as file:
        for line in file:
            pinyins = line.split()

            char_num = len(pinyins)
            values = [{} for _ in range(char_num)]

            for i, pinyin in enumerate(pinyins):
                chars = pinyin_map[pinyin]
                for char in chars:
                    stats_map.setdefault(char, {"start": 0, "total": 0})
                    values[i].setdefault(char, {"probs": 10000, "parent": ""})
                
                total_curr = sum([stats_map[c][("total" if i > 0 else "start")] for c in chars])

                if i == 0:
                    for c in chars:
                        p = stats_map[c]["start"] / total_curr
                        values[i][c]["probs"] = -math.log(p) if p > 0 else 10000
                else:
                    prev_chars = pinyin_map[pinyins[i - 1]]
                    for char in chars:
                        for prev_char in prev_chars:
                            prob = get_prob(
                                stats_map, prev_char, char, total_curr, LAMBDA)

                            val = values[i - 1][prev_char]["probs"] + prob

                            if val < values[i][char]["probs"]:
                                values[i][char]["probs"] = val
                                values[i][char]["parent"] = prev_char
            
            last_layer = values[char_num - 1]
            last_char = min(
                last_layer, key=lambda k: last_layer[k]["probs"])
            
            res_str = last_char

            for i in range(char_num - 1):
                layer = char_num - 1 - i
                last_char = values[layer][last_char]["parent"]
                res_str = last_char + res_str
            result += res_str + "\n"

    
    with open(args.output, "w") as file:
        file.write(result)
    