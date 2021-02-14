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


/* Template Keys: */

/* Main HTML keys */
#define TEMPLATE_PATH_KEY "TEMPLATE_PATH"
#define PAGE_CONTENT_KEY "PAGE_CONTENT"
#define STYLE_PATH_KEY "STYLE_PATH"
#define SCRIPT_PATH_KEY "SCRIPT_PATH"
#define CLIENT_LINK_KEY "CLIENT_LINK"
#define CLIENT_NAME_KEY "CLIENT_NAME"

/* Login keys: */
#define SIGNUP_ERROR_KEY "SIGNUP_ERROR"
#define LOGIN_ERROR_KEY "LOGIN_ERROR"

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
#define USER_POST_LIST_KEY "USER_POST_LIST"
#define USER_COMMENT_LIST_KEY "USER_COMMENT_LIST"

/* Community HTML keys */
#define COMMUNITY_POST_LIST_KEY "COMMUNITY_POST_LIST"

/* Post HTML keys */
#define POST_COMMENT_LIST_KEY "POST_COMMENT_LIST"

/* Feed HTML keys */
#define FEED_ITEM_LIST_KEY "FEED_ITEM_LIST"
#define FEED_TITLE_KEY "FEED_TITLE"

char* build_template(const ks_hashmap* hm);

#endif
