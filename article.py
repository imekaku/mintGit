__author__ = "lee"

# -*- coding:UTF-8 -*-
# 处理超大文本的文章数据
# 2016年03月13日15:45:13
# 需要在linux环境下运行，windows会出现编码错误

import time
import sqlite3
import os
import shutil
from collections import Counter
import matplotlib.pyplot as plt

# 完成从原始文本中读取到数据到数据库
class Getdatabase:
    dbname = "allmessage.db"
    
    # 创建数据库，该数据库是从文本中读取
    def create_db(self, dbname):
        if(os.path.exists("./allmessage.db")):
            print("The database is exists, and the programming is dropping it!")
            os.remove("./allmessage.db")
        conn = sqlite3.connect(dbname)
        cursor = conn.cursor()
        cursor.execute("create table table1("
                       "id int, "
                       "title text, "
                       "authors text, "
                       "year text, "
                       "conf text, "
                       "citation text, "
                       "index_id int primary key, "
                       "pid text, "
                       "ref text, "
                       "ref_count int, "
                       "refed_count int,"
                       "abstract text"
                       ")")
        conn.commit()
        cursor.close()
        conn.close()
        print("datebase created success!")

    # 从文本中读取数据到数据库
    def insert_db(self, dbname):
        if(os.path.exists("./ref")):
            print("The folder ref exists, and the programming is dropping it!")
            shutil.rmtree("./ref")
            os.mkdir("./ref")
            ftxt = open("./ref/refedcount.txt","a") # 用于存储引用和被引用的记录
            fref = open("./ref/refset.txt","a") # 用于存储被引用文章的id的集合
        if(os.path.exists("./net")):
            print("The folder net exists, and the programming is dropping it!")
            shutil.rmtree("./net")
            os.mkdir("./net")
            fnet1 = open("./net/directed_net_index.txt","a") # 含有索引的有向网络
            fnet2 = open("./net/undirected_net_no_index.txt", "a") # 不含索引的无向网络

        f = open("acm_output.txt", "r")

        conn = sqlite3.connect(dbname)
        cursor = conn.cursor()
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
                    
                    # 向文本中写入不带序号的无向网络
                    fnet2.write(str(index_id))
                    fnet2.write(",")
                    fnet2.write(str(line[2:].strip("\n")))
                    fnet2.write("\n")

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
                    
                    # 向文本中写入不带序号的无向网络
                    fnet2.write(str(index_id))
                    fnet2.write(",")
                    fnet2.write(str(line[2:].strip("\n")))
                    fnet2.write("\n")

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
        fnet2.close()
        
        print("database inserted success!")

    # 更新数据库的refed_count字段
    def update_db(self, dbname):
        if(not os.path.exists("./ref/refset.txt")):
            print("There is no folder named ref or no file named refset.txt!")
            return -1
        f = open("./ref/refset.txt", "r")
        line = f.readline().strip("\n").split(",")
        line = line[:-1]
        myset = set(line)
        elementCounter = Counter(line)

        conn = sqlite3.connect(dbname)
        cursor = conn.cursor()

        for item in myset:
            cursor.execute("update table1 set refed_count = ? where index_id = ?", (int(elementCounter[item]), int(item)))
        conn.commit()
        cursor.close()
        conn.close()
        f.close()

        print("database update success!")
    def work(self):
        self.create_db(self.dbname)
        self.insert_db(self.dbname)
        self.update_db(self.dbname)
        print("----------------------The class Getdatabase finished!----------------------")

