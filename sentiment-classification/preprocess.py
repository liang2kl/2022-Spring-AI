import json

SETS = ["train", "validation", "test"]

if __name__ == "__main__":
    for set in SETS:
        with open(f"./dataset/{set}.txt") as input_file:
            with open(f"./dataset/{set}.json", "w") as output_file:
                for line in input_file:
                    components = line.split()
                    if components:
                        json_line = json.dumps({
                            "sentiment": int(components[0]),
                            "text": components[1:]
                        }, ensure_ascii=False)
                        output_file.write(json_line + "\n")
