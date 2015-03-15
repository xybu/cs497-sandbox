Mid-March Report
================

This document is a summary of what has been done as of March 14, 2015, and what to do next.

# Getting Started

The first few weeks were spent on getting familiar with concepts of software-defined networking and OpenFlow. Then we playwed with network "emulator" [mininet](http://mininet.org/), which allows one to create a virtual network with any SDN controller and switch, and in any topology. Some [demo scripts](/misc/demo) to start mininet and build topology are present in this repository.

Then we spent more time on comparing SDN controllers, most famous ones of which are [NOX](http://www.noxrepo.org/nox/about-nox/), [POX](http://www.noxrepo.org/pox/about-pox/), [OpenDaylight](http://www.opendaylight.org/software), and [Ryu](https://osrg.github.io/ryu/) (pronouced as "lew" in case one wonders). We picked Ryu as our reference controller because it supports all versions of OpenFlow protocols (whereas NOX and POX support only OFP 1.0) and is under active development (whereas NOX and POX are in stagnation). The API of Ryu is rich and well documented.

Having picked Ryu as the reference controller, we played with Ryu API, creating simple topology with Mininet Python API and making learning switches based on Ryu. This gave me first impressions on how the target software could be tested. A [malicious switch](/archive/simple_malicious_actions) based on Ryu was tried and documented in the repository.

# Middleware Approach

While we agreed on injecting a middleware between the SDN controller and switch to intercept packets, it turned out a bad idea to leverage Ryu's API to test an arbitrary SDN controller / switch, because the correctness of the result of attacking the test target is based on the correctness of Ryu's code base, whereas Ryu itself is a target to attack. Besides, inserting a Ryu unit below the controller and above any switch changes the actual network topology.

To make it clear what we cannot modify, the middleware (or man-in-the-middle) must NOT

 * modify host OS configurations including iptables and firewall settings
 * modify existing network topology

so as to test the controller and switch in a context similar to how they run in the real world. (Specialized scenarios may not find problems in real-world scenario.)

This imples that the middleware is neither a SDN controller nor a SDN switch. All it can do is to intercept packets, parse and modify them if necessary, and send out the packets as if nothing happens.

In other words, the middleware works like either a sniffer or a proxy.

## Sniffer

Sniffer software like [wireshark / tshark](https://www.wireshark.org/) and its underlying library [libpcap](http://www.tcpdump.org/) allow one to intercept traffic that passes through a network interface (device) and log packets that meet a set of filter rules before letting them pass.

A middleware that implements sniffer approach was made and tested. It did not meet our need because 1) we will need to modify the packet and 2) we need to drop the original packet before sending the modified packet, both of which require some kind of hack on the host OS. Besides, if switches and controllers are located on more than one host, a sniffer must run on each host so as to intercept all traffic. This is not flexible.

More about comparisons of sniffer approach to proxy approach is [in the README document](/mproxy/README.md#proxy-vs-sniffer) of our current ongoing product, [mproxy](/mproxy/).

## Proxy

A proxy accepts traffic on behalf of a destination and forwards the traffic to the destination at its convenience. Our current approach, [mproxy](/mproxy/), implements a proxy that communicates with a server on behalf of one or more client.

[This document](/mproxy/README.md) describes in detail about mproxy.

An important feature of this program is the event-driven model. It is way better than blocking-call based polling or semaphore-based scheduling in terms of both flexibility and performance.

# Next

Our next steps include:

* Make a OFP parser that can analyze the intercepted messages;
* Based on the parser, implement malicious actions that can modify some field in OFP message (e.g., set a field above its allowed maximum), or change the delivery behavior (delay, drop, or repeated delivery) of the message, according to a predefined set of rules;
* Add this component to mproxy;
* Test how well the proxy will work to attack the target SDN controller / switch.