# 获取绘画
class Paint:
    def collected_refed_count(self):
        if(not os.path.exists("./ref/refset.txt")):
            print("There is no folder named ref or no file named refset.txt!")
            return -1
        fr = open("./ref/refset.txt", "r")
        fw_count_set = open("./ref/refed_count_set.txt", "w") # 被引用的次数的集合
        fw_count_count = open("./ref/refed_count_count.txt", "w") # 被引用的次数 与 被引用次数的次数

        line = fr.readline().strip("\n").split(",")
        line = line[:-1]
        myset1 = set(line)
        elementCounter = Counter(line)

        # 从引用集合更改为字典，只获取字典中被引用次数，得到被引用次数的id
        for item in myset1:
            fw_count_set.write(str(elementCounter[item]))
            fw_count_set.write(",")
        
        fw_count_set.close()
        fr_count_set = open("./ref/refed_count_set.txt", "r")
        line2 = fr_count_set.readline().strip("\n").split(",")
        line2 = line2[:-1]
        myset2 = set(line2)
        elementCounter2 = Counter(line2)
        
        # 从被引用次数集合获取到被引用次数的集合的集合
        for item in myset2:
            # elementCounter2[line2[i]]中的值表示被引用的次数的次数
            # elementCounter2[line2[i]]=line[0]表示x轴
            # line2[i]表示被引用的次数=line[1]表示y轴
            fw_count_count.write(str(elementCounter2[item]))
            fw_count_count.write(",")
            fw_count_count.write(str(item))
            fw_count_count.write("\n")
        fr_count_set.close()
        fw_count_count.close()
        os.remove("./ref/refed_count_set.txt")
        print("refed_count_count txt getted success!")

    def paint_count_count_by_log(self):
        if(not os.path.exists("./ref/refed_count_count.txt")):
            print("There is no folder named ref or no file named refset_count_count.txt!")
            return -1
        f = open("./ref/refed_count_count.txt", "r")
        line = f.readline()
        fig = plt.figure()
        ax = fig.add_subplot(111)

        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.set_xlim(1e-1, 1e6) # <-- check this as pointed out by @tillsten
        ax.set_ylim(1e-1, 1e6) # <--
        ax.set_aspect(1)
        ax.set_title("The ref's count of ref's count by log")

        while line:
            line = line.split(",")
            y = line[0]
            x = line[1]
            ax.plot(x, y, "r.")
            line = f.readline()

        plt.xlabel("refed_count")
        plt.ylabel("refed_count_count")
        plt.savefig("count_count_by_log.png", format="png")
        print("Get the image success!")

    def work(self):
        self.collected_refed_count()
        self.paint_count_count_by_log()
        print("--------------------------The class Paint finished!--------------------------------")

