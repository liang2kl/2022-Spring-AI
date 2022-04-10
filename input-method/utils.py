import math, argparse

def _get_prob_raw(map, prev, curr, total_curr, lamb):
    count_1 = map[prev]["total"]
    count_2 = map[prev].get(curr, 0)
    p_single = map[curr]["total"] / total_curr
    p = (lamb * count_2 / count_1 if count_1 > 0 else 0) + (1 - lamb) * p_single
    return p

def get_prob(map, prev, curr, total_curr, lamb):
    p = _get_prob_raw(map, prev, curr, total_curr, lamb)
    return -math.log(p) if p > 0 else 10000

def get_prob_ter(bin_map, ter_map, prev, curr, total_curr, lamb, alpha):
    count_1 = ter_map[prev]["total"]
    count_2 = ter_map[prev].get(curr, 0)
    p_single = bin_map[curr]["total"] / total_curr
    p_ternary = (lamb * count_2 / count_1 if count_1 > 0 else 0) + (1 - lamb) * p_single
    p_binary = _get_prob_raw(bin_map, prev[1], curr, total_curr, lamb)
    p = p_ternary * (1 - alpha) + p_binary * alpha
    return -math.log(p) if p > 0 else 10000

def parse_args(ternary: bool):
    parser = argparse.ArgumentParser("Translate sentences from Hanyu Pinyin")
    parser.add_argument("input_file", metavar="INPUT", type=str)
    parser.add_argument("--output", "-o", type=str)
    parser.add_argument("--_lambda", action="store", type=float)
    if ternary:
        parser.add_argument("--alpha", action="store", type=float)
    return parser.parse_args()
