#!/usr/bin/python

"""
demo_mininet_topo.py

Sample topology class with Mininet.

G = {V, E}
V = {h1, h2, h3, h4, h51, h52, s0, s1, s4, s5}
	# of hosts = 6
	# of switches = 4
E = {
		(h1, s1), (h2, s1), (h3, s1), 
	 	(h4, s4), 
		(h51, s5), (h52, s5), 
		(s0, s1), (s0, s4), (s5, s4)
	}
"""

from mininet.topo import Topo

class DemoTopology(Topo):
	
	def __init__(self):
		
		Topo.__init__(self)
		
		# Add some hosts
		h1 = self.h1 = self.addHost('h1')
		h2 = self.h2 = self.addHost('h2')
		h3 = self.h3 = self.addHost('h3')
		h4 = self.h4 = self.addHost('h4')
		h51 = self.h51 = self.addHost('h51')
		h52 = self.h52 = self.addHost('h52')
		
		# Add switches
		s0 = self.s0 = self.addSwitch('s0')
		s1 = self.s1 = self.addSwitch('s1')
		s4 = self.s4 = self.addSwitch('s4')
		s5 = self.s5 = self.addSwitch('s5')
		
		# Link hosts with switches
		self.addLink(h1, s1)
		self.addLink(h2, s1)
		self.addLink(h3, s1)
		self.addLink(h4, s4)
		self.addLink(h51, s5)
		self.addLink(h52, s5)
		
		# Link switches with switches
		self.addLink(s0, s1)
		self.addLink(s0, s4)
		self.addLink(s5, s4)
	
topos = {
	'demo': lambda: DemoTopology()
}	