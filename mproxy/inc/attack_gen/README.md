Attack Profile
==============

This is a tool that transforms the attack specifier CSV to C code.

# CSV Format

Each line specifies an attack action, and should have six columns:

 * `LEN`: length of the line. `int` value.
 * `CID`: id of the controller to which the attack is applicable. `int` value or '*' (of value `-1`) meaning applicable to all.
 * `SID`: id of the switch to which the attack is applicable. `int` value or '*' (of value `-1`) meaning applicable to all.
 * `TARGET`: on what field of the data to apply the attack. `string` value of format `MSG_TYPE(\.FIELD_NAME)*` or `*` meaning "anything". Details are discussed later.
 * `ATTACK_TYPE`: type of the attack. `uint8_t` value or its `string` alias. Supported values are listed later.
 * `ARGS`: arguments for the attack. `string` of format `field=val(;field=val)*`. Supported fields for each attack type are listed later.

Notes:
 * Should we add a `priority` column?

## Attack Types and Arguments

The following table lists the types of attacks and their supported arguments, where 

 * `<PROB>`: a placeholder for a decimal number `x` whose value is in range [0, 1].
 * `<INT>`: a placeholder for an integer like `123`.
 * `<INT[a,b]>`: a placeholder for an integer `x` such that `a<=x<=b`. The range after token `INT` conforms to its mathematical meaning. Other range specifiers include `(a,b)`, `(a,b]`, `[a, b)`. For example, `<INT[1,3]>`.
 * `<DECIMAL>`: a placeholder for a decimal number like `0.1`.
 * `<DECIMAL[a,b]>`: a placeholder for a decimal number in the specified range. For example, `<DECIMAL[0.0, 1.0]>` is equivalent to `<PROB>`.

| ID | Name   | Alias     | Description                                             | Arguments            | Status  |
| :--: | ------ | --------- | ------------------------------------------------------- | -------------------- | ------- |
| 00 | Drop   | `DROP`    | Drop the message with a specified probability `p`.      | `p=<PROB>`           | Planned |
| 01 | Delay  | `DELAY`   | Delay the message delivery by `t` milliseconds.         | `t=<INT>`            | Planned |
| 02 | Duplicate | `DUP`  | Repeatedly send the message `r` times.                  | `r=<INT>`            | Planned |
| 04 | Modification | `LIE` | TBD.                                                  | TBD.                 | Planned |

Notes:
 * Could add conditional actions like Dropping the message if a field `f` has value `v` (with alias like `DROPC`).

## Target

The column `TARGET` specifies the scope for the attack. The format is `MSG_TYPE(\.FIELD_NAME)*`, or wildcard char `*` meaning that the attack is applicable to all messages. More specifically, if `TARGET` is not `*`, then the first part of `TARGET` string is the type of message, followed by the field specifiers in the message data structure. For example, a value of `Hello` means the attack is applicable to all messages of type `Hello`, while `PacketIn.in_port` means the attack will target at the `in_port` field of messages of type `PacketIn`. Type value can be used in lieu of type alias. For example, type `Hello` has type value `0`. A quick summary of OpenFlow message types can be found [here](http://flowgrammable.org/sdn/openflow/message-layer).

Not all actions require a data member field. For example, `DROP` action only requires message type and all following fields will be ignored.

# Example

TBA.

