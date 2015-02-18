# CS497 Honor Research Project

## Introduction

The past few years witnessed the increasing popularity of software-defined networking (SDN). While today many software developers are working on new SDN controllers and and switches, it seems there are few achievements regarding testing those programs in a comprehensive, convenient, and probably "universal" manner, at least in open source community.

Inspired by Turret[1], a platform to automatedly test distributed systems as applications, we want to explore how to test SDN controllers and switches in automated ways.

### Status Quo

There are commercial tools to test and stress test OpenFlow controllers[2].
The tests showned in its document are bundled with NEC hardware.



### Tools

[Mininet](http://mininet.org) is a light-weight and easy-to-use network emulator that (1) has built-in support for various SDN controllers and switches, and (2) provides standard classes and interfaces to support more SDN components. We may use it to construct emulated network environment and inject tests.

[Ryu](http://osrg.github.io/ryu/) is an actively-developed (whereas [Nox](http://noxrepo.org/) and [Pox](http://www.noxrepo.org/pox/about-pox/) are mostly in stagnation) SDN framework that supports a wide range of SDN protocols (as for OpenFlow, it supports 1.0+) and has built-in support from Mininet. With its well-designed API we may have easier access to the protocols. Besides, we may also be interested in testing Ryu as a SDN controller.

[Docker](http://docker.io). (note: my intuitions tell me I may need it. Not sure. Or use `qemu-kvm` for full virtualization.) 

## Stages

### Planning

 * Get familiar with Mininet standard interfaces
 * Abstract the controllers (in terms of ?) with Ryu as an example
 * Think about how to test the given SDN controller in Mininet
   * in what environment (context)
   * for which functionality
   * how to automate
 * Generalize the findings to a tool

### Sprints

TBD.

## Conclusion

TBD.

## People

[Xiangyu Bu](http://xybu.me)

[Cristina Nita-Rotaru](http://homes.cerias.purdue.edu/~crisn/index.html) (supervisor)

## Reference

 * [1] Hyojeong Lee; Seibert, J.; Hoque, E.; Killian, C.; Nita-Rotaru, C., "Turret: A Platform for Automated Attack Finding in Unmodified Distributed System Implementations," Distributed Computing Systems (ICDCS), 2014 IEEE 34th International Conference on , vol., no., pp.660,669, June 30 2014-July 3 2014. [doi: 10.1109/ICDCS.2014.73](http://ieeexplore.ieee.org/xpl/articleDetails.jsp?arnumber=6888941)
 
 * [2] [SDN Controller Testing: Part 1](http://www.necam.com/docs/?id=2709888a-ecfd-4157-8849-1d18144a6dda), white paper from Ixia.
