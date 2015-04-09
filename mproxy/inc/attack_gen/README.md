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
 * `ARGS`: arguments for the attack. `string` of format `field=val(;field=val)*`. Supported fields for each attack type are listed later.

# Attack Types

The following table lists the types of attacks and their supported arguments, where 

 * `<PROB>`: a placeholder for a decimal number `x` whose value is in range [0, 1].
 * `<INT>`: a placeholder for an integer like `123`.
 * `<INT[a,b]>`: a placeholder for an integer `x` such that `a<=x<=b`. The range after token `INT` conforms to its mathematical meaning. Other range specifiers include `(a,b)`, `(a,b]`, `[a, b)`. For example, `<INT[1,3]>`.
 * `<DECIMAL>`: a placeholder for a decimal number like `0.1`.
 * `<DECIMAL[a,b]>`: a placeholder for a decimal number in the specified range. For example, `<DECIMAL[0.0, 1.0]>` is equivalent to `<PROB>`.

| Name   | Alias     | Description                                              | Arguments            | Status  |
| ------ | --------- | -------------------------------------------------------- | -------------------- | ------- |
| Drop   | `DROP`    | Drop the message with a specified probability `p`.       | `p=<PROB>`           | Started |
