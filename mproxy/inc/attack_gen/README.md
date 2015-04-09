Attack Generator
================

This is a tool that transforms the attack specifier CSV to C code.

# CSV format

Each line specifies an attack action, and should have six columns:

 * `LEN`: length of the line. `int` value.
 * `CID`: id of the controller to which the attack is applicable. `int` value or '*' (of value `-1`) meaning applicable to all.
 * `SID`: id of the switch to which the attack is applicable. `int` value or '*' (of value `-1`) meaning applicable to all.
 * `TARGET`: on what field of the data to apply the attack. `string` value of format `MSG_TYPE(.FIELD_NAME)*`. Details are discussed later.
 * `ATTACK_TYPE`: type of the attack. `uint8_t` value or its `string` alias. Supported values are listed later.
 * `ARGS`: arguments for the attack. `string` of format `field=val(,field=val)*`. Supported fields for each attack type are listed later.

# Attack Types


