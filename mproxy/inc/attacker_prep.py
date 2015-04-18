#!/usr/bin/python3

"""
profile_parser.py

Parse the attack profile CSV into C source code.
"""

import sys
import os
import argparse
import shutil
import csv
from urllib.parse import parse_qs

NUM_OF_COLUMNS = 8

OFP_V1_0_MSG_ALIAS = [
						'Hello', 'Error', 'EchoReq', 'EchoRes', 'Vendor',
						'FeatureReq', 'FeatureRes', 'GetConfigReq', 'GetConfigRes', 'SetConfig',
						'PacketIn', 'FlowRemoved', 'PortStatus', 'PacketOut', 'FlowMod', 
						'PortMod', 'StatsReq', 'StatsRes', 'BarrierReq', 'BarrierRes', 
						'QueueGetConfigReq', 'QueueGetConfigRes'
					]

OFP_V1_1_MSG_ALIAS = [
						'Hello', 'Error', 'EchoReq', 'EchoRes', 'Experimenter',
						'FeatureReq', 'FeatureRes', 'GetConfigReq', 'GetConfigRes', 'SetConfig',
						'PacketIn', 'FlowRemoved', 'PortStatus', 'PacketOut', 'FlowMod', 
						'GroupMod', 'PortMod', 'TableMod', 'StatsReq', 'StatsRes', 
						'BarrierReq', 'BarrierRes', 'QueueGetConfigReq', 'QueueGetConfigRes'
					]

OFP_V1_2_MSG_ALIAS = [
						'Hello', 'Error', 'EchoReq', 'EchoRes', 'Experimenter',
						'FeatureReq', 'FeatureRes', 'GetConfigReq', 'GetConfigRes', 'SetConfig',
						'PacketIn', 'FlowRemoved', 'PortStatus', 'PacketOut', 'FlowMod', 
						'GroupMod', 'PortMod', 'TableMod', 'StatsReq', 'StatsRes', 
						'BarrierReq', 'BarrierRes', 'QueueGetConfigReq', 'QueueGetConfigRes', 'RoleReq',
						'RoleRes'
					]

OFP_V1_3_MSG_ALIAS = [
						'Hello', 'Error', 'EchoReq', 'EchoRes', 'Experimenter',
						'FeatureReq', 'FeatureRes', 'GetConfigReq', 'GetConfigRes', 'SetConfig',
						'PacketIn', 'FlowRemoved', 'PortStatus', 'PacketOut', 'FlowMod', 
						'GroupMod', 'PortMod', 'TableMod', 'MultipartReq', 'MultipartRes'
						'BarrierReq', 'BarrierRes', 'QueueGetConfigReq', 'QueueGetConfigRes', 'RoleReq',
						'RoleRes', 'GetAsyncReq', 'GetAsyncRes', 'SetAsync', 'MeterMod'
					]

OFP_V1_4_MSG_ALIAS = [
						'Hello', 'Error', 'EchoReq', 'EchoRes', 'Experimenter',
						'FeatureReq', 'FeatureRes', 'GetConfigReq', 'GetConfigRes', 'SetConfig',
						'PacketIn', 'FlowRemoved', 'PortStatus', 'PacketOut', 'FlowMod', 
						'GroupMod', 'PortMod', 'TableMod', 'MultipartReq', 'MultipartRes'
						'BarrierReq', 'BarrierRes', None, None, 'RoleReq',
						'RoleRes', 'GetAsyncReq', 'GetAsyncRes', 'SetAsync', 'MeterMod',
						'RoleStatus', 'TableStatus', 'RequestForward', 'BundleControl', 'BundleAddMessage'
					]

OFP_COMMON_MSG_ALIAS = [
						'Hello', 'Error', 'EchoReq', 'EchoRes', None,
						'FeatureReq', 'FeatureRes', 'GetConfigReq', 'GetConfigRes', 'SetConfig',
						'PacketIn', 'FlowRemoved', 'PortStatus', 'PacketOut', 'FlowMod',
						None, None, None, None, None, 
						None, None, None, None, 'RoleReq', 
						'RoleRes', 'GetAsyncReq', 'GetAsyncRes', 'SetAsync', 'MeterMod',
						'RoleStatus', 'TableStatus', 'RequestForward', 'BundleControl', 'BundleAddMessage'
					]

