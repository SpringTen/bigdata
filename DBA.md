# Locks
- ```sql
  select relation::regclass, * from pg_locks where not granted;
  
  SELECT blocked_locks.pid     AS blocked_pid,
         blocked_activity.usename  AS blocked_user,
         blocking_locks.pid     AS blocking_pid,
         blocking_activity.usename AS blocking_user,
         blocked_activity.query    AS blocked_statement,
         blocking_activity.query   AS current_statement_in_blocking_process
   FROM  pg_catalog.pg_locks         blocked_locks
    JOIN pg_catalog.pg_stat_activity blocked_activity  ON blocked_activity.pid = blocked_locks.pid
    JOIN pg_catalog.pg_locks         blocking_locks 
        ON blocking_locks.locktype = blocked_locks.locktype
        AND blocking_locks.database IS NOT DISTINCT FROM blocked_locks.database
        AND blocking_locks.relation IS NOT DISTINCT FROM blocked_locks.relation
        AND blocking_locks.page IS NOT DISTINCT FROM blocked_locks.page
        AND blocking_locks.tuple IS NOT DISTINCT FROM blocked_locks.tuple
        AND blocking_locks.virtualxid IS NOT DISTINCT FROM blocked_locks.virtualxid
        AND blocking_locks.transactionid IS NOT DISTINCT FROM blocked_locks.transactionid
        AND blocking_locks.classid IS NOT DISTINCT FROM blocked_locks.classid
        AND blocking_locks.objid IS NOT DISTINCT FROM blocked_locks.objid
        AND blocking_locks.objsubid IS NOT DISTINCT FROM blocked_locks.objsubid
        AND blocking_locks.pid != blocked_locks.pid
  
    JOIN pg_catalog.pg_stat_activity blocking_activity ON blocking_activity.pid = blocking_locks.pid
   WHERE NOT blocked_locks.granted;
  
  SELECT blocked_locks.pid     AS blocked_pid,
           blocked_activity.usename  AS blocked_user,
           blocking_locks.pid     AS blocking_pid,
           blocking_activity.usename AS blocking_user,
           blocked_activity.query    AS blocked_statement,
           blocking_activity.query   AS current_statement_in_blocking_process,
           blocked_activity.application_name AS blocked_application,
           blocking_activity.application_name AS blocking_application
     FROM  pg_catalog.pg_locks         blocked_locks
      JOIN pg_catalog.pg_stat_activity blocked_activity  ON blocked_activity.pid = blocked_locks.pid
      JOIN pg_catalog.pg_locks         blocking_locks 
          ON blocking_locks.locktype = blocked_locks.locktype
          AND blocking_locks.DATABASE IS NOT DISTINCT FROM blocked_locks.DATABASE
          AND blocking_locks.relation IS NOT DISTINCT FROM blocked_locks.relation
          AND blocking_locks.page IS NOT DISTINCT FROM blocked_locks.page
          AND blocking_locks.tuple IS NOT DISTINCT FROM blocked_locks.tuple
          AND blocking_locks.virtualxid IS NOT DISTINCT FROM blocked_locks.virtualxid
          AND blocking_locks.transactionid IS NOT DISTINCT FROM blocked_locks.transactionid
          AND blocking_locks.classid IS NOT DISTINCT FROM blocked_locks.classid
          AND blocking_locks.objid IS NOT DISTINCT FROM blocked_locks.objid
          AND blocking_locks.objsubid IS NOT DISTINCT FROM blocked_locks.objsubid
          AND blocking_locks.pid != blocked_locks.pid
   
      JOIN pg_catalog.pg_stat_activity blocking_activity ON blocking_activity.pid = blocking_locks.pid
     WHERE NOT blocked_locks.GRANTED;
   
  SELECT a.datname,
           l.relation::regclass,
           l.transactionid,
           l.mode,
           l.GRANTED,
           a.usename,
           a.query,
           a.query_start,
           age(now(), a.query_start) AS "age",
           a.pid
  FROM pg_stat_activity a
  JOIN pg_locks l ON l.pid = a.pid
  ORDER BY a.query_start;
   
  ```
