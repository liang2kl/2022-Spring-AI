import json
from pathlib import Path
from split import OUTPUT_FILE as SRC_FILE

OUTPUT_FILE = Path("./res/stats.json")

def get_stats_map():
    with open(OUTPUT_FILE) as file:
        data = json.loads(file.read())
        return data["binary"], data["ternary"]

if __name__ == "__main__":
    res = {}
    res_ternary = {}

    with open(SRC_FILE, "r") as file:
        for line in file:
            line = line[:-1] # get rid of \n
            for i, c in enumerate(line):
                # binary
                res.setdefault(c, {"start": 0, "total": 0})

                res[c]["total"] += 1

                if i == len(line) - 1: # last char
                    continue
                else:
                    next = line[i + 1]
                    res[c].setdefault(next, 0)
                    res[c][next] += 1

                    if i == 0:
                        res[c]["start"] += 1
                        
                # ternary
                if i == 0:
                    continue
                
                chars = line[i - 1] + c
                next = line[i + 1]
                res_ternary.setdefault(chars, {"start": 0, "total": 0})
                res_ternary[chars].setdefault(next, 0)

                res_ternary[chars]["total"] += 1
                res_ternary[chars][next] += 1

                if i == 1:
                    res_ternary[chars]["start"] += 1

    
    with open(OUTPUT_FILE, "w") as file:
        file.write(json.dumps(
            {"binary": res, "ternary": res_ternary}, ensure_ascii=False, indent=4))
    

