一般持久化存储的核心是WAL, WAL是通用技术, 在其他存储系统中也普遍存在, 因此，这里的涉及到的知识点具有普适性, 也可以指导其他系统的代码阅读.

WAL主要包括:
- prepare queue: 未提交日志在内存中.
- 日志格式: 持久化设备中的日志作为sequential file, 有特定的内部格式,  总之可以抽象成一个有带有单调递增编号LSN(Log Sequence Number)的record序列.
- In-memory数据结构: 数据在内存中的组织方法.
- On-disk数据格式: 数据在持久化设备上如果组织.

WAL的精髓是4个LSN, 找到了这四个点, 基本上可以掌握一个存储引擎的代码RoadMap, 后续的继续深入研究, 会从全局问题转变成局部问题.
- prepare point(准备点):  prepare queue中的最大LSN.
- commit point(提交点):  这是最关键的LSN, 改点以前的数据为以提交数据, 改点之后的数据为未提交数据. 
- apply point(播放点): 已提交的日志应用于In-memory数据结构的最大LSN.
- checkpoint(检查点): 截止该点的数据已经持久化于设备, 改点之前的日志可以情理掉. recovery from crash需要读到这个点的磁盘镜像, 然后播放日志，一般日志播放的起点只要不晚于该checkpoint即可. 然后, 有的系统的涉及不够严谨, 需要满足日志不重播的约束(比如HDFS namenode的FSEditLog, 当然FSEditLog并不是WAL,  不再赘述).

这个点需要满足约束关系:

checkpoint <= apply point <= commit point <= prepare point

1. 写请求不断到达,  parepare queue入队, prepare point点持续增长.
2. 做顺序的group commit， commit point始终追赶prepare point， prepare queue出队.
3. 将apply point点和commit point之间日志, 应用于内存数据结构, apply point持续追赶commit point,  单机的提交语义的完成，需要apply point追赶上commit point，因为只有这样, 已提交的数据才能对用户可见; 如果是主从复制协议，那么follower replica上apply point显然可以和commit point保持距离.
4. 内存中数据dump或者flush到磁盘上，并且可靠地持久化后，on-disk image所包含的日志, 可以清理掉.

显然找到这4个点, 基本上代码结构就已经很清楚了. 这4个点其实涉及如下几个关键问题:
1. 提交语义是啥, 因为WAL是redo log, 先commit后apply的提交语义已经明确, 这个做法是固定的. 当然undo log， redo/undo log的提交语义会用差别, 不再赘述.
2. flush机制: 数据怎么转存到持久化设备.
3. checkpoint, 日志备份清理和数据恢复.

这些问题属于正确性. 涉及性能， 则还需要终点研究:
1. 写放大
2. 读方法
3. 空间放大

能不能不看官方文档，直接阅读代码呢？ 答案是肯定的. 因为存储引擎的宏观上设计空间其实是可以通过代码阅读快速确定的. 从细粒度看问题，则存储引擎涉及的东西, 比较多. 但是从粗粒度看问题，持久化存储引擎的精髓是fsync. 涉及fsync地方有:
1. 日志切换：比如日志写满一个文件，切下一个文件, 对这个文件所在父目录执行fsync. 举例子，比如etcd.
2. 日志提交：group commit需要调用fsync, 当然fsync延迟较高, 分布式存储系统中，比如hdfs和kafka, 日志这块，也可以采用flush. 多副本比较安全, 机房有备用电池也能够保证数据的掉电后刷盘.
3. 数据checkpoint：截止checkpoint, 数据已经已经可靠地持久化.

所以, 存储引擎的代码，第一步找fsync/fdatasync/sync_file_range