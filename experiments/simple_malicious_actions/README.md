Simple Malicious Action
=======================

This experiment explores what happens when the network has a 
malicious controller connected, with Ryu framework's 
`simple_switch_13.py` controller code and mininet's `OVSSwitch` 
switch code as reference implementations.


# Scenario


## Mininet Topology

The script `mn.py` builds a mininet work with two remote controllers:

 * `c0`: remote controller running on `127.0.0.1:6633`. Used to 
   connect with the target controller we want to test.

 * `c1`: remote controller running on `127.0.0.1:6634`. Used to 
   connect to the malicious controller we implemented with Ryu API.

Then it creates the following topology:

 * switches `s0`, `s1`, linked to `c0` and `c1`, respectively. The 
   switches are mininet's default implementation `OVSSwitch`.
 * two sets of hosts, `hosts[0]` and `hosts[1]`.
 	 * each set of hosts, `hosts[i]`, is linked to switch `si`.
 	 * each `hosts[i]` contains two hosts, `hosts[i][0]` and 
 	   `hosts[i][1]`.

and starts mininet command-line interface.


## Automated Script

The script `run.sh` automates the setup work:

 * It first kills all residual `ryu-manager` processes and cleans 
   mininet cache.

 * Then it tries to find a suitable terminal program (not a shell; 
   e.g., `xfce4-terminal` or `xterm`) to fork later.

 * Next it forks a new terminal to start the benign, target controler.
   In our case it is the `simple_switch_13.py` controller shipped with
   Ryu framework. It listens to port `6633` and acts as `c0`.

 * Subsequently it forks another new terminal to start the malicious 
   controller (`c1`). It is the `malicious_ryu_13.py` code in this 
   directory (will explain later).

 * It then waits for 3 seconds so that the two controllers start 
   before next step.

 * It runs `mn.py` to initialize and start the mininet. Current 
   terminal now runs mininet's CLI shell.

 * When mininet's CLI shell exits, the script kills the two 
   controller processes, does housekeeping, and exits.


## Malicious Controller

The script `malicious_ryu_13.py` is a malicious controller based on 
the original `simple_switch_13.py`. Currently we focus on `PacketIn` 
event (http://ryu.readthedocs.org/en/latest/ofproto_v1_2_ref.html#packet-in-message),
printing what packet is received, and does malicious work before the packet gets sent
to its destination. The message is not modified (may explore later).

There are two types of malicious actions: (1) delaying `send_msg`
call, and (2) calling `send_msg` zero or more time. To separate 
different kinds of malicious actions, the controller does the 
following:

 * Randomly decide which malicious action to take for the packet.
   Each malicious action has equal probability to be chosen.

 * If the malicious action is to delay sending, the controller then 
   generates a random number _d_ (_d_ < 1), and have the thread sleep 
   for d second before calling `send_msg`. Choosing large _d_ makes no
   practical sense because we already know the sender will detect a
   timeout.

 * If the malicious action is to repeat sending, the controller
   generates a random integer _x_ from {0, 1, 5}, and call `send_msg`
   _x_ times. Having _x_ equal to 0 is equivalent to dropping the 
   message. Having _x_ equal to 1 is equivalent to no malicious 
   action.

The ranges can be adjusted with ease by modifying the source code.

Before this setup, another approach is attempted: first decide _x_, 
and then before each call to `send_msg`, decide a delay time _d_.
However, it turns out not very useful because mostly likely all 
initial pings fail, and we cannot distinguish a failure caused 
directly by the malicious action from a failure caused by a previous
execution of malicious action.


# Observations

We do a consecutive of `pingallfull` commands after both controllers
get stabilized.

## Baseline

Without malicious action injected (however, the malicious 
controller still calculates one random number), three consecutive rounds of `pingallfull` give:

```bash
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 4.522/4.522/4.522/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 8.822/8.822/8.822/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 9.855/9.855/9.855/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.225/0.225/0.225/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 8.371/8.371/8.371/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 9.785/9.785/9.785/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.357/0.357/0.357/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.204/0.204/0.204/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 11.250/11.250/11.250/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.694/0.694/0.694/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.321/0.321/0.321/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.715/0.715/0.715/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.029/0.029/0.029/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.024/0.024/0.024/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.028/0.028/0.028/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.046/0.046/0.046/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.034/0.034/0.034/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.029/0.029/0.029/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.027/0.027/0.027/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.050/0.050/0.050/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.033/0.033/0.033/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.028/0.028/0.028/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.025/0.025/0.025/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.036/0.036/0.036/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.047/0.047/0.047/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.029/0.029/0.029/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.025/0.025/0.025/0.000 ms
```

Note that the switches quickly learn the datapaths from a source
to a destination. Starting from the second round, no `PacketIn` 
packet is observed on either `c0` or `c1`.

In the first round, the average of average amount of time it takes to

 * Ping between `hosts[0]` and `hosts[1]`: AVERAGE(8.822 + 9.855 + 
   8.371 + 9.785 + 0.357 + 0.204 + 0.694 + 0.321) = 4.801125 ms
 * Ping within `hosts[0]`: AVERAGE(4.522 + 0.225) = 2.3735 ms
 * Ping within `hosts[1]`: AVERAGE(11.25 + 0.715) = 5.9825 ms

One may need to run the baseline a multiple of times to get a more 
accurate result.

## Delaying Only

Each message passed through `c1` has 50% chance to be delayed for 
_t_ < 1 second (average of _t_ is 0.5), and 50% chance to be sent 
out normally. Logging is turned off to save the time for the extra 
output.

Do three rounds of `pingallfull` and observe:

```bash
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 4.256/4.256/4.256/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 9.470/9.470/9.470/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 9.148/9.148/9.148/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.165/0.165/0.165/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 8.002/8.002/8.002/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 12.067/12.067/12.067/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.199/0.199/0.199/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.261/0.261/0.261/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 16.187/16.187/16.187/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.511/0.511/0.511/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.165/0.165/0.165/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.213/0.213/0.213/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.320/0.320/0.320/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.595/0.595/0.595/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.456/0.456/0.456/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.025/0.025/0.025/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.366/0.366/0.366/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.312/0.312/0.312/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.036/0.036/0.036/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.293/0.293/0.293/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.081/0.081/0.081/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.035/0.035/0.035/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.024/0.024/0.024/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.029/0.029/0.029/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.040/0.040/0.040/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.047/0.047/0.047/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.024/0.024/0.024/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.028/0.028/0.028/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.029/0.029/0.029/0.000 ms
mininet> 
```

For the first round, the average of average amount of time it takes
to 

 * Ping between `hosts[0]` and `hosts[1]`: AVERAGE(9.47 + 9.148 + 8.002 + 12.067 + 0.199 + 0.261 + 0.511 + 0.165) = 4.977875 ms
 * Ping within `hosts[0]`: AVERAGE(4.256 + 0.165) = 2.2105 ms
 * Ping within `hosts[1]`: AVERAGE(16.187 + 0.213) = 8.2 ms

There is very slight increase compared to the data in baseline.

While the learning capabilities gradually rid the communications of
the malicious action, the second round is still affected, including 
communications within `hosts[0]`.

## Repeating Only

Each packet that passes through `c1` has 1/2 chance being sent out
normally (by "normally", we mean no extra work compared to 
baseline), 1/6 chance being dropped, 1/6 chance being sent out with a very minimal delay caused by generating a random number and executing the `if` branch, 1/6 chance being sent out 5 times. 
Again, logging is turned off.

```bash
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 X 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 4.458/4.458/4.458/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 12.081/12.081/12.081/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 9.438/9.438/9.438/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.312/0.312/0.312/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 17.444/17.444/17.444/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 11.086/11.086/11.086/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 2.758/2.758/2.758/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.186/0.186/0.186/0.000 ms
 h10->h11: 1/0, rtt min/avg/max/mdev 0.000/0.000/0.000/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.837/0.837/0.837/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.805/0.805/0.805/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.649/0.649/0.649/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.528/0.528/0.528/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.618/0.618/0.618/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.441/0.441/0.441/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.023/0.023/0.023/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.387/0.387/0.387/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.308/0.308/0.308/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.025/0.025/0.025/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.334/0.334/0.334/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.028/0.028/0.028/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.088/0.088/0.088/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.033/0.033/0.033/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.335/0.335/0.335/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.566/0.566/0.566/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.408/0.408/0.408/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.027/0.027/0.027/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.431/0.431/0.431/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.333/0.333/0.333/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.035/0.035/0.035/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.033/0.033/0.033/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.349/0.349/0.349/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.078/0.078/0.078/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.080/0.080/0.080/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.313/0.313/0.313/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.552/0.552/0.552/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.449/0.449/0.449/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.025/0.025/0.025/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.375/0.375/0.375/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.328/0.328/0.328/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.035/0.035/0.035/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.319/0.319/0.319/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.035/0.035/0.035/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.345/0.345/0.345/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.569/0.569/0.569/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.410/0.410/0.410/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.029/0.029/0.029/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.426/0.426/0.426/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.287/0.287/0.287/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.029/0.029/0.029/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.279/0.279/0.279/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.034/0.034/0.034/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.028/0.028/0.028/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.038/0.038/0.038/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.351/0.351/0.351/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.540/0.540/0.540/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.476/0.476/0.476/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.028/0.028/0.028/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.452/0.452/0.452/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.389/0.389/0.389/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.040/0.040/0.040/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.053/0.053/0.053/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.319/0.319/0.319/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.035/0.035/0.035/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.113/0.113/0.113/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
mininet> 
```

`pingallfull` is run 6 times but we no longer observe that all ping 
times fall into a time less than 0.1 ms. This is unexpected because
on the last few rounds, no `PacketIn` event occur on any controller.

Also it is hard to separate the time consumed on repeatedly calling 
`send_msg` (which might act as a delay) from the reall effect.

## Malicious Actions Combined

```bash
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 4.037/4.037/4.037/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 12.340/12.340/12.340/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 10.849/10.849/10.849/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.269/0.269/0.269/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 1027.221/1027.221/1027.221/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 29.376/29.376/29.376/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.488/0.488/0.488/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.370/0.370/0.370/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 47.948/47.948/47.948/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.419/0.419/0.419/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.202/0.202/0.202/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.324/0.324/0.324/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.306/0.306/0.306/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.600/0.600/0.600/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.432/0.432/0.432/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.028/0.028/0.028/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.463/0.463/0.463/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.314/0.314/0.314/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.035/0.035/0.035/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.028/0.028/0.028/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.307/0.307/0.307/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.050/0.050/0.050/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.054/0.054/0.054/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.038/0.038/0.038/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.034/0.034/0.034/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.027/0.027/0.027/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.034/0.034/0.034/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.037/0.037/0.037/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.046/0.046/0.046/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.027/0.027/0.027/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.026/0.026/0.026/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.060/0.060/0.060/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.034/0.034/0.034/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.029/0.029/0.029/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.078/0.078/0.078/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.033/0.033/0.033/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.041/0.041/0.041/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.036/0.036/0.036/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.029/0.029/0.029/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.035/0.035/0.035/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.026/0.026/0.026/0.000 ms
mininet> 
```

All ping time drops below 0.1 ms since the third round, unlike what 
we have observed in "Repeating Only" section. Running several more 
rounds does incur some ping time > 0.1 ms, though. But again, for 
those rounds we did not see controllers get `PacketIn` messages.

## Repeating 5 times

As an extra experiment to see how oversending affects the network, in this section the malicious controller has 50% chance to send a 
message normally and 50% chance to send it 5 times to the 
destination.

As usual, logging is turned off.

```bash
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 4.552/4.552/4.552/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 12.345/12.345/12.345/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 10.735/10.735/10.735/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.329/0.329/0.329/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 45.043/45.043/45.043/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 11.607/11.607/11.607/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.402/0.402/0.402/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.353/0.353/0.353/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 15.748/15.748/15.748/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.790/0.790/0.790/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.205/0.205/0.205/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.251/0.251/0.251/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.318/0.318/0.318/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.540/0.540/0.540/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.391/0.391/0.391/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.027/0.027/0.027/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.430/0.430/0.430/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.302/0.302/0.302/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.040/0.040/0.040/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.297/0.297/0.297/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.044/0.044/0.044/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.025/0.025/0.025/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.368/0.368/0.368/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.492/0.492/0.492/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.440/0.440/0.440/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.027/0.027/0.027/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.393/0.393/0.393/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.359/0.359/0.359/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.036/0.036/0.036/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.290/0.290/0.290/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.035/0.035/0.035/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.084/0.084/0.084/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.365/0.365/0.365/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.601/0.601/0.601/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.452/0.452/0.452/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.471/0.471/0.471/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.347/0.347/0.347/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.044/0.044/0.044/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.321/0.321/0.321/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.046/0.046/0.046/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.036/0.036/0.036/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.033/0.033/0.033/0.000 ms
mininet> 
```

Something similar to the result in "Repeating Only" section happened.

## Dropping Only

In this part we have the controller drop a message with 1/6 chance, 
and have it delivered normally with 5/6 chance.

As before, logging is turned off.

```bash
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 X X 
h01 -> h00 h10 h11 
h10 -> h00 h01 X 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 4.169/4.169/4.169/0.000 ms
 h00->h10: 1/0, rtt min/avg/max/mdev 0.000/0.000/0.000/0.000 ms
 h00->h11: 1/0, rtt min/avg/max/mdev 0.000/0.000/0.000/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.550/0.550/0.550/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 16.104/16.104/16.104/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 17.520/17.520/17.520/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 1.151/1.151/1.151/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.261/0.261/0.261/0.000 ms
 h10->h11: 1/0, rtt min/avg/max/mdev 0.000/0.000/0.000/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.924/0.924/0.924/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.884/0.884/0.884/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.755/0.755/0.755/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.257/0.257/0.257/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.546/0.546/0.546/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 1.261/1.261/1.261/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.050/0.050/0.050/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.669/0.669/0.669/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.350/0.350/0.350/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.034/0.034/0.034/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.039/0.039/0.039/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.334/0.334/0.334/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.035/0.035/0.035/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.029/0.029/0.029/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.028/0.028/0.028/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.316/0.316/0.316/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.494/0.494/0.494/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.488/0.488/0.488/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.026/0.026/0.026/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.498/0.498/0.498/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.331/0.331/0.331/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.033/0.033/0.033/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.030/0.030/0.030/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.317/0.317/0.317/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.033/0.033/0.033/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.099/0.099/0.099/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.471/0.471/0.471/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.518/0.518/0.518/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.441/0.441/0.441/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.029/0.029/0.029/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.480/0.480/0.480/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.348/0.348/0.348/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.336/0.336/0.336/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.033/0.033/0.033/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.035/0.035/0.035/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.098/0.098/0.098/0.000 ms
mininet>
```

Dropping message with 1/6 chance seems not as disastrous as sending
a message 5 times to destination with 50% chance. But what if we send
each message 5 times with 1/6 chance?

## Repeating with 1/6 Chance

In this part, the malicious controller send a message 5 times with 
1/6 chance and send the message once with 5/6 chance.

```bash
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 4.283/4.283/4.283/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 9.867/9.867/9.867/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 9.607/9.607/9.607/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.337/0.337/0.337/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 17.480/17.480/17.480/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 6.985/6.985/6.985/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.349/0.349/0.349/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.431/0.431/0.431/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 17.365/17.365/17.365/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.273/0.273/0.273/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.301/0.301/0.301/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.203/0.203/0.203/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.316/0.316/0.316/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.544/0.544/0.544/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.456/0.456/0.456/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.024/0.024/0.024/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.449/0.449/0.449/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.355/0.355/0.355/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.031/0.031/0.031/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.028/0.028/0.028/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.316/0.316/0.316/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.029/0.029/0.029/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.032/0.032/0.032/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.035/0.035/0.035/0.000 ms
mininet> pingallfull
*** Ping: testing ping reachability
h00 -> h01 h10 h11 
h01 -> h00 h10 h11 
h10 -> h00 h01 h11 
h11 -> h00 h01 h10 
*** Results: 
 h00->h01: 1/1, rtt min/avg/max/mdev 0.310/0.310/0.310/0.000 ms
 h00->h10: 1/1, rtt min/avg/max/mdev 0.435/0.435/0.435/0.000 ms
 h00->h11: 1/1, rtt min/avg/max/mdev 0.473/0.473/0.473/0.000 ms
 h01->h00: 1/1, rtt min/avg/max/mdev 0.028/0.028/0.028/0.000 ms
 h01->h10: 1/1, rtt min/avg/max/mdev 0.481/0.481/0.481/0.000 ms
 h01->h11: 1/1, rtt min/avg/max/mdev 0.383/0.383/0.383/0.000 ms
 h10->h00: 1/1, rtt min/avg/max/mdev 0.037/0.037/0.037/0.000 ms
 h10->h01: 1/1, rtt min/avg/max/mdev 0.040/0.040/0.040/0.000 ms
 h10->h11: 1/1, rtt min/avg/max/mdev 0.328/0.328/0.328/0.000 ms
 h11->h00: 1/1, rtt min/avg/max/mdev 0.035/0.035/0.035/0.000 ms
 h11->h01: 1/1, rtt min/avg/max/mdev 0.123/0.123/0.123/0.000 ms
 h11->h10: 1/1, rtt min/avg/max/mdev 0.051/0.051/0.051/0.000 ms
mininet> 
```

Controllers no longer get `PacketIn` messages since the second 
round. 

# Comparisons

 * Comparing "Repeating with 1/6 Chance" section with "Dropping
   Only" section, Dropping a message is more disastrous in that it fails the ping, but the ping failure does not have noticeable
   effect on the next `pingallfull` round.

 * Comparing "Baseline" and "Delaying Only" sections, an expected 
   delay of 0.5 second with 50% chance (that is, each message is 
   expected to be delayed for 0.25 second) results in way longer
   real delay than the injected delay.

 * Comparing "Repeating 5 times" (expected delivery: 0.5*1 + 0.5*5 
   = 3 times / msg) and "Repeating Only" (expected delivery: 0.5*1 + 
   1/6 + 5/6 = 1.5 times / msg; dropping allowed) sections with 
   "Baseline" section (expected delivery = 1 time / msg), more 
   repetition in message delivery results in negligible increase in 
   ping time. However, there are two outliers: `h01->h10` in 
   "Repeating 5 times" (almost tripled) and `h10-h11` in "Repeating 
   Only" (message dropping failed the ping).

 * As long as the malicious controller misbehave, consecutive 
   `pingallfull`s can no longer reach the optimal response time
   observed in "Baseline".

# Analysis and Questions

 * OVSSwitch has good ability to "learn" datapaths. As long as the 
   path can be established and there is no drastic change on the destination, a malicious controller cannot do much. For better message intercepting, we might need a malicious switch.

 * There is a question about the second and third rounds of 
   `pingallfull`s. Why can't the switches and hosts fully recover 
   from the malicious actions when they seem not to be interacting 
   with the malicious controller? 

    * Is there anything done outside `PacketIn` event? But even so, 
      since there is no malicious action buried outside `PacketIn`, 
      the ping time should still recover to the baseline values 
      (_t_ < 0.1). However, we consistently got _t_ >= 0.1.

 * We may need to add more event handlers to observe what is 
   happening.

 * We may also explore what happens when the message gets modified.

# Links

 * For convenience, the event API of Ryu is described in webpage
   http://ryu.readthedocs.org/en/latest/ryu_app_api.html and 
   http://ryu.readthedocs.org/en/latest/ofproto_ref.html#ofproto-ref.