-
- # Disk
- ```sql
  -- List tables by total disk usage (including indexes).
  
  CREATE OR REPLACE VIEW disk_usage AS
   SELECT pg_tablespace.spcname AS tablespace, pg_namespace.nspname AS schema, pg_class.relname AS relation,
      pg_size_pretty(pg_table_size(pg_class.oid::regclass)) AS table_size,
      pg_size_pretty(pg_indexes_size(pg_class.oid::regclass)) AS index_size,
      pg_size_pretty(pg_total_relation_size(pg_class.oid::regclass)) AS total_size,
      COALESCE(pg_stat_user_tables.seq_scan + pg_stat_user_tables.idx_scan, 0) AS scans
     FROM pg_class
     LEFT JOIN pg_stat_user_tables ON pg_stat_user_tables.relid = pg_class.oid
     LEFT JOIN pg_namespace ON pg_namespace.oid = pg_class.relnamespace
     LEFT JOIN pg_tablespace ON pg_tablespace.oid = pg_class.reltablespace
    WHERE pg_class.relkind = 'r'::"char"
    AND pg_namespace.nspname NOT IN ('pg_catalog', 'information_schema')
    ORDER BY pg_total_relation_size(pg_class.oid::regclass) DESC;
  ```
-
- # PS
- ```sql
  -- A simple view to show active and waiting queries
  
  CREATE OR REPLACE VIEW ps AS
   SELECT pg_stat_activity.pid,
      (host(pg_stat_activity.client_addr) || ':' || pg_stat_activity.client_port) AS client,
      pg_stat_activity.usename AS "user",
      now() - pg_stat_activity.xact_start AS age,
      pg_stat_activity.wait_event_type,
     (SELECT string_agg(relname, ', ') FROM pg_locks, pg_class
  	WHERE pg_locks.pid = pid
  	AND pg_locks.relation = pg_class.oid
  	AND pg_locks.granted = false
     ) AS waiting_for_tables,
      CASE WHEN pg_stat_activity.state = 'active'::text THEN
        pg_stat_activity.query
      ELSE
        pg_stat_activity.state
     END
     FROM pg_stat_activity
    WHERE pg_stat_activity.state <> 'idle'::text
    ORDER BY now() - pg_stat_activity.xact_start DESC;
  ```
-
- # Arrary
- ```sql
  SELECT ARRAY[[1, 2], [3, 4]];
  SELECT ARRAY[1,2,3];
  ```
-
- ### 🧮 PostgreSQL 数组函数一览表
  
  | 函数名             | 参数说明                                               | 返回类型       | 示例语句                                                                 |
  |--------------------|--------------------------------------------------------|----------------|--------------------------------------------------------------------------|
  | `array_append`     | `(array, element)`：向数组尾部添加元素                 | `array`        | `SELECT array_append(ARRAY[1,2], 3);` → `{1,2,3}`                        |
  | `array_prepend`    | `(element, array)`：向数组头部添加元素                 | `array`        | `SELECT array_prepend(0, ARRAY[1,2]);` → `{0,1,2}`                       |
  | `array_cat`        | `(array1, array2)`：连接两个数组                       | `array`        | `SELECT array_cat(ARRAY[1,2], ARRAY[3,4]);` → `{1,2,3,4}`                |
  | `array_remove`     | `(array, value)`：移除数组中所有指定值                 | `array`        | `SELECT array_remove(ARRAY[1,2,2,3], 2);` → `{1,3}`                      |
  | `array_replace`    | `(array, old, new)`：替换数组中指定值为新值            | `array`        | `SELECT array_replace(ARRAY[1,2,5], 5, 9);` → `{1,2,9}`                  |
  | `array_length`     | `(array, dim)`：返回指定维度的长度                     | `integer`      | `SELECT array_length(ARRAY[1,2,3], 1);` → `3`                            |
  | `array_dims`       | `(array)`：返回数组维度的文本表示                     | `text`         | `SELECT array_dims(ARRAY[[1,2],[3,4]]);` → `[1:2][1:2]`                  |
  | `array_ndims`      | `(array)`：返回数组维度数量                           | `integer`      | `SELECT array_ndims(ARRAY[[1,2],[3,4]]);` → `2`                          |
  | `array_upper`      | `(array, dim)`：返回指定维度的上界                     | `integer`      | `SELECT array_upper(ARRAY[10,20,30], 1);` → `3`                          |
  | `array_lower`      | `(array, dim)`：返回指定维度的下界                     | `integer`      | `SELECT array_lower('[0:2]={1,2,3}'::int[], 1);` → `0`                   |
  | `array_position`   | `(array, value [, start])`：返回首次出现位置           | `integer`      | `SELECT array_position(ARRAY['a','b','c'], 'b');` → `2`                 |
  | `array_positions`  | `(array, value)`：返回所有出现位置组成的数组           | `integer[]`    | `SELECT array_positions(ARRAY['x','y','x'], 'x');` → `{1,3}`            |
  | `array_fill`       | `(value, dims [, lower_bounds])`：生成填充数组         | `array`        | `SELECT array_fill(7, ARRAY[2,3]);` → `{{7,7,7},{7,7,7}}`               |
  | `array_to_string`  | `(array, delimiter [, null_str])`：数组转字符串        | `text`         | `SELECT array_to_string(ARRAY[1,NULL,3], ',', '*');` → `1,*,3`          |
  | `string_to_array`  | `(text, delimiter [, null_str])`：字符串转数组         | `text[]`       | `SELECT string_to_array('a~b~c', '~');` → `{a,b,c}`                      |
  | `unnest`           | `(array)`：展开数组为多行                             | `setof element`| `SELECT unnest(ARRAY[1,2,3]);` → `1` `2` `3`（每行一个元素）            |
  | `cardinality`      | `(array)`：返回数组总元素数                           | `integer`      | `SELECT cardinality(ARRAY[[1,2],[3,4]]);` → `4`                          |
  | `trim_array`       | `(array, n)`：移除数组尾部 n 个元素                   | `array`        | `SELECT trim_array(ARRAY[1,2,3,4], 2);` → `{1,2}`                        |
  | `array_sample`     | `(array, n)`：随机抽取 n 个元素组成新数组             | `array`        | `SELECT array_sample(ARRAY[1,2,3,4,5], 3);` → `{2,5,1}`（结果随机）     |
  | `array_shuffle`    | `(array)`：打乱数组顺序                               | `array`        | `SELECT array_shuffle(ARRAY[1,2,3]);` → `{3,1,2}`（结果随机）           |
  
  ---
