# Log

## Week 6

### 02/17/2015

 * For convenience, installation scripts for Mininet and Ryu are written at 
   `/misc/inst/install_mininet.sh` and `/misc/inst/install_ryu.sh`, respectively. 
   Tested on Ubuntu 14.04.
   
 * A demo topology from Mininet Python binding is at 
   `/misc/demo/demo_mininet_topo.py`. The sample command to construct the topo is at
   `/misc/demo/demo_mininet_topo.sh`.
   
	 * The reason why we use `remote` controller instead of Mininet's built-in 
      `ryu` is that we can gain better control over Ryu. We may want its 
      graphics features, various apps, etc.
   
 * While Mininet creates a virtual network, from the viewpoint of host 
   OS it is not a network emulator like ns-3. The network looks _real_ on the 
   host. Mininet does not hold packets itself. This means we cannot inject 
   tests by manipulating Mininet like what we did on ns-3 in Turret.
   
 * Given this fact, what I am thinking is that 
	 
	 * to test controllers, we need instrumented switches (we write them), and 
	 * to test switches, we need instrumented controllers.
   
 * Ryu provides beautiful API for writing a switch.
 
 * To test a controller, we need to define the following:
	
	 * How do we evaluate the level of error tolerance of a controller? 
	
		 * If the packet can reach its destination when an error occurs on 
		   the path.
		  
		 * The amount of effort the controller uses to recover from the error.
	
	 * How do we evaluate the performance of a controller?
	 
	 	 * Packet per second?
	 
	 * And probably in more aspects.
 
 * We can write a malicious switch to inject attacks against the controller.
 
 * To get better perspective we can test a controller / switch, we may be 
   interested to learn [the unit tests of Ryu](https://github.com/osrg/ryu/tree/master/ryu/tests).
 
 
 
