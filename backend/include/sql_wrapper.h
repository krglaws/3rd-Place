
#ifndef _SQL_WRAPPER_H_
#define _SQL_WRAPPER_H_

/* user sql field names */
#define SQL_FIELD_USER_ID            (0)
#define SQL_FIELD_USER_NAME          (1)
#define SQL_FIELD_USER_PASSWORD_HASH (2)
#define SQL_FIELD_USER_ABOUT         (3)
#define SQL_FIELD_USER_POINTS        (4)
#define SQL_FIELD_USER_POSTS         (5)
#define SQL_FIELD_USER_COMMENTS      (6)
#define SQL_FIELD_USER_DATE_JOINED   (7)

/* post sql field names */
#define SQL_FIELD_POST_ID             (0)
#define SQL_FIELD_POST_COMMUNITY_ID   (1)
#define SQL_FIELD_POST_COMMUNITY_NAME (2)
#define SQL_FIELD_POST_AUTHOR_ID      (3)
#define SQL_FIELD_POST_AUTHOR_NAME    (4)
#define SQL_FIELD_POST_TITLE          (5)
#define SQL_FIELD_POST_BODY           (6)
#define SQL_FIELD_POST_POINTS         (7)
#define SQL_FIELD_POST_DATE_POSTED    (8)

/* comment sql field names */
#define SQL_FIELD_COMMENT_ID             (0)
#define SQL_FIELD_COMMENT_POST_ID        (1)
#define SQL_FIELD_COMMENT_POST_TITLE     (2)
#define SQL_FIELD_COMMENT_COMMUNITY_NAME (3)
#define SQL_FIELD_COMMENT_AUTHOR_ID      (4)
#define SQL_FIELD_COMMENT_AUTHOR_NAME    (5)
#define SQL_FIELD_COMMENT_BODY           (6)
#define SQL_FIELD_COMMENT_POINTS         (7)
#define SQL_FIELD_COMMENT_DATE_POSTED    (8)

/* community sql field names */
#define SQL_FIELD_COMMUNITY_ID           (0)
#define SQL_FIELD_COMMUNITY_NAME         (1)
#define SQL_FIELD_COMMUNITY_ABOUT        (2)
#define SQL_FIELD_COMMUNITY_MEMBERS      (3)
#define SQL_FIELD_COMMUNITY_DATE_CREATED (4)

/* subscription field names */
#define SQL_FIELD_SUB_ID             (0)
#define SQL_FIELD_SUB_USER_ID        (1)
#define SQL_FIELD_SUB_USER_NAME      (2)
#define SQL_FIELD_SUB_COMMUNITY_ID   (2)
#define SQL_FIELD_SUB_COMMUNITY_NAME (4)

/* post up vote field names */
#define SQL_FIELD_POST_UP_VOTE_ID        (0)
#define SQL_FIELD_POST_UP_VOTE_POST_ID   (1)
#define SQL_FIELD_POST_UP_VOTE_USER_ID   (2)
#define SQL_FIELD_POST_UP_VOTE_USER_NAME (3)

/* post down vote field names */
#define SQL_FIELD_POST_DOWN_VOTE_ID        (0)
#define SQL_FIELD_POST_DOWN_VOTE_POST_ID   (1)
#define SQL_FIELD_POST_DOWN_VOTE_USER_ID   (2)
#define SQL_FIELD_POST_DOWN_VOTE_USER_NAME (3)

/* comment up vote field names */
#define SQL_FIELD_COMMENT_UP_VOTE_ID        (0)
#define SQL_FIELD_COMMENT_UP_VOTE_POST_ID   (1)
#define SQL_FIELD_COMMENT_UP_VOTE_USER_ID   (2)
#define SQL_FIELD_COMMENT_UP_VOTE_USER_NAME (3)

/* comment down vote field names */
#define SQL_FIELD_COMMENT_DOWN_VOTE_ID        (0)
#define SQL_FIELD_COMMENT_DOWN_VOTE_POST_ID   (1)
#define SQL_FIELD_COMMENT_DOWN_VOTE_USER_ID   (2)
#define SQL_FIELD_COMMENT_DOWN_VOTE_USER_NAME (3)

list** query_database_ls(char* query);

#endif

