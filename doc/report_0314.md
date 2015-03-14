Mid-March Report
================

This document is a summary of what has been done as of March 14, 2015, and what to do next.

# Getting Started

The first few weeks were spent on getting familiar with concepts of software-defined networking and OpenFlow. Then we playwed with network "emulator" [mininet](http://mininet.org/), which allows one to create a virtual network with any SDN controller and switch, and in any topology. Some [demo scripts](/misc/demo) to start mininet and build topology are present in this repository.

Then we spent more time on comparing SDN controllers, most famous ones of which are [NOX](http://www.noxrepo.org/nox/about-nox/), [POX](http://www.noxrepo.org/pox/about-pox/), [OpenDaylight](http://www.opendaylight.org/software), and [Ryu](https://osrg.github.io/ryu/). We picked Ryu as our reference controller because it supports all versions of OpenFlow protocols (whereas NOX and POX support only OFP 1.0) and is under active development (whereas NOX and POX are in stagnation). The API of Ryu is rich and well documented.

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

## Proxy

