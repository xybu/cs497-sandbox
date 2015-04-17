/**
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
extern float _drop_action_table[6][35];
