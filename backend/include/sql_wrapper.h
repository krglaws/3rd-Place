#ifndef _SQL_WRAPPER_H_
#define _SQL_WRAPPER_H_

#include <kylestructs.h>

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

/* post down vote field names */
#define SQL_FIELD_POST_DOWN_VOTE_ID        (0)
#define SQL_FIELD_POST_DOWN_VOTE_POST_ID   (1)
#define SQL_FIELD_POST_DOWN_VOTE_USER_ID   (2)

/* comment up vote field names */
#define SQL_FIELD_COMMENT_UP_VOTE_ID        (0)
#define SQL_FIELD_COMMENT_UP_VOTE_POST_ID   (1)
#define SQL_FIELD_COMMENT_UP_VOTE_USER_ID   (2)

/* comment down vote field names */
#define SQL_FIELD_COMMENT_DOWN_VOTE_ID        (0)
#define SQL_FIELD_COMMENT_DOWN_VOTE_POST_ID   (1)
#define SQL_FIELD_COMMENT_DOWN_VOTE_USER_ID   (2)

/* query templates */
#define QUERY_USER_BY_UNAME "SELECT * FROM users WHERE name = '%s';"
#define QUERY_POSTS_BY_UNAME "SELECT * FROM posts WHERE author_name = '%s';"
#define QUERY_COMMENTS_BY_UNAME "SELECT * FROM comments WHERE author_name = '%s';"
#define QUERY_POST_BY_ID "SELECT * FROM posts WHERE id = '%s';"
#define QUERY_POSTS_BY_COMMUNITY_NAME "SELECT * FROM posts WHERE community_name = '%s';"
#define QUERY_COMMENTS_BY_POSTID "SELECT * FROM comments WHERE post_id = '%s';"
#define QUERY_COMMUNITY_BY_NAME "SELECT * FROM communities WHERE name = '%s';"

#define QUERY_POST_UPVOTE_BY_POSTID_USERID "SELECT * FROM post_up_votes WHERE post_id = %s AND user_id = %d;"
#define QUERY_POST_DOWNVOTE_BY_POSTID_USERID "SELECT * FROM post_down_votes WHERE post_id = %s AND user_id = %d;"

#define QUERY_COMMENT_UPVOTE_BY_COMMENTID_USERID "SELECT * FROM comment_up_votes WHERE comment_id = %s AND user_id = %d"
#define QUERY_COMMENT_DOWNVOTE_BY_COMMENTID_USERID "SELECT * FROM comment_down_votes WHERE comment_id = %s AND user_id = %d;"

#define INSERT_NEW_USER "INSERT INTO users (name, password_hash, date_joined) VALUES ('%s', '%s', UNIX_TIMESTAMP());"
#define INSERT_NEW_COMMENT "";
#define INSERT_NEW_POST "";
#define INSERT_NEW_COMMUNITY "";

list** query_database_ls(char* query);

void delete_query_result(list** result);

#endif
