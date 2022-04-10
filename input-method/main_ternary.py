from pinyin import get_pinyin_map
from stats import get_stats_map
from utils import get_prob_ter, parse_args
import math

INPUT_FILE = "./data/input.txt"

if __name__ == "__main__":
    args = parse_args(True)
    _lambda = args._lambda
    alpha = args.alpha

    pinyin_map = get_pinyin_map()
    bin_stats_map, ter_stats_map = get_stats_map()

    result = ""

    with open(args.input_file, "r") as file:
        for line in file:
            pinyins = line.split()

            char_num = len(pinyins)
            values = [{} for _ in range(char_num)]

            for i, pinyin in enumerate(pinyins):
                chars = pinyin_map[pinyin]
                for char in chars:
                    bin_stats_map.setdefault(char, {"start": 0, "total": 0})
                
                total_curr = sum([bin_stats_map[c][("total" if i > 0 else "start")] for c in chars])

                if i == 0:
                    for c in chars:
                        word = "\0" + c
                        p = bin_stats_map[c]["start"] / total_curr
                        ter_stats_map.setdefault(word,
                            {"start": bin_stats_map[c]["start"], "total": bin_stats_map[c]["total"]})
                        values[i].setdefault(word, {"probs": 10000, "parent": ""})
                        values[i][word]["probs"] = -math.log(p) if p > 0 else 10000
                else:
                    prev_chars = pinyin_map[pinyins[i - 1]]
                    if i == 1:
                        prev_prev_chars = ["\0"]
                    else:
                        prev_prev_chars = pinyin_map[pinyins[i - 2]]
                    
                    for prev_prev_char in prev_prev_chars:
                        for prev_char in prev_chars:
                            
                            prev_word = prev_prev_char + prev_char
                            
                            for char in chars:
                                curr_word = prev_char + char
                                
                                ter_stats_map.setdefault(prev_word, {"start": 0, "total": 0})

                                prob = get_prob_ter(
                                    bin_stats_map, ter_stats_map, prev_word,
                                    char, total_curr, _lambda, alpha)
                                
                                values[i - 1].setdefault(prev_word, {"probs": 10000, "parent": ""})
                                values[i].setdefault(curr_word, {"probs": 10000, "parent": ""})
                                
                                val = values[i - 1][prev_word]["probs"] + prob

                                if val < values[i][curr_word]["probs"]:
                                    values[i][curr_word]["probs"] = val
                                    values[i][curr_word]["parent"] = prev_word
            
            last_layer = values[char_num - 1]
            last_word = min(
                last_layer, key=lambda k: last_layer[k]["probs"])
            
            res_str = last_word

            for i in range(char_num - 2):
                layer = char_num - 1 - i
                last_word = values[layer][last_word]["parent"]
                res_str = last_word[0] + res_str
            
            result += res_str + "\n"
    

    with open(args.output, "w") as file:
        file.write(result)
    