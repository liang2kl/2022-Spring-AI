import os, sys

if __name__ == "__main__":
    if not os.path.isdir("./res"):
        os.mkdir("./res")
    with open("./split.py") as file:
        exec(file.read())
    with open("./stats.py") as file:
        exec(file.read())
    sys.argv = ["main_ternary.py", "./data/input.txt", "--_lambda", "0.9",
        "--alpha", "0.5", "-o", "./res/output.txt"
    ]
    with open(sys.argv[0]) as file:
        exec(file.read())