class Net:
    dbname = "allmessage.db"

    def get_directed_net_id_out(self, dbname):
        if(not os.path.exists("./net")):
            print("There is no folder named net!")
            return -1
        f = open("./net/directed_net_in_out.txt", "a")
        conn = sqlite3.connect(dbname)
        cursor = conn.cursor()
        cursor.execute("select index_id, ref_count, refed_count from table1 where ref_count > 0 or refed_count > 0")
        rows = cursor.fetchall()
        for row in rows:
            row = list(row)
            f.write(str(row[0]))
            f.write(",")
            f.write(str(row[1]))
            f.write(",")
            f.write(str(row[2]))
            f.write("\n")
        f.close()
        cursor.close()
        conn.close()
    def get_undirected_net_index(self):
        if(not os.path.exists("./net/undirected_net_no_index.txt")):
            print("There is no file named undirected_net_no_index.txt!")
            return -1
        f = open("./net/undirected_net_no_index.txt","r") # 这个文本存放的是引用id和被引用id
        fsort = open("./net/undirected_net_index_sort.txt","a") # 这个文本存放的是，整理之后文章id, 和他引用的id
        allref = []
        line = f.readline()
        while line:
            line = line.strip("\n")
            line = line.split(",")
            if(int(line[0]) < int(line[1])):
                a = int(line[0])
                b = int(line[1])
            else:
                a = int(line[1])
                b = int(line[0])
            tupleref = (a, b)
            allref.append(tupleref)
            line = f.readline()

        allref = sorted(allref, key=lambda allref_tuple:allref_tuple[0])
        index = 1
        for l in allref:
            fsort.write(str(index))
            fsort.write(",")
            fsort.write(str(l[0]))
            fsort.write(",")
            fsort.write(str(l[1]))
            fsort.write("\n")
            index = index + 1

        fsort.close()

    def drop_undirected_net_index_something_same(self):
        if(not os.path.exists("./net/undirected_net_index_sort.txt")):
            print("There is no file named undirected_net_index_sort.txt!")
            return -1
        f = open("./net/undirected_net_index_sort.txt", "r") # 整理过后，但是存在重复的文件
        fdelete = open("./net/undirected_net_index_sort_final.txt", "a") # 整理过后，并且不重复的文件
        line = f.readline()
        first_id = 0
        first_ref = 0
        index = 0
        while line:
            index = index + 1
            line = line.strip("\n").split(",")
            last_id = int(line[1])
            last_ref = int(line[2])
            if(first_ref == last_ref):
                if(first_id == last_id):
                    index = index - 1
                else:
                    fdelete.write(str(index))
                    fdelete.write(",")
                    fdelete.write(str(last_id))
                    fdelete.write(",")
                    fdelete.write(str(last_ref))
                    fdelete.write("\n")
            else:
                fdelete.write(str(index))
                fdelete.write(",")
                fdelete.write(str(last_id))
                fdelete.write(",")
                fdelete.write(str(last_ref))
                fdelete.write("\n")
            line = f.readline()
            first_id = last_id
            first_ref = last_ref
        f.close()
        fdelete.close()

    def get_undirected_samenode(self):
        if(not os.path.exists("./net/undirected_net_index_sort.txt")):
            print("There is no file named undirected_net_index_sort.txt!")
            return -1
        f = open("./net/undirected_net_index_sort.txt","r")
        fsame = open("./net/undirected_net_samenode.txt", "a")
        line = f.readline()

        first_id = 0
        first_ref = 0

        while line:
            line = line.strip("\n").split(",")
            last_id = int(line[1])
            last_ref = int(line[2])
            if(first_ref == last_ref):
                if(first_id == last_id):
                    fsame.write(str(line[1]))
                    fsame.write(",")
                    fsame.write(str(line[2]))
                    line = f.readline()
                    while line:
                        line = line.strip("\n").split(",")
                        last_id = int(line[1])
                        last_ref = int(line[2])
                        if(first_ref == last_ref):
                            if(first_id == last_id):
                                fsame.write(",")
                                fsame.write(str(line[1]))
                                fsame.write(",")
                                fsame.write(str(line[2]))
                        line = f.readline()
                        first_id = last_id
                        first_ref = last_ref
                    break
            line = f.readline()
            first_id = last_id
            first_ref = last_ref
        f.close()
        fsame.close()
    def get_directed_net_add_of_in_out(self):
        if(not os.path.exists("./net/directed_net_in_out.txt")):
            print("There is no file named directed_net_in_out.txt!")
            return -1
        f = open("./net/directed_net_in_out.txt", "r") # 有向图的网络
        # 存放"节点 入度 出度"
        faddinout = open("./net/directed_net_add_of_in_out.txt", "a") # 有向网络的入度加出度 格式是：id du
        # 存放"节点 度数（入度+出度）"
        line = f.readline()
        while line:
            line = line.strip("\n")
            line = line.split(",")
            indu = int(line[1])
            outdu = int(line[2])
            du = indu + outdu
            faddinout.write(line[0])
            faddinout.write(",")
            faddinout.write(str(du))
            faddinout.write("\n")
            line = f.readline()
        f.close()
        faddinout.close()

    def create_database(self, dbname):
        conn = sqlite3.connect(dbname)
        cursor = conn.cursor()
        cursor.execute("create table table2(id int, du int)")
        conn.commit()
        cursor.close()
        conn.close()
        print("database table2 created success")

    def insert_database(self, dbname):
        if(not os.path.exists("./net/directed_net_add_of_in_out.txt")):
            print("There is no file named directed_net_add_of_in_out.txt!")
            return -1
        conn = sqlite3.connect(dbname)
        cursor = conn.cursor()
        f = open("./net/directed_net_add_of_in_out.txt", "r")
        line = f.readline()

        while line:
            line = line.strip("\n").split(",")
            cursor.execute("insert into table2(id, du) values(?, ?)", (int(line[0]), int(line[1])))
            line = f.readline()

        conn.commit()
        cursor.close()
        conn.close()
        f.close()
        print("insert database table2 success! put the text into database!")

    def drop_same(self, dbname):
        # filename2 是彼此互相引用的文章id的集合
        if(not os.path.exists("./net/undirected_net_samenode.txt")):
            print("There is no file named undirected_net_samenode.txt!")
            return -1
        conn = sqlite3.connect(dbname)
        cursor = conn.cursor()
        f = open("./net/undirected_net_samenode.txt", "r")
        lines = f.readline().strip("\n").split(",")
        lines = lines[:-1]
        for line in lines:
            line = int(line)
            cursor.execute("select du from table2 where id = ?", (line,))
            row = cursor.fetchone()
            row = list(row)
            newdu = int(row[0])-1
            cursor.execute("update table2 set du = ? where id = ?", (newdu, line))
        conn.commit()
        cursor.close()
        conn.close()
        f.close()
        print("drop same success! pull out the same du from database!")

    def download_database(self, dbname):
        conn = sqlite3.connect(dbname)
        cursor = conn.cursor()
        f = open("./net/undirected_net_no_same_du.txt", "a")
        cursor.execute("select * from table2")
        rows = cursor.fetchall()
        for row in rows:
            row = list(row)
            index_id = str(row[0])
            du = str(row[1])
            f.write(index_id)
            f.write(",")
            f.write(du)
            f.write("\n")
        conn.commit()
        cursor.close()
        conn.close()
        f.close()
        print("download database success! get the new text where doesn't exist the same du!'")
    def work(self):
        self.get_directed_net_id_out(self.dbname)
        self.get_undirected_net_index()
        self.drop_undirected_net_index_something_same()
        self.get_undirected_samenode()
        self.get_directed_net_add_of_in_out()
        self.create_database(self.dbname)
        self.insert_database(self.dbname)
        self.drop_same(self.dbname)
        self.download_database(self.dbname)
        print("----------------The class Net finished!--------------------------")

if __name__ == "__main__":
    start = time.clock()
    getdatabase = Getdatabase()
    getdatabase.work()
    paint = Paint()
    paint.work()
    net = Net()
    net.work()
    end = time.clock()
    print("-----------------------finished------------------")
    print("The programming took time: "+str(end-start))
