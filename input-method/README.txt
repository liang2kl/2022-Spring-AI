环境：python 3.9

文件结构：
.
├── README.txt
├── data
│   ├── dataset（训练语料）
│   │   ├── 2016-02.txt
│   │   ├── 2016-04.txt
│   │   ├── 2016-05.txt
│   │   ├── 2016-06.txt
│   │   ├── 2016-07.txt
│   │   ├── 2016-08.txt
│   │   ├── 2016-09.txt
│   │   ├── 2016-10.txt
│   │   └── 2016-11.txt
│   ├── input.txt（测试文件）
│   └── pinyin.txt（拼音到汉字的映射文件）
├── res（中间结果和最终结果）
│   └── ...
├── utils.py（一些工具函数）
├── pinyin.py（获取拼音到汉字的映射）
├── split.py（将语料分割为不含汉字外字符的句子）
├── stats.py（统计分割后语料的数据）
├── main.py（进行拼音到汉字的转化，二元模型）
├── main_ternary.py（进行拼音到汉字的转化，三元模型）
├── analysis.py（正确率计算）
└── test.sh（对不同的参数进行测试）

自动运行（使用预设参数）：
1. 将语料和数据放入 data 文件夹中
2. python auto_run.py

手动运行：
1. 将语料和数据放入 data 文件夹中
2. 新建 res 文件夹
3. 依次运行 split.py stats.py 得到统计数据
4. 根据 --help 运行二元模型（main.py）或三元模型（main_ternary.py）
