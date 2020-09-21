# ReadMe
目标:提供搜索、推荐、广告等服务端场景下, 工业生产环境下可用的高性能常用基础组件实现, 而不是coding practice的玩具demo。

# 目录结构
## src
1. blocking_queue.h 提供传统的多生产者多消费者的阻塞队列实现BlockingMap，提供尽可能丰富用户接口。
2. blocking_map.h 提供带有阻塞语意的map，阻塞语意上类似BlockingQueue，区别是这个数据结构会在调用take(key)时，对获取特定key对应的value时阻塞。
3. log.h 提供简单的异步日志打印框架(TODO)

## unittest/ut_framework
1. ./unittest/ 应用ut_framework中单元测试框架，对基础组件的测试
2. ./ut_framework/ 单元测试框架的简单实现，用法上类似google的uittest测试框架。

