__author__ = 'lee'
# -*- coding:UTF-8 -*-
# linux mint 
# cannot run the program on windows

import time
import sqlite3
from collections import Counter

def create_db(dbname):
    conn = sqlite3.connect(dbname)
    cursor = conn.cursor()
    cursor.execute("create table table1("
                   "id text, "
                   "title text, "
                   "authors text, "
                   "year text, "
                   "conf text, "
                   "citation text, "
                   "index_id unsigned big int primary key, "
                   "pid text, "
                   "ref text, "
                   "ref_count text, "
                   "refed_count text,"
                   "abstract text"
                   ")")
    conn.commit()
    cursor.close()
    conn.close()
    print("datebase created success!")

def insert_db(dbname):
    conn = sqlite3.connect(dbname)
    cursor = conn.cursor()
    f = open(r"/home/lee/lab/python/readdata/acm_output.txt", "r")
    ftxt = open("refedcount.txt","a") # 用于存储引用和被引用的记录
    fref = open("refset.txt","a") # 用于存储被引用文章的id的集合
    fnet1 = open("directed_net_index.txt","a") # 含有索引的有向网络
    fnet2 = open("undirected_net_index.txt", "a") # 含有索引的无向网络
    line = f.readline() # 这一行是读取总共有多少条数据，并不需要

    # 初始化
    net_id = 0 # 有向网络的索引id
    unnet_id = 0 # 无向网络的索引id
    id = 0 # 自定义的id
    title = ''
    author = ''
    year = ''
    conf = ''
    citation = ''
    index_id = ''
    pid = ''
    ref = ''
    ref_count = 0 # 该文章引用其他文章的数目
    refed_count = 0 # 该文章被引用的次数
    abstract = ''


    while line:
        line = f.readline()
        if line[0:2] == "#*":
            title = line[2:].strip("\n")
        elif line[0:2] == "#@":
            author = line[2:].strip("\n")
        elif line[0:5] == "#year":
            year = line[5:].strip("\n")
        elif line[0:5] == "#conf":
            conf = line[5:].strip("\n")
        elif line[0:9] == "#citation":
            citation = line[9:].strip("\n")
        elif line[0:6] == "#index":
            index_id = int(line[6:].strip("\n"))
        elif line[0:8] == "#arnetid":
            pid = line[8:].strip("\n")
        elif line[0:2] == "#%":
            if ref == "":
                ref = line[2:].strip("\n")
                ref_count = 1 # 第一次读取到有引用文章，表示该文章引用其他文章的数目为1

                # 向文本写入引用的条目和被引用的条目
                ftxt.write(str(index_id))  # 该文章的id
                ftxt.write(" ")
                ftxt.write(str(line[2:].strip("\n")))
                ftxt.write("\n")

                # 向文本中写入有向网络
                net_id = int(net_id) + 1
                fnet1.write(str(net_id))
                fnet1.write(",")
                fnet1.write(str(index_id))
                fnet1.write(",")
                fnet1.write(str(line[2:].strip("\n")))
                fnet1.write("\n")
                
                # 向文本中写入被引用id的集合
                fref.write(str(line[2:].strip("\n")))
                fref.write(",")
            else:
                ref = ref + "," # id所引用的条目
                ref = ref + line[2:].strip("\n")
                ref_count = ref_count + 1 # 文章引用其他文章的数目+1

                # 向文本中写入引用和被引用
                ftxt.write(str(index_id))
                ftxt.write(" ")
                ftxt.write(str(line[2:].strip("\n")))
                ftxt.write("\n")
                
                # 向文本中写入有向网络
                net_id = int(net_id) + 1
                fnet1.write(str(net_id))
                fnet1.write(",")
                fnet1.write(str(index_id))
                fnet1.write(",")
                fnet1.write(str(line[2:].strip("\n")))
                fnet1.write("\n")
                
                # 向文本中写入被引用id的集合
                fref.write(str(line[2:].strip("\n")))
                fref.write(",")

        elif line[0:2] == "#!":
            abstract = line[2:].strip("\n")
        elif line == "\n":
            id = int(id) + 1
            cursor.execute(\
            "insert into table1(\
            id, title, authors, year, conf, citation, index_id, pid, ref, ref_count, refed_count, abstract) \
            values(?,?,?,?,?,?,?,?,?,?,?,?)",\
            (id, title, author, year, conf, citation, index_id, pid, ref, ref_count, refed_count, abstract))
            title = ''
            author = ''
            year = ''
            conf = ''
            citation = ''
            index_id = ''
            pid = ''
            ref = ''
            ref_count = 0
            abstract = ''
    conn.commit()
    cursor.close()
    conn.close()
    ftxt.close()
    fref.close()
    fnet1.close()
    
    print("database inserted success!")

# 将refset(其中放置的是被引用的条目)中的被引用的条目的次数更新在数据库中，表明index_id被引用的次数
def update_db(dbname):
    f = open("refset.txt","r")
    line = f.readline()
    line = line.split(",")
    elementCounter = Counter(line)
    dictCounter = dict(elementCounter)
    dictNum = len(dictCounter)

    conn = sqlite3.connect(dbname)
    cursor = conn.cursor()

    for i in range(0, dictNum):
        cursor.execute("update table1 set refed_count = ? where index_id = ?", (elementCounter[line[i]], int(line[i])))
    

    conn.commit()
    cursor.close()
    conn.close()

    print("database updated success!")

# 获取引用的次数之间的关系，详情见邮件
def collected_refed_count(dbname):
    fr = open("refset.txt", "r")
    fw_count_set = open("refed_count_set.txt", "w") # 被引用的次数的集合
    fr_count_set = open("refed_count_set.txt", "r")
    fw_count_count = open("refed_count_count.txt", "w") # 被引用的次数 与 被引用次数的次数

    line = fr.readline()
    line = line.split(",")
    elementCounter = Counter(line)
    dictCounter = dict(elementCounter)
    dictNum = len(dictCounter)

    # 从引用集合更改为字典，只获取字典中被引用次数，得到被引用次数的id
    for i in range(0, dictNum):
        fw_count_set.write(str(elementCounter[line[i]]))
        fw_count_set.write(",")
    
    line2 = fr_count_set.readline()
    line2 = line2.split(",")
    elementCounter2 = Counter(line2)
    dictCounter2 = dict(elementCounter2)
    dictNum2 = len(dictCounter2)
    
    # 从被引用次数集合获取到被引用次数的集合的集合
    for i in range(0, dictNum2):
        # elementCounter2[line2[i]]中的值表示被引用的次数的次数
        # elementCounter2[line2[i]]=line[0]表示x轴
        # line2[i]表示被引用的次数=line[1]表示y轴
        fw_count_count.write(str(elementCounter2[line2[i]]))
        fw_count_count.write(",")
        fw_count_count.write(str(line2[i]))
        fw_count_count.write("\n")

    fw_count_set.close()
    fw_count_count.close()

if __name__ == '__main__':
    start = time.clock()
    dbname = "addrefcount.db"
    create_db(dbname)
    insert_db(dbname)
    update_db(dbname)
    #collected_refed_count(dbname)
    end = time.clock()
    
    print("the program took time: ")
    print(end-start)
