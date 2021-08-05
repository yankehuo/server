* 缘起
	* 游双的著作读过之后，练练手
* 线程池
	* 利用互斥锁和条件变量实现线程同步
	* 共享队列为function object，可直接运行
	* shared_ptr管理资源
	* RAII：资源获取即初始化
* HTTP解析
	* 状态机设置图形！！！
	* request 解析HTTP请求
	* response 将响应报文写入buffer
	* conn 处理连接

* buffer
	* 缓冲：读写指针 
	* muduo设计
	* 标准库容器自增长

* logbq
	* 阻塞队列
	* 异步写日志 未设置队列大小同步写日志

*  server
	*  listenfd connfd 设置ET LT等
	*  设置处理模式 proactor或者reactor

* heaptimer
	* 定时器小根堆 
	* 获得当前的时间加上过期时间作为expireds 与 当前时间比较
		* 小于，则清除，设置超时时间参数
		* 大于，则将差值设为epoll_wait函数的超时参数
	
	
* 模式对比
	* reactor 模拟的proactor 
	* ET LT
	* 时间
	* 线程
	* 是否开启日志

* LT + LT
	* reactor
	![[Pasted image 20210805224126.png]]
	* proactor
	![[Pasted image 20210805224735.png]]
* LT + ET
	* reactor
	![[Pasted image 20210805225143.png]]
	* proactor
	![[Pasted image 20210805224918.png]]
* ET + LT
	* reactor
	![[Pasted image 20210805225256.png]]
	* proactor
	![[Pasted image 20210805225426.png]]
* ET + ET
	* reactor
	![[Pasted image 20210805230716.png]]
	* proactor
	![[Pasted image 20210805225631.png]]


* 参考资料：
	* 游双高性能服务器编程
	* qinguoyi https://github.com/qinguoyi/TinyWebServer
	* markparticle https://github.com/markparticle/WebServer

