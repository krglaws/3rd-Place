#ifndef VALIDATOR_H
#define VALIDATOR_H

/* If any of the functions in this header return false,
 * it means that the string input contained an invalid
 * character or character encoding, did not meet the character
 * requirement (only the case for passwords), or was either too
 * long or too short.
 */

/* These functions just check for valid characters,
 * character requirements, and lengths.
 */
bool valid_user_name(const char* user_name);
bool valid_community_name(const char* community_name);


/* The function that calls these functions will need
 * to allocate a separate buffer since the source string
 * will be decoded from URL encoding and then re-encoded into
 * HTML escaped strings (& -> &amp; for example).
 */
bool valid_passwd(char* dest, const char* src);
bool valid_user_about(char* dest, const char* src);
bool valid_community_about(char* dest, const char* src);
bool valid_comment_body(char* dest, const char* src);
bool valid_post_body(char* dest, const char* src);
bool valid_post_title(char* dest, const char* src);

#endif
