
#ifndef _SQL_WRAPPER_H_
#define _SQL_WRAPPER_H_

/* user sql field names */
#define SF_USER_ID       (0)
#define SF_USER_NAME     (1)
#define SF_USER_ABOUT    (2)
#define SF_USER_POINTS   (3)
#define SF_USER_COMMENTS (4)
#define SF_USER_POSTS    (5)
#define SF_USER_BDAY     (6)

/* post sql field names */
#define SF_POST_ID           (0)
#define SF_POST_COMMUNITY_ID (1)
#define SF_POST_COMMUNITY_NAME (2)
#define SF_POST_AUTHOR_ID    (3)
#define SF_POST_AUTHOR_NAME  (4)
#define SF_POST_TITLE        (5)
#define SF_POST_BODY         (6)
#define SF_POST_BDAY         (7)

/* comment sql field names */
#define SF_COMMENT_ID             (0)
#define SF_COMMENT_POST_ID        (1)
#define SF_COMMENT_POST_TITLE     (2)
#define SF_COMMENT_COMMUNITY_NAME (3)
#define SF_COMMENT_AUTHOR_ID      (4)
#define SF_COMMENT_AUTHOR_NAME    (5)
#define SF_COMMENT_BODY           (6)
#define SF_COMMENT_BDAY           (7)

/* community sql field names */
#define SF_COMMUNITY_ID    (0)
#define SF_COMMUNITY_NAME  (1)
#define SF_COMMUNITY_ABOUT (2)
#define SF_COMMUNITY_BDAY  (3)

/* subscription field names */
#define SF_SUB_ID           (0)
#define SF_SUB_USER_ID      (1)
#define SF_SUB_COMMUNITY_ID (2)

list** query_database_ls(char* query);

#endif

