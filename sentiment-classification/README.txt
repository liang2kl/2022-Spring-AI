环境：Python 3.9.10

安装依赖：pip3 install -r requirements.txt

文件结构：
.
├── README.txt
├── analysis.py
├── dataset
│   ├── test.txt
│   ├── train.txt
│   ├── validation.txt
│   └── wiki_word2vec_50.bin
├── main.py
├── model.py
├── preprocess.py
└── requirements.txt

运行：
1. python3 preprocess.py
2. 根据 python3 main.py --help 运行。例：
    (a) python3 main.py LSTM --dropout=0.4 --layers=2 --bidirectional
    (b) python3 main.py CNN --dropout=0.4 --filter-num=128
