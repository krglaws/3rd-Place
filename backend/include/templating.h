#ifndef TEMPLATING_H
#define TEMPLATING_H

#include <kylestructs.h>

/* This header defines the build_template() function, which builds
 * html pages from the data contained within a hashmap. It also defines
 * several string constants that map strings found in templates to data
 * within the hashmap.
 */


/* These constants are used for indicating
 * the start and end of template strings.
 */
#define TEMPLATE_BEGIN "<<"
#define TEMPLATE_END ">>"

/* Keys with this prefix are considered optional,
 * and if no value is found for such keys, they
 * will be replaced with "", and no error will
 * be logged.
 *
 * If a key does not have this prefix, and no value
 * is found for it, it will still be replaced with "",
 * but an error WILL be logged.
 */
#define OPTIONAL_PREFIX "OPT_"


/* Template Keys: */

/* Main HTML keys */
#define TEMPLATE_PATH_KEY "TEMPLATE_PATH"
#define PAGE_CONTENT_KEY "PAGE_CONTENT"
#define STYLE_PATH_KEY "STYLE_PATH"
#define SCRIPT_PATH_KEY (OPTIONAL_PREFIX "SCRIPT_PATH")
#define CLIENT_LINK_KEY "CLIENT_LINK"
#define CLIENT_NAME_KEY "CLIENT_NAME"

/* option visibility keys */
#define NEW_OPTION_VISIBILITY_KEY "NEW_OPTION_VISIBILITY"
#define EDIT_OPTION_VISIBILITY_KEY "EDIT_OPTION_VISIBILITY"
#define DELETE_OPTION_VISIBILITY_KEY "DELETE_OPTION_VISIBILITY"
#define LOGOUT_OPTION_VISIBILITY_KEY "LOGOUT_OPTION_VISIBILITY"
#define SUBSCRIPTIONS_OPTION_VISIBILITY_KEY "SUBSCRIPTIONS_OPTION_VISIBILITY"

/* User form keys: */
#define SIGNUP_ERR_KEY (OPTIONAL_PREFIX "SIGNUP_ERROR")
#define LOGIN_ERR_KEY (OPTIONAL_PREFIX "LOGIN_ERROR")
#define EDIT_USER_PASSWD_ERR_KEY (OPTIONAL_PREFIX "EDIT_USER_PASSWD_ERR")
#define EDIT_USER_ABOUT_ERR_KEY (OPTIONAL_PREFIX "EDIT_USER_ABOUT_ERR")
#define SUBMITTED_LOGIN_UNAME_KEY (OPTIONAL_PREFIX "SUBMITTED_LOGIN_UNAME")
#define SUBMITTED_SIGNUP_UNAME_KEY (OPTIONAL_PREFIX "SUBMITTED_SIGNUP_UNAME")
#define SUBMITTED_USER_ABOUT_KEY (OPTIONAL_PREFIX "SUBMITTED_USER_ABOUT")

/* Post form keys */
#define POST_FORM_ERR_KEY (OPTIONAL_PREFIX "POST_FORM_ERR")
#define EDIT_POST_FORM_ERR_KEY (OPTIONAL_PREFIX "EDIT_POST_FORM_ERR")
#define SUBMITTED_POST_TITLE_KEY (OPTIONAL_PREFIX "SUBMITTED_POST_TITLE")
#define SUBMITTED_POST_BODY_KEY (OPTIONAL_PREFIX "SUBMITTED_POST_BODY")

/* Comment form keys */
#define COMMENT_FORM_ERR_KEY (OPTIONAL_PREFIX "COMMENT_FORM_ERR")
#define EDIT_COMMENT_FORM_ERR_KEY (OPTIONAL_PREFIX "EDIT_COMMENT_FORM_ERR")
#define SUBMITTED_COMMENT_BODY_KEY (OPTIONAL_PREFIX "SUBMITTED_COMMENT_BODY")

/* Community form keys */
#define COMMUNITY_FORM_ERR_KEY (OPTIONAL_PREFIX "COMMUNITY_FORM_ERR")
#define NEW_MOD_FORM_ERR_KEY (OPTIONAL_PREFIX "NEW_MOD_FORM_ERR")
#define EDIT_COMMUNITY_FORM_ERR_KEY (OPTIONAL_PREFIX "EDIT_COMMUNITY_FORM_ERR")
#define SUBMITTED_COMMUNITY_NAME_KEY (OPTIONAL_PREFIX "SUBMITTED_COMMUNITY_NAME")
#define SUBMITTED_COMMUNITY_ABOUT_KEY (OPTIONAL_PREFIX "SUBMITTED_COMMUNITY_ABOUT")

/* CSS vote class keys */
#define UPVOTE_CLICKED_KEY "UPVOTE_CLICKED"
#define DOWNVOTE_CLICKED_KEY "DOWNVOTE_CLICKED"

/* Vote wrapper types (POST_TYPE or COMMENT_TYPE) */
#define POST_TYPE "POST_TYPE"
#define COMMENT_TYPE "COMMENT_TYPE"

/* post vote wrapper keys */
#define POST_ID_KEY "POST_ID"
#define POST_POINTS_KEY "POST_POINTS"
#define POST_KEY "POST"

/* comment vote wrapper keys */
#define COMMENT_ID_KEY "COMMENT_ID"
#define COMMENT_POINTS_KEY "COMMENT_POINTS"
#define COMMENT_KEY "COMMENT"

/* User HTML keys */
#define USER_POST_LIST_KEY (OPTIONAL_PREFIX "USER_POST_LIST")
#define USER_COMMENT_LIST_KEY (OPTIONAL_PREFIX "USER_COMMENT_LIST")

/* Community HTML keys */
#define COMMUNITY_POST_LIST_KEY (OPTIONAL_PREFIX "COMMUNITY_POST_LIST")
#define COMMUNITY_SUBSCRIPTION_STATUS_KEY "COMMUNITY_SUBSCRIPTION_STATUS"

/* Post HTML keys */
#define POST_COMMENT_LIST_KEY (OPTIONAL_PREFIX "POST_COMMENT_LIST")

/* Feed HTML keys */
#define FEED_ITEM_LIST_KEY "FEED_ITEM_LIST"
#define FEED_TITLE_KEY "FEED_TITLE"

char* build_template(const ks_hashmap* hm);

#endif
