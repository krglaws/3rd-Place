#ifndef VALIDATION_H
#define VALIDATION_H

enum validation_result
{
  VALRES_OK, // valid
  VALRES_TOO_LONG, // string too long
  VALRES_TOO_SHORT, // string too short
  VALRES_INV_CHAR, // invalid character
  VALRES_INV_ENC, // invalid encoding
  VALRES_UNMET // unmet requirements
};


/* Name strings can only contain alphanumeric characters
 * or the '_' character. Input strings will not be decoded
 * since valid names should not be encoded to begin with.
 *
 * If an input name contains an invalid character, the backend
 * should not pass the invalid name back to the frontend.
 *
 * These two functions return: VALRES_OK, VALRES_TOO_LONG,
 * VALRES_TOO_SHORT, and VALRES_INV_CHAR.
 */
enum validation_result validate_user_name(const char* user_name);
enum validation_result validate_community_name(const char* community_name);


/* The function that calls these functions will need
 * to allocate a separate buffer since the source string
 * will be decoded from URL encoding and then re-encoded into
 * HTML escaped strings (& -> &amp; for example).
 *
 * These return: VALRES_OK, VALRES_TOO_LONG, VALRES_TOO_SHORT,
 * and VALRES_INV_ENC.
 */
enum validation_result validate_user_about(char* dest, const char* src);
enum validation_result validate_community_about(char* dest, const char* src);
enum validation_result validate_comment_body(char* dest, const char* src);
enum validation_result validate_post_body(char* dest, const char* src);
enum validation_result validate_post_title(char* dest, const char* src);

/* This function will only decode from URL encoding, and will not
 * reencode into HTML escaped chars (since the password should obviously
 * never be displayed to the browser).
 *
 * returns: VALRES_OK, VALRES_TOO_SHORT, VALRES_TOO_LONG, VALRES_INV_ENC,
 * and VALRES_UNMET.
 */
enum validation_result validate_password(char* dest, const char* src);

#endif
