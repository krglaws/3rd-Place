#ifndef VALIDATOR_H
#define VALIDATOR_H

enum validation_result
{
  VALRES_OK, // valid
  VALRES_TOO_LONG, // string too long
  VALRES_TOO_SHORT, // string too short
  VALRES_INV_CHAR, // invalid character
  VALRES_INV_ENC, // invalid encoding
  VALRES_UNMET // unmet requirements
};

/* These functions just check for valid characters,
 * character requirements, and lengths.
 */
enum validation_result valid_user_name(const char* user_name);
enum validation_result valid_community_name(const char* community_name);


/* The function that calls these functions will need
 * to allocate a separate buffer since the source string
 * will be decoded from URL encoding and then re-encoded into
 * HTML escaped strings (& -> &amp; for example).
 */
enum validation_result valid_passwd(char* dest, const char* src);
enum validation_result valid_user_about(char* dest, const char* src);
enum validation_result valid_community_about(char* dest, const char* src);
enum validation_result valid_comment_body(char* dest, const char* src);
enum validation_result valid_post_body(char* dest, const char* src);
enum validation_result valid_post_title(char* dest, const char* src);

#endif
