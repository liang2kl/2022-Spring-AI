import sys

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Invalid arguments\n\tUsage: <program> <output_file> <std_file>")
        exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]

    with open(input_file) as file:
        input_data = file.readlines()
    with open(output_file) as file:
        output_data = file.readlines()
    
    accurate_sentences = 0
    accurate_chars = 0
    total_chars = 0
    
    for i in range(len(input_data)):
        if input_data[i] == output_data[i]:
            accurate_sentences += 1
        for j in range(len(input_data[i])):
            if input_data[i][j] == output_data[i][j]:
                accurate_chars += 1
        total_chars += len(input_data[i])
    print("sentence:", accurate_sentences / len(input_data) * 100)
    print("character:", accurate_chars / total_chars * 100)