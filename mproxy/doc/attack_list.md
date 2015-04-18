Attack List
===========

TODO: eliminate the use of floating-point numbers.

# CSV Format

Each non-comment line specifies an attack action, and should have six columns:

 * `LEN`: length of the line. `int` value.
 * `CID`: id of the controller to which the attack is applicable. `int` value or '*' (of value `-1`) meaning applicable to all.
 * `SID`: id of the switch to which the attack is applicable. `int` value or '*' (of value `-1`) meaning applicable to all.
 * `OFP_VER`: version of OpenFlow protocol.
 * `MSG_TYPE`: on what type of message to apply the attack. Either an `int` of OFP message type or a `string` alias of the type defined later. Wildcard `*` has value `0`.
 * `FIELD`: on what field of message to apply the attack. A `string` of format `F1(.Fi)*`. For example, `data.in_addr`.
 * `ATTACK_TYPE`: type of the attack. `uint8_t` value or its `string` alias. Supported values are listed later.
 * `ARGS`: arguments for the attack. `string` of format `field=val(;field=val)*`. Supported fields for each attack type are listed later.

A line starting with `#` is a comment line and will be ignored.

## Attack Types and Arguments

The following table lists the types of attacks and their supported arguments, where 

 * `<PROB>`: a placeholder for an integer `x` whose value is in range [0, 100].
 * `<INT>`: a placeholder for an integer like `123`.
 * `<INT[a,b]>`: a placeholder for an integer `x` such that `a<=x<=b`. The range after token `INT` conforms to its mathematical meaning. Other range specifiers include `(a,b)`, `(a,b]`, `[a, b)`. For example, `<INT[0,100]>` is equivalent to `<PROB>`.
 * `<DECIMAL>`: a placeholder for a decimal number like `0.1`.
 * `<DECIMAL[a,b]>`: a placeholder for a decimal number in the specified range.

| ID | Name   | Alias     | Description                                             | Arguments            | Status  |
| :--: | ------ | --------- | ------------------------------------------------------- | -------------------- | ------- |
| 00 | Drop   | `DROP`    | Drop the message with a specified probability `p` (default value: `100`).      | `p=<PROB>`           | Working |
| 01 | Delay  | `DELAY`   | Delay the message delivery by `t` milliseconds.         | `t=<INT>`            | Planned |
| 02 | Duplicate | `DUP`  | Repeatedly send the message `r` times.                  | `r=<INT>`            | Planned |
| 03 | Modification | `LIE` | Modify the target field.                              | TBD.                 | Planned |

Notes:
 * Could add conditional actions like Dropping the message if a field `f` has value `v` (with alias like `DROPC`).
 * Use ampersand (`&`) to separate arguments (to form a query string format).

## OFP Version

`OFP_VER` column dictates the version of OFP message. It is necessary since same `MSG_TYPE` integer can represent different types in different versions of OFP.

## Message Types and Fields

A quick summary of OpenFlow message types can be found [here](http://flowgrammable.org/sdn/openflow/message-layer).
Besides, wildcard char `*` can be used to match messages of any type.

If the column `OFP_VER` has value `*`, then only type aliases that are the same across all OFP versions are supported.

The column `FIELD` specifies where to apply an attack. The format is `F1(.Fi)*`. Not all attack types require `FIELD`. For example, if the attack is to modify a field, then the specified field will be modified according to the rule. For the attack types that do not require `FIELD`, `FIELD` will be ignored.

# Example

TBA.