-
- # 性能分析
- 好的，没问题。对于DBA（数据库管理员）来说，Linux性能排查是核心技能之一，因为数据库的性能与底层操作系统的状态息息相关。
  
  下面我为您整理了一份详尽的Linux性能排查工具指南，以表格形式呈现，涵盖了综合、CPU、内存、I/O和网络五个方面。每个命令都包含了其作用、常用示例以及最重要的**“DBA关注点”**，以帮助您快速将系统指标与数据库性能问题关联起来。
- ### **Linux性能问题排查基本思路**
  
  在深入了解工具之前，先建立一个排查框架：
- **全局检查**：从整体负载入手，判断问题的大致方向。
	- 使用 `uptime` 查看负载（Load Average）。
	- 使用 `dmesg | tail` 查看有无内核级别的严重错误（如OOM Killer）。
	- 使用 `top` 或 `htop` 快速浏览CPU和内存的概况。
- **定位瓶颈**：根据全局检查的结果，判断是CPU、内存、I/O还是网络瓶颈。
	- `vmstat` 和 `sar` 是定位具体瓶颈方向的利器。例如，`vmstat` 输出中 `wa` 值持续偏高，大概率是I/O问题。`si`/`so` 值不为0，则是内存问题。
- **深入分析**：使用特定领域的工具深入挖掘根源。
	- **I/O问题**：用 `iostat` 查看具体磁盘的压力，用 `iotop` 查看是哪个进程在读写。
	- **CPU问题**：用 `mpstat` 查看CPU核的负载是否均衡，用 `pidstat` 或 `perf` 分析具体是哪个进程/线程/函数消耗了CPU。
	- **内存问题**：用 `free -h` 查看内存和交换空间，检查数据库自身的内存配置参数。
	- **网络问题**：用 `ss` 或 `netstat` 检查连接状态，用 `sar -n DEV` 查看历史流量。
- **关联数据库**：将系统层面的发现与数据库内部的监控工具（如Oracle的AWR, PostgreSQL的`pg_stat_activity`等）进行关联，最终定位到具体的SQL或会话。
  
  ---
