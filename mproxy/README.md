mproxy
======

__mproxy__ (short for "_malicious proxy_") is a man-in-the-middle traffic forwarding tool. It accepts socket connections as a server, and forwards traffic to a real server as client(s). In the context of OpenFlow protocol, OFP switches connect to mproxy as if the latter were a OFP controller, and for each client mproxy creates a socket connection with the real OFP controller so that mproxy seems a host of one or more OFP switch.

Let _C_ denote a client, _S_ the real server, and _P_ mproxy. There are four directions how traffic flows:

 * in-flow request: _C_->_P_
 * in-flow response: _P_ -> _C_
 * out-flow request: _P_ -> _S_
 * out-flow response: _S_ -> _P_

By "malicious", it means that mproxy has the potential to modify the TCP payload of any of the four directions according to a predefined set of rules. For example, it can parse OFP PDU and modify it according to the rules loaded. This part is currently under development.

# Installation

## Environment

mproxy currently supports Linux and is tested on Xubuntu 14.04 64-bit only, with compiler g++ 4.8.2. However, any OS that supports POSIX standard and has C++11 compiler available should compile the program.

## Pre-requisite

mproxy requires [libevent](http://libevent.org/) of version 2.0 or greater. One can either compile and install `libevent` from source, or install from packages. For Ubuntu, the package is `libevent-dev`.

## Compilation / Execution

```bash
# to compile
make

# to compile in debug mode
make debug

# to show help info
./main -h

# to run with default port 6633 and default server dest 127.0.0.1:6635
./main

# to run with another port and server
./main --port 1234 --target localhost:5678

# to run with valgrind
make valgrind

# to clean
make clean
```

# Mechanism

## Man-in-the-middle

As described in the intro, this tool forwards traffic as a proxy. However, it does not hides its existence. Its clients treat mproxy as the server, and the real server treats different connections with mproxy as different clients.

## IPv4 / IPv6 / Unix Socket

Clients can reach mproxy by the hostname mproxy is located and the port number mproxy listens to (by default, 6633), while mproxy itself can talk to any server by IPv4 address and port, IPv6 address and port, or Unix socket address.

Current test covers only IPv4 so far.

## Proxy vs Sniffer

There are two major approaches how we deal with the traffic: acting as a proxy and acting as a sniffer.

Sniffer approach makes it easier to preservs the order of messages, and makes it possible to hide this man-in-the-middle process from client-server communication. However, it is inefficient because all packets through a _device_ (not a specific port) are monitered, and are sent to the program for analysis according to some rules (see [libtins](http://libtins.github.io) and `libpcap`). And such sniffer program tends to require higher privilege to run. Besides, work outside the sniffer program is needed to drop the original packet if you do packet injection; otherwise the destination receives two packets. Such work may be done in iptables, etc.

By contrast, in proxy approach it is more difficult to keep messages in-order. Further, with standard socket API the packet headers are hidden. But the program generally requires low privilege, and can be more efficient.

As indicated by the program name, we use proxy approach.

## Event-driven Model

Polling is inefficient. Semaphore-based scheduling does not adapt well to the complex client-server data stream and does not do well in keeping messages in order (at least the solution is unclear to me without considerable amount of thinking). So I pursue event-driven model instead.

mproxy leverages [libevent](http://libevent.org/) to achieve asynchronous I/O on sockets. This dramatically improves the responsiveness and resource consumption compared to the behavior in semaphore-based codebase.

## Multi-threading

If the client requests and server responses are linear, there would be no need to do with multi-threading. However, linear request-response is simply not the case. For OpenFlow protocol, switches and controller establish resident socket connection and after greeting step, communications between them can be initiated from either side (which makes semaphore-based scheduling not adaptive) at various conditions. In short, multi-threading is inevitable. The question is not why, but how.

Possible approaches are multi-in-single-out (each inflow is dealt with by a thread, and one thread deals with outflow) and multi-in-multi-out (each inflow is dealt with by a thread, and so is each outflow). mproxy adopts the latter. 

To be specific, in mproxy, each socket file descriptor is designated to a thread to monitor. Main thread monitors incoming connections, and when a client arrives, creates one thread for inflow and outflow, respectively. The designated thread will monitor be waken up to work as soon as an event occurs to the file descriptor it monitors (e.g., an event indicating something is readable). Once waken up, the thread can read the message, parse it, and either put the message to send to a task queue (discussed later) or send the message directly.

## Task queue

It can take different amount of time to parse different types of messages, which can mess up the order of messages even when the order is critical. This is why mproxy implements a task queue.

In OpenFlow protocol it can take equal amount of time to determine if the message implies strict ordering by checking the message type. If the order is not critical, a thread is free to send out the processed message to destination by itself as soon as the processing is done. However, if ordering is important, the thread should first grab a seat in task queue, process the data, and then signal the mutex associated with that seat to indicate the message is ready to be delivered. A worker will wait for tasks to be put on task queue, wait for the mutex to be ready, and deliver messages in order.

So the multi-thread architecture in mproxy is M(ain)+M(ulti)I(n)M(ulti)O(out)+W(orker).

## Clients

To some extent, mproxy sees each socket file descriptor as a client regardless of who sits on the other side. A pair of inflow and outflow sockets are mirrorred clients and represented with the same `ClientDetail` data structure (with socket and forward socket values swapped, though). The difference is that Main thread assigns different event handlers to deal with events occuring to the socket file descriptor.

# Malicious Template

Currently working on it. Need to define the format, field types, allowed modifications on each field, etc.
