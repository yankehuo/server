* 缘起
	* 游双的著作读过之后，练练手
* 线程池
	* 利用互斥锁和条件变量实现线程同步
	* 共享队列为function object，可直接运行
	* shared_ptr管理资源
	* RAII：资源获取即初始化
* HTTP解析
	* 状态机设置
	![httpparse](https://user-images.githubusercontent.com/33850772/128378264-bd17ee04-56fb-4dbd-bce2-0b52d70b6cae.png)
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
	
	![Pasted image 20210805224126](https://user-images.githubusercontent.com/33850772/128377431-1653cb53-bdd0-457a-b6f1-37a8f4adb018.png)
	* proactor
	
	![Pasted image 20210805224735](https://user-images.githubusercontent.com/33850772/128377512-2212c0f1-3f76-4e05-9fdb-707da8831624.png)
* LT + ET
	* reactor
	
	![Pasted image 20210805225143](https://user-images.githubusercontent.com/33850772/128377611-6701a319-0fde-4fd9-8f13-ddb25229d7e1.png)
	* proactor
	
	![Pasted image 20210805224918](https://user-images.githubusercontent.com/33850772/128377645-0e99abfe-d20e-4df2-bc49-5dd483176a0d.png)
* ET + LT
	* reactor
	
	![Pasted image 20210805225256](https://user-images.githubusercontent.com/33850772/128377684-8171c1f6-45d5-44c5-9683-5217e8dc797b.png)
	* proactor
	
	![Pasted image 20210805225426](https://user-images.githubusercontent.com/33850772/128377744-1760c546-2631-415a-ac5e-c2dc8562b947.png)
* ET + ET
	* reactor
	
	![Pasted image 20210805230716](https://user-images.githubusercontent.com/33850772/128377780-ccc2c5bb-0bca-44fe-b9fd-0191073da898.png)
	* proactor
	
	![Pasted image 20210805225631](https://user-images.githubusercontent.com/33850772/128377810-789c87b7-6897-4576-a530-6aedfd87ca9b.png)


* 参考资料：
	* 游双高性能服务器编程
	* qinguoyi https://github.com/qinguoyi/TinyWebServer
	* markparticle https://github.com/markparticle/WebServer

