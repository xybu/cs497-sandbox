Malicious Proxy
===============

This malicious proxy acts as a man in the middle between OpenFlow switches and controllers. It intercepts messages, parses them, does actions on them, and sends them as if the messages were not modified.

# Mechanism

Each connection to the proxy is handled by two threads.

# Compilation

```bash
# to compile
make

# to compile in debug mode
make debug

# to clean
make clean
```

# Known Issues

 * The order of messages cannot be mirrorred.
 	 * Different messages need different amount of time to parse.
 	 * No acceptable solution to preserve the order in inflow and outflow.
 		 * Either too inefficient or not working.
 * Mechanism to free up all resources at exit time has not been established.
 * Passive socket recv/send should be changed to an event-driven model.
 * Haven't tested on IPv6 (though it is supported).

# Proxy approach vs. Sniffer approach

Sniffer approach preserves the order of messages (good), but requires higher privilege to run the program (bad). All packets through a device are monitered and (good and bad) those useful are filtered out for processing. The deadly problem is there is no effective way to drop the original packet before sending the modified packet. Need to fix timestamp and checksum if payload is changed.

Proxy approach makes it hard to preserve order (very bad). Low requirement on privileges (good). TCP headers are hidden.
