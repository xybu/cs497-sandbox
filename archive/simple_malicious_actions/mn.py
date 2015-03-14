#!/usr/bin/python

"""
The goal is to insert a customized controller between the top-level
controller and the switches connected to it.

Mininet Controller - Injected Controller - Hosts and Switches

Reference: 
https://github.com/mininet/mininet/blob/master/examples/controllers2.py
"""

import itertools
from mininet.net import Mininet
from mininet.node import RemoteController, OVSSwitch, Host
from mininet.cli import CLI


class bcolors:
	HEADER = '\033[95m'
	OKBLUE = '\033[94m'
	OKGREEN = '\033[92m'
	WARNING = '\033[93m'
	FAIL = '\033[91m'
	ENDC = '\033[0m'
	BOLD = '\033[1m'
	UNDERLINE = '\033[4m'


def main():
	print(bcolors.OKBLUE + 'Constructing Mininet...' + bcolors.ENDC)
	mn = Mininet(build=False)
	# create target controller
	controllers = []
	print(bcolors.OKBLUE + 'Adding target controller c0...' + bcolors.ENDC)
	# controllers.append(mn.addController(name='c0', controller=Controller, ip='127.0.0.1', port=6633))
	controllers.append(mn.addController(name='c0', controller=RemoteController, port=6633))
	# create the injected controller
	print(bcolors.OKBLUE + 'Adding malicious controller c1...' + bcolors.ENDC)
	controllers.append(mn.addController(name='c1', controller=RemoteController, port=6634))
	# mn.addLink(controllers[0], controllers[1])
	# create hosts
	print(bcolors.OKBLUE + 'Adding hosts...' + bcolors.ENDC)
	hosts = {}
	for i in range(0, 2):
		# create hosts h00, h01, etc.
		hosts[i] = [ mn.addHost('h' + str(i) + str(n), cls=Host, defaultRoute=None) for n in range(0, 2) ]
	# create switches
	print(bcolors.OKBLUE + 'Adding switches...' + bcolors.ENDC)
	switches = []
	for k in hosts:
		s = mn.addSwitch('s%d' % k, cls=OVSSwitch, protocols='OpenFlow13')
		switches.append(s)
		for i in hosts[k]:
			# link hosts in set k with switch k
			print(bcolors.OKBLUE + 'Linking host {0} to switch {1}...'.format(i.name, s.name) + bcolors.ENDC)
			mn.addLink(i, s)
	# interconnect switches with each other
	print(bcolors.OKBLUE + 'Linking switches...' + bcolors.ENDC)
	for t in itertools.combinations(switches, 2):
		a, b = t
		print(bcolors.OKBLUE + 'Linking switch {0} with {1}...'.format(a.name, b.name) + bcolors.ENDC)
		mn.addLink(a, b)
	# build network
	print(bcolors.OKBLUE + 'Building mininet...' + bcolors.ENDC)
	mn.build()
	# start controllers
	print(bcolors.OKBLUE + 'Starting controllers...' + bcolors.ENDC)
	for c in controllers:
		c.start()
	# start switches
	print(bcolors.OKBLUE + 'Starting switches...' + bcolors.ENDC)
	for k in range(0, len(switches)):
		# connect switch k to controller k % (total number of controllers)
		c = controllers[k % len(controllers)]
		s = switches[k]
		print(bcolors.OKBLUE + 'Starting switch {0} on controller {1}...'.format(s.name, c.name) + bcolors.ENDC)
		s.start([c])
		# s.start(controllers)
	# start cli
	print(bcolors.OKBLUE + 'Starting CLI...' + bcolors.ENDC)
	CLI(mn)
	# stop
	print(bcolors.OKBLUE + 'Stopping mininet...' + bcolors.ENDC)
	mn.stop()

if __name__ == '__main__':
	main()
