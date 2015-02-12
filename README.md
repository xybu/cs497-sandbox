# CS497 Honor Research Project

## Introduction

The past few years witnessed the increasing popularity of software-defined networking (SDN). While today many software developers are working on new SDN controllers and and switches, it seems there are few achievements regarding testing those programs in a comprehensive and convenient manner.

Inspired by [Turret, a platform to automatedly test distributed systems as applications](http://ieeexplore.ieee.org/xpl/articleDetails.jsp?arnumber=6888941), we want to explore how to test SDN controllers and switches in automated ways.

### Tools

[Mininet](http://mininet.org) is a light-weight and easy-to-use network emulator that (1) has built-in support for various SDN controllers and switches, and (2) provides standard classes and interfaces to support more SDN components. We may use it to construct emulated network environment and inject tests.

[Ryu](http://osrg.github.io/ryu/) is an actively-developed (whereas [Nox](http://noxrepo.org/) and [Pox](http://www.noxrepo.org/pox/about-pox/) are mostly in stagnation) SDN framework that supports a wide range of SDN protocols (as for OpenFlow, it supports 1.0+) and has built-in support from Mininet. With its well-designed API we may have easier access to the protocols. Besides, we may also be interested in testing Ryu as a SDN controller.

[Docker](http://docker.io). (note: my intuitions tell me I may need it. Not sure.) 

## Stages

### Planning

 * Get familiar with Mininet interfaces
 * Abstract the controllers (in terms of ?) with Ryu as example
 * Think about how to test the given SDN controller in Mininet
   * in what environment
   * for which functionality
   * how to automate
 * Generalize the findings to a tool

## Conclusion

## People

[Xiangyu Bu](http://xybu.me)

[Cristina Nita-Rotaru](http://homes.cerias.purdue.edu/~crisn/index.html) (supervisor)

