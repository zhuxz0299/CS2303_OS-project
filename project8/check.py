# -*- coding: utf-8 -*-

def compare_files(file1, file2):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        lines1 = f1.readlines()
        lines2 = f2.readlines()

    if len(lines1) != len(lines2):
        print("文件内容不同：行数不一致")
        return

    for i, (line1, line2) in enumerate(zip(lines1, lines2)):
        if line1 != line2:
            print("文件内容不同：第{}行".format(i+1))
            print("文件1内容：{}".format(line1))
            print("文件2内容：{}".format(line2))
            return

    print("文件内容完全相同")

# 示例用法
file1 = './output.txt'
file2 = './correct.txt'
compare_files(file1, file2)
