# cpp\_etudes

cpp\_etudes is a cpp project integrated with google benchmark, google test for validating cpp features and refining vectorized builtin function of databases. It can be
built by both clang and gcc. Some interesting perl scripts are also included.


## Scripts

### create\_table.pl: create a ascii-border table from CSV format.

1. create a csv file

```shell
cat >repeat.csv  <<'DONE'
t,vectorization,new-vectorization,non-vectorization,new speedup,vectorization speedup
repeat/const_times,3112ms, 9797ms, 13986ms(4X memory),3.14,4.49
repeat/const_s, 8107ms, 30231ms, 39501ms(32X memory), 3.72,4.87
repeat,9141ms, 32182ms,44315ms(32X memory), 3.52,4.84
DONE
```

2. generate a ascii-border table

```
cat repeat.csv |./create_table.pl

+====================+===============+===================+=====================+=============+=======================+
| test               | vectorization | new-vectorization | non-vectorization   | new speedup | vectorization speedup |
+====================+===============+===================+=====================+=============+=======================+
| repeat/const_times | 3112ms        | 9797ms            | 13986ms(4X memory)  | 3.14        | 4.49                  |
| repeat/const_s     | 8107ms        | 30231ms           | 39501ms(32X memory) | 3.72        | 4.87                  |
| repeat             | 9141ms        | 32182ms           | 44315ms(32X memory) | 3.52        | 4.84                  |
+--------------------+---------------+-------------------+---------------------+-------------+-----------------------+
```

### cpp\_class\_hierarchy.pl: show class hierarchy of cpp project in the style of Linux utility tree.

1. usage, try with incubator-doris, other cpp projects are just ok.

```
# format

./cpp_class_hierarchy.pl <keyword|regex> <verbose(0|1)> <depth(num)>
- keyword for exact match, regex for fuzzy match
- verbose=0, no file locations output; otherwise succinctly output.
- depth=num, print max derivation depth.

git clone https://github.com/satanson/incubator-doris.git
cd incubator-doris

# show all classes
./cpp_class_hierarchy.pl '\w+'

# show all classes with file locations.
./cpp_class_hierarchy.pl '\w+' 1

# show all classes exact-match ExecNode if ExecNode class exists
./cpp_class_hierarchy.pl 'ExecNode' 1

# show all classes fuzzy-match regex '.*Node$' if the literal class name not exists.
./cpp_class_hierarchy.pl '.*Node$' 1

# show all classes and depth of derivation relationship is less than 3
./cpp_class_hierarchy.pl '\w+' 1 3

```
2. some outputs

```
cd incubator-doris
./cpp_class_hierarchy.pl 'ExecNode'
```
![image](./images/doris_execnode.png)


```
cd ClickHouse
./cpp_class_hierarchy.pl IProcessor
```
![image](./images/clickhouse_iprocessor.png)


```
cd ClickHouse
./cpp_class_hierarchy.pl IDataType 1
```
![image](./images/clickhouse_idatatype.png)

### color\_palette.pl: show color palette for colorful terminal output.

1. usage
```
./color_palette.pl
```

![image](./images/color_palette.png)


