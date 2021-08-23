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
	
	![Pasted image 20210806102910](https://user-images.githubusercontent.com/33850772/128448406-c0635c57-7488-4c0c-97c7-0073902000a1.png)

* LT + ET
	* reactor
	
	![Pasted image 20210805225143](https://user-images.githubusercontent.com/33850772/128377611-6701a319-0fde-4fd9-8f13-ddb25229d7e1.png)
	* proactor
	
	![Pasted image 20210806103037](https://user-images.githubusercontent.com/33850772/128448444-f30bd34d-a955-42b2-95f6-4ad0bf46f69d.png)
* ET + LT
	* reactor
	
	![Pasted image 20210805225256](https://user-images.githubusercontent.com/33850772/128377684-8171c1f6-45d5-44c5-9683-5217e8dc797b.png)
	* proactor
	
	![Pasted image 20210806103147](https://user-images.githubusercontent.com/33850772/128448493-95f44f3d-ef64-4ce9-96e7-fb00c61b7b36.png)
* ET + ET
	* reactor
	
	![Pasted image 20210806220126](https://user-images.githubusercontent.com/33850772/128522093-654faf52-17fa-47c8-8713-b6e082a2cad1.png)
	* proactor
	
	![Pasted image 20210806220059](https://user-images.githubusercontent.com/33850772/128522042-12f78481-0860-49cd-a842-c20a45c299f0.png)


* 参考资料：
	* 游双 <<Linux高性能服务器编程>>
	* [qinguoyi](https://github.com/qinguoyi/TinyWebServer)
	* [markparticle](https://github.com/markparticle/WebServer)