OFP_MSG_TYPE_ALIAS_DICT = {
	0: OFP_COMMON_MSG_ALIAS,
	1: OFP_V1_0_MSG_ALIAS,
	2: OFP_V1_1_MSG_ALIAS,
	3: OFP_V1_2_MSG_ALIAS,
	4: OFP_V1_3_MSG_ALIAS,
	5: OFP_V1_4_MSG_ALIAS,
}

# find the largest size of type sets
MAX_TYPE_VALUE = 0
for k in OFP_MSG_TYPE_ALIAS_DICT:
	if len(OFP_MSG_TYPE_ALIAS_DICT[k]) > MAX_TYPE_VALUE:
		MAX_TYPE_VALUE = len(OFP_MSG_TYPE_ALIAS_DICT[k])

DROP_ACTION_TABLE = [0] * len(OFP_MSG_TYPE_ALIAS_DICT) * MAX_TYPE_VALUE

class ParserValueError(Exception):
	pass

class ParserFormatError(Exception):
	pass

class decor:
	NODECOR = '\033[0m'
	RED = '\033[91m'
	GREEN = '\033[92m'
	YELLOW = '\033[93m'
	CYAN = '\033[96m'

def ofp_normalize_version(val):
	"""Convert the version string to its normalized integer value."""
	val = str(val)
	if '.' in val:
		if val == '1.0':
			return 1
		elif val == '1.1':
			return 2
		elif val == '1.2':
			return 3
		elif val.startswith('1.3'):
			return 4
		elif val == '1.4':
			return 5
	try:
		val = int(val)
		if val not in OFP_MSG_TYPE_ALIAS_DICT:
			raise ParserValueError('unsupported OFP version value: %d' % val)
		return val
	except ValueError:
		if val == '*':
			return 0
		raise ParserValueError('unrecognized OFP version string: "{}"'.format(val))

def parse_row(row, line):
	"""Parse the row array to a dict of correct types.
	Will print as many errors as possible and return None if row is misformed."""
	# make sure exact number of columns
	has_error = False
	if len(row) != NUM_OF_COLUMNS:
		print(decor.RED + 'Line {0}: incorrect number of columns. Got {1}. Expect {2}.'.format(line, len(row), NUM_OF_COLUMNS) + decor.NODECOR)
		return None
	# parse length field (check syntax but not used though)
	try:
		act_len = int(row[0])
	except:
		act_len = -1
		if row[0] != '*':
			print(decor.YELLOW + 'Line {0}: incorrect length specifier "{1}". Expect an int value.'.format(line, row[0]) + decor.NODECOR)
			has_error = True
	# parse controller ID field (check syntax but not used though)
	try:
		act_cid = int(row[1])
	except:
		act_cid = -1
		if row[1] != '*':
			print(decor.YELLOW + 'Line {0}: incorrect CID specifier "{1}". Expect an int value.'.format(line, row[1]) + decor.NODECOR)
			has_error = True
	# parse switch ID field (check syntax but not used though)
	try:
		act_sid = int(row[2])
	except:
		act_sid = -1
		if row[2] != '*':
			print(decor.YELLOW + 'Line {0}: incorrect SID specifier "{1}". Expect an int value.'.format(line, row[2]) + decor.NODECOR)
			has_error = True
	# parse OFP version string and message type
	try:
		act_ver = ofp_normalize_version(row[3])
		act_type_alias = OFP_MSG_TYPE_ALIAS_DICT[act_ver]
		if row[4].isdigit():
			# assume int field
			act_type_int = int(row[4])
			if act_type_int < 0 or act_type_int >= len(act_type_alias) or act_type_alias[act_type_int] is None:
				act_type_int = 0
				print(decor.YELLOW + 'Line {0}: unsupported message type ({1}) for the specified OFP version.'.format(line, act_type_int) + decor.NODECOR)
		else:
			# the field is string
			if row[4] not in act_type_alias:
				if row[4] != '*':
					act_type_int = -1
					has_error = True
					print(decor.YELLOW + 'Line {0}: unsupported message type "{1}" for the specified OFP version.'.format(line, row[4]) + decor.NODECOR)
				else:
					act_type_int = -1
			else:
				act_type_int = act_type_alias.index(row[4])
	except ParserValueError as e:
		print(decor.YELLOW + 'Line {0}: {1}.'.format(line, str(e)) + decor.NODECOR)
		act_ver = -1
		act_type_int = 0
		has_error = True
	# TODO: parse attack field
	# parse attack type and args
	act_attack_type = row[6].upper()
	if act_attack_type == 'DROP':
		# supported arg: p
		act_attack_args = parse_qs(row[7])
		if 'p' not in act_attack_args:
			print(decor.YELLOW + 'Line {0}: DROP action requires a probability param p.'.format(line) + decor.NODECOR)
			has_error = True
		else:
			p = act_attack_args['p'][0]
			try:
				p = float(p)
				if p < 0.0 or p > 1.0:
					raise ValueError()
				act_attack_args['p'] = p
			except ValueError:
				print(decor.YELLOW + 'Line {0}: probability param p must be a float number in range [0, 1]. Got "{1}".'.format(line, p) + decor.NODECOR)
	elif act_attack_type == 'DELAY':
		act_attack_args = {}
	elif act_attack_type == 'DUP':
		act_attack_args = {}
	else:
		print(decor.YELLOW + 'Line {0}: unsupported malicious action "{1}".'.format(line, row[6]) + decor.NODECOR)
		has_error = True

	if has_error:
		return None
	return {
			'length': act_len,
			'target_cid': act_cid,
			'target_sid': act_sid,
			'target_ver': act_ver,
			'target_type': act_type_int,
			'target_field': None,
			'attack_type': act_attack_type,
			'attack_args': act_attack_args
		}