- ### **Linux性能排查工具速查表**
- #### **一、 综合性能监控工具 (Overall Performance Tools)**
  
  这类工具提供系统整体的视图，是排查的第一站。
  
  | 命令 (Command) | 主要作用 (Primary Purpose) | 常用示例 (Common Usage Example) | DBA关注点 (Key Points for DBAs) |
  | ---- | ---- | ---- | ---- |
  | **`top` / `htop`** | 实时动态显示系统中各个进程的资源占用状况，是交互式排查的首选。 | `top` (按`1`可看各CPU核，按`M`按内存排序，`P`按CPU排序)<br>`htop` (更友好的彩色界面) | **Load Average**: 是否超过CPU核数。**%CPU(s)**: `us`(用户态CPU)高说明数据库进程忙；`sy`(内核态CPU)高可能和IO或系统调用有关；`wa`(IO等待)高是典型的IO瓶颈；`id`(空闲)低说明CPU资源紧张。<br>**RES/VIRT**: 数据库进程占用的物理/虚拟内存是否符合预期。**进程列表**: 快速定位消耗资源最多的数据库相关进程。 |
  | **`uptime`** | 快速查看系统已运行时间、当前用户数和系统平均负载（1、5、15分钟）。 | `uptime` | **Load Average**: 如果1分钟负载远高于15分钟负载，说明系统压力在近期急剧增大。负载持续高于CPU逻辑核心数，意味着CPU存在瓶颈，任务需要排队等待。 |
  | **`vmstat`** | 全能的性能分析工具，能实时输出系统的进程、内存、交换、I/O、CPU活动信息。 | `vmstat 1 5`  (每1秒采样一次，共采样5次) | **procs**: `r`(运行队列)列长期大于CPU核数，CPU瓶颈；`b`(阻塞队列)列有值，说明有进程在等待IO等资源。<br>**memory**: `swpd`有值且`si`/`so`(换入/换出)列不为0，说明在用交换分区，对数据库性能是**致命打击**。**cpu**: `wa`(iowait)列持续高于20-30%，说明IO瓶颈严重。 |
  | **`sar`** | 功能强大的系统活动信息收集和报告工具，最大优势是可以查看历史数据。 | `sar -u 1 3` (CPU使用率)<br>`sar -d 1 3` (磁盘活动)<br>`sar -r 1 3` (内存使用)<br>`sar -n DEV 1 3` (网络流量)<br>`sar -f /var/log/sa/saDD` (查看历史) | **历史数据分析**: “昨天下午3点数据库很慢”这类问题的完美排查工具。通过 `-f` 参数加载历史文件，结合 `-s` 和 `-e` 参数指定时间范围，可以回溯任意时间的系统性能指标。 |
  | **`dmesg`** | 显示或控制内核环形缓冲区的内容，用于诊断硬件、驱动程序的错误信息。 | `dmesg管道tail -n 50`<br>`dmesg -T` (人性化显示时间)<br>`dmesg 管道 grep -i "error管道oom"` | **OOM (Out of Memory) Killer**: 如果看到 `Out of memory: Kill process ...`，说明内存严重不足，内核杀掉了某个进程（很可能是数据库）。<br>**磁盘错误**: `I/O error`, `sector error`等信息，预示着磁盘硬件故障。 |
- #### **二、 CPU性能分析工具 (CPU Performance Tools)**
  
  当 `top` 或 `vmstat` 显示CPU是瓶颈时，使用以下工具深入分析。
  
  | 命令 (Command) | 主要作用 (Primary Purpose) | 常用示例 (Common Usage Example) | DBA关注点 (Key Points for DBAs) |
  | ---- | ---- | ---- |
  | **`mpstat`** | 实时或历史地报告每个可用CPU的活动状态，是分析多核CPU负载是否均衡的利器。 | `mpstat -P ALL 1 3` (显示所有CPU核的统计，每秒1次，共3次) | **CPU负载均衡**: 观察各CPU核的 `%usr` 和 `%idle`。如果某个核（如CPU0）接近100%使用率，而其他核很空闲，可能是数据库中存在单线程执行的瓶颈SQL或操作。 |
  | **`pidstat`** | 监控特定进程或所有进程的CPU、内存、I/O等统计信息。 | `pidstat -u -p <PID> 1` (监控某进程CPU)<br>`pidstat -u -p <PID> -t 1` (监控某进程下所有线程的CPU) | **定位到线程**: 对数据库进程使用 `-t` 参数，可以清晰地看到是哪个线程（如前台会话线程、后台的LGWR/DBWR等）在消耗CPU，从而精确定位问题源头。 |
  | **`perf`** | Linux下终极性能分析工具，基于性能事件采样，可进行函数级别的性能剖析。 | `perf top` (实时显示热点函数)<br>`perf record -g -p <PID> sleep 10` (记录某进程10秒的函数调用图)<br>`perf report` (分析记录结果) | **热点函数分析**: 当CPU占用高但无法从SQL层面解释时，用 `perf` 可以分析出是数据库内核的哪个C函数在消耗CPU，用于非常深入的底层分析或向数据库厂商提报告。 |
