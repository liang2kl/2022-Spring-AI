mkdir -p ./res

for alpha in 0 0.1 0.2 0.5 1.0; do
    echo alpha=$alpha
    output_file=./res/output-$alpha.txt
    python3 main_ternary.py ./data/input.txt --_lambda 0.9 --alpha $alpha -o $output_file

    if [[ "$?" -ne "0" ]]; then
        exit $?
    fi

    ana_output=./res/analyse-$alpha.txt
    touch $ana_output
    echo $alpha > $ana_output
    python3 analysis.py $output_file ./res/std_output.txt >> $ana_output

    if [[ "$?" -ne "0" ]]; then
        exit $?
    fi
done