def parse_profile(f):
	# CSV formatter
	csv.register_dialect('Profile', skipinitialspace=True)
	# start parse CSV
	reader = csv.reader(f, dialect='Profile')
	# skip the header row
	next(reader, None)
	# iterate over the attack action rows
	for row in reader:
		action = parse_row(row, reader.line_num)
		if action is None:
			continue
		if action['attack_type'] == 'DROP':
			if action['target_type'] == -1:
				for i in range(0, MAX_TYPE_VALUE):
					DROP_ACTION_TABLE[action['target_ver'] * MAX_TYPE_VALUE + i] = action['attack_args']['p']
			else:
				DROP_ACTION_TABLE[action['target_ver'] * MAX_TYPE_VALUE + action['target_type']] = action['attack_args']['p']
		print(decor.GREEN + 'Accepted action at line %d.' % reader.line_num + decor.NODECOR)
		print(action)

def main():
	parser = argparse.ArgumentParser(description='Attack profile C code generator.')
	parser.add_argument('file', nargs=1, help='path to attack CSV file')
	args = parser.parse_args()

	try:
		if os.path.isfile('./inc'):
			os.remove('./inc')
		elif os.path.isdir('./inc'):
			shutil.rmtree('./inc')
		if os.path.isfile('./src'):
			os.remove('./src')
		elif os.path.isdir('./src'):
			shutil.rmtree('./src')
		os.makedirs('./inc/attacker')
		os.makedirs('./src')
	except OSError as e:
		print(decor.RED + str(e) + decor.NODECOR)
		sys.exit(1)

	try:
		csv_file = open(args.file[0], 'r')
	except OSError as e:
		print(decor.RED + str(e) + decor.NODECOR)
		sys.exit(1)

	parse_profile(csv_file)
	csv_file.close()

	with open('./inc/attacker/attacker.h', 'w') as f:
		f.write("""/**
 * attacker.h
 * Defines prototypes for attack actions.
 */

/**
 * An in-line function that returns true if a message should be dropped, and false otherwise.
 * If the message should be dropped, the float variable s will store the probability of doing the dropping.
 * @param float s Variable name that stores the probability of dropping the message.
 * @param char v Version value of the message.
 * @param char z Message type value of the message.
 */
#define should_drop_msg(s, v, z)	((s = _drop_action_table[v][z]) || (s = _drop_action_table[0][z]))

// a global table to query which message to drop.
// if a message has version number 3 and type value 4, then drop the message
// if drop_action_table[3][4] > 0 or drop_action_table[0][4] > 0.
extern float _drop_action_table[{0}][{1}];
""".format(len(OFP_MSG_TYPE_ALIAS_DICT), MAX_TYPE_VALUE))

	with open('./src/attacker.c', 'w') as f:
		drop_prob_string = [str(v) + ', ' for v in DROP_ACTION_TABLE]
		for i in range(1, len(OFP_MSG_TYPE_ALIAS_DICT)):
			drop_prob_string.insert(i * MAX_TYPE_VALUE, '\n')
		drop_prob_string = ''.join(drop_prob_string)
		f.write("""#include <attacker/attacker.h>

float _drop_action_table[{0}][{1}] = {{{2}}};
""".format(
	len(OFP_MSG_TYPE_ALIAS_DICT), 
	MAX_TYPE_VALUE, 
	drop_prob_string))

if __name__ == '__main__':
	main()