- #### **三、 内存性能分析工具 (Memory Performance Tools)**
  
  | 命令 (Command) | 主要作用 (Primary Purpose) | 常用示例 (Common Usage Example) | DBA关注点 (Key Points for DBAs) |
  | ---- | ---- | ---- |
  | **`free`** | 快速查看系统内存和交换空间的使用情况。 | `free -h` (以人类可读格式显示) | **swap/used**: swap的使用量必须为0或接近0。一旦数据库服务器开始大量使用swap，性能会急剧下降。 **available**: 这是当前应用**真正可用**的内存量，比`free`列更有意义，因为它包含了可回收的cache。如果`available`持续走低，说明内存压力大。 |
  | **`/proc/meminfo`** | `free`命令的数据来源，提供了更详细的内存信息。 | `cat /proc/meminfo` | **HugePages**: 关注 `HugePages_Total`, `HugePages_Free`。数据库（如Oracle, PG）通常建议使用大页内存以提升性能，这里可以确认大页的配置和使用情况。<br>**Slab/SReclaimable**: 内核数据结构缓存，如果 `SUnreclaim` 异常大，可能存在内核内存泄漏。 |
- #### **四、 磁盘I/O性能分析工具 (I/O Performance Tools)**
  
  当 `top` 的 `wa` 指标或 `vmstat` 的 `wa` 和 `b` 列异常时，使用以下工具。
  
  | 命令 (Command) | 主要作用 (Primary Purpose) | 常用示例 (Common Usage Example) | DBA关注点 (Key Points for DBAs) |
  | ---- | ---- | ---- |
  | **`iostat`** | 监控系统I/O设备和CPU的活动情况，是诊断I/O瓶颈的核心工具。 | `iostat -dmx 1 5` (显示详细的、兆字节单位的磁盘统计信息) | **%util**: 磁盘繁忙程度百分比。如果接近100%，说明该磁盘I/O饱和，成为瓶颈。**await**: 平均每次I/O请求的等待时间（毫秒）。这个值最关键，它包括了队列等待和设备处理时间。对于SSD，应在1ms以下；对于HDD，5-15ms是正常范围。持续高于20ms说明存在严重I/O性能问题。**avgqu-sz**: 平均I/O队列长度。值持续偏高说明I/O请求堆积。**r/s, w/s**: 每秒读/写次数 (IOPS)。**rkB/s, wkB/s**: 每秒读/写数据量 (吞吐量)。结合这几项可判断I/O模式。 |
  | **`iotop`** | 类似于 `top`，但按I/O使用量对进程进行排序，直观显示哪个进程在进行大量的读写。 | `iotop` <br>`iotop -o` (只显示正在活动的进程) | **定位I/O元凶**: 直接找出是哪个数据库进程（或非数据库进程，如备份、批处理）造成了I/O压力，非常直观。 |
  | **`lsof`** | 列出当前系统打开的文件。 | `lsof -p <PID>` | **确认文件访问**: 查看数据库进程正在读写哪些数据文件、日志文件、临时文件等，有助于确认I/O行为是否符合预期。 |
- #### **五、 网络性能分析工具 (Network Performance Tools)**
  
  | 命令 (Command) | 主要作用 (Primary Purpose) | 常用示例 (Common Usage Example) | DBA关注点 (Key Points for DBAs) |
  | ---- | ---- | ---- |
  | **`ss`** | `netstat`的现代替代品，用于获取socket统计信息，速度更快，信息更全。 | `ss -s` (网络使用统计摘要)<br>`ss -tan` (显示所有TCP连接)<br>`ss -tan state established 管道 wc -l` (统计已建立的连接数) | **连接数**: 监控`ESTABLISHED`状态的连接数，如果数量异常暴增，可能存在应用端的连接池问题或连接泄漏。<br>**队列积压**: `Recv-Q`和`Send-Q`列如果长期有非零值，说明内核的网络缓冲区有数据积压，可能是应用处理慢或网络拥塞。 |
  | **`netstat`** | 传统的网络连接、路由表、接口统计信息查看工具。 | `netstat -anp 管道 grep <PORT>` | **监听端口**: 确认数据库的监听端口是否正常启动并处于`LISTEN`状态。 |
  | **`ping` / `mtr`** | 检查网络连通性和延迟。`mtr`结合了`ping`和`traceroute`的功能。 | `ping <app_server_ip>` <br>`mtr <app_server_ip>` | **网络延迟和丢包**: 检查数据库服务器到应用服务器之间的网络质量。高延迟或丢包会直接影响SQL响应时间。 |
  | **`iftop`** | 类似于`top`，但用于实时监控网络接口的带宽使用情况。 | `iftop -i <interface>` | **带宽占用**: 查看哪个IP地址（应用服务器）与数据库之间的通信占用了最多带宽，有助于发现异常流量。 |