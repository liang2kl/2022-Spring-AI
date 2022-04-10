from pathlib import Path
import json
import re

SRC_DIR = Path("./data/dataset")
OUTPUT_FILE = Path("./res/segments.txt")

if __name__ == "__main__":
    file_paths = SRC_DIR.glob("*.txt")

    all_content = ""

    for path in file_paths:
        print("processing", path)
        with open(path, "r") as file:
            for line in file:
                article = json.loads(line)
                content = article["html"]
                for c in content:
                    if c < u'\u4E00' or c > u'\u9FA5':
                        content = content.replace(c, " ")
                all_content += "".join(
                    [s + "\n" for s in content.split() if len(s) >= 2]
                )
    
    with open(OUTPUT_FILE, "w") as file:
        file.write(all_content)
