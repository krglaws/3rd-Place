#!/bin/bash

# read database configuration file
eval $(cat ./db.config)


mysql -uroot <<STORED_PROCEDURES

# allow non-deterministic functions
SET GLOBAL log_bin_trust_function_creators = 1;

###################
# stored procedures
###################

USE $DBNAME;
DELIMITER $$


###################
# Toggle* procedures
###################

DROP PROCEDURE IF EXISTS ToggleSubscription;
CREATE PROCEDURE ToggleSubscription (IN cid INT, IN uid INT) NOT DETERMINISTIC
BEGIN

  # check if user is already subscribed
  IF EXISTS (SELECT * FROM subscriptions WHERE community_id = cid AND user_id = uid) THEN
    DELETE FROM subscriptions WHERE community_id = cid AND user_id = uid;
    UPDATE communities SET members = members - 1 WHERE id = cid;
  ELSE
    INSERT INTO subscriptions (user_id, community_id) VALUES (uid, cid);
    UPDATE communities SET members = members + 1 WHERE id = cid;
  END IF;

END;$$


DROP PROCEDURE IF EXISTS TogglePostUpVote;
CREATE PROCEDURE TogglePostUpVote (IN pid INT, IN uid INT) NOT DETERMINISTIC
proc_label:BEGIN

  # get post author id
  DECLARE aid INT;
  SELECT author_id INTO aid FROM posts WHERE id = pid;

  # check if upvote exists
  IF EXISTS (SELECT * FROM post_up_votes WHERE post_id = pid AND user_id = uid) THEN
    DELETE FROM post_up_votes WHERE post_id = pid AND user_id = uid;

    # update points
    UPDATE posts SET points = points - 1 WHERE id = pid;
    UPDATE users SET points = points - 1 WHERE id = aid;
    LEAVE proc_label;
  END IF;

  # check if downvote exists
  IF EXISTS (SELECT * FROM post_down_votes WHERE post_id = pid AND user_id = uid) THEN
    DELETE FROM post_down_votes WHERE post_id = pid AND user_id = uid;

    # update points
    UPDATE posts SET points = points + 1 WHERE id = pid;
    UPDATE users SET points = points + 1 WHERE id = aid;
  END IF;

  # create upvote entry
  INSERT INTO post_up_votes (post_id, user_id) VALUES (pid, uid);

  # update points
  UPDATE posts SET points = points + 1 WHERE id = pid;
  UPDATE users SET points = points + 1 WHERE id = aid;

  LEAVE proc_label;

END;$$


DROP PROCEDURE IF EXISTS TogglePostDownVote;
CREATE PROCEDURE TogglePostDownVote (IN pid INT, IN uid INT) NOT DETERMINISTIC
proc_label:BEGIN

  # get post author id
  DECLARE aid INT;
  SELECT author_id INTO aid FROM posts WHERE id = pid;

  # check if downvote exists
  IF EXISTS (SELECT * FROM post_down_votes WHERE post_id = pid AND user_id = uid) THEN
    DELETE FROM post_down_votes WHERE post_id = pid AND user_id = uid;

    # update points
    UPDATE posts SET points = points + 1 WHERE id = pid;
    UPDATE users SET points = points + 1 WHERE id = aid;
    LEAVE proc_label;
  END IF;

  # check if upvote exists
  IF EXISTS (SELECT * FROM post_up_votes WHERE post_id = pid AND user_id = uid) THEN
    DELETE FROM post_up_votes WHERE post_id = pid AND user_id = uid;

    # update points
    UPDATE posts SET points = points - 1 WHERE id = pid;
    UPDATE users SET points = points - 1 WHERE id = aid;
  END IF;

  # create downvote entry
  INSERT INTO post_down_votes (post_id, user_id) VALUES (pid, uid);

  # update points
  UPDATE posts SET points = points - 1 WHERE id = pid;
  UPDATE users SET points = points - 1 WHERE id = aid;

  LEAVE proc_label;

END;$$


DROP PROCEDURE IF EXISTS ToggleCommentUpVote;
CREATE PROCEDURE ToggleCommentUpVote(IN cid INT, IN uid INT) NOT DETERMINISTIC
proc_label:BEGIN

  # get comment author id
  DECLARE aid INT;
  SELECT author_id INTO aid FROM comments WHERE id = cid;

  # check if upvote exists
  IF EXISTS (SELECT * FROM comment_up_votes WHERE comment_id = cid AND user_id = uid) THEN
    DELETE FROM comment_up_votes WHERE comment_id = cid AND user_id = uid;

    # update points
    UPDATE comments SET points = points - 1 WHERE id = cid;
    UPDATE users SET points = points - 1 WHERE id = aid;
    LEAVE proc_label;
  END IF;

  # check if downvote exists
  IF EXISTS (SELECT * FROM comment_down_votes WHERE comment_id = cid AND user_id = uid) THEN
    DELETE FROM comment_down_votes WHERE comment_id = cid AND user_id = uid;

    # update points
    UPDATE comments SET points = points + 1 WHERE id = cid;
    UPDATE users SET points = points + 1 WHERE id = aid;
  END IF;

  # create upvote entry
  INSERT INTO comment_up_votes (comment_id, user_id) VALUES (cid, uid);

  # update points
  UPDATE comments SET points = points + 1 WHERE id = cid;
  UPDATE users SET points = points + 1 WHERE id = aid;

  LEAVE proc_label;

END;$$


DROP PROCEDURE IF EXISTS ToggleCommentDownVote;
CREATE PROCEDURE ToggleCommentDownVote(IN cid INT, IN uid INT) NOT DETERMINISTIC
proc_label:BEGIN

  # get comment author id
  DECLARE aid INT;
  SELECT author_id INTO aid FROM comments WHERE id = cid;

  # check if downvote exists
  IF EXISTS (SELECT * FROM comment_down_votes WHERE comment_id = cid AND user_id = uid) THEN
    DELETE FROM comment_down_votes WHERE comment_id = cid AND user_id = uid;

    # update points
    UPDATE comments SET points = points + 1 WHERE id = cid;
    UPDATE users SET points = points + 1 WHERE id = aid;
    LEAVE proc_label;
  END IF;

  # check if upvote exists
  IF EXISTS (SELECT * FROM comment_up_votes WHERE comment_id = cid AND user_id = uid) THEN
    DELETE FROM comment_up_votes WHERE comment_id = cid AND user_id = uid;

    # update points
    UPDATE comments SET points = points - 1 WHERE id = cid;
    UPDATE users SET points = points - 1 WHERE id = aid;
  END IF;

  # create downvote entry
  INSERT INTO comment_down_votes (comment_id, user_id) VALUES (cid, uid);

  # update points
  UPDATE comments SET points = points - 1 WHERE id = cid;
  UPDATE users SET points = points - 1 WHERE id = aid;

  LEAVE proc_label;

END;$$


###################
# Create procedures
###################

DROP FUNCTION IF EXISTS CreateUser;
CREATE FUNCTION CreateUser(name VARCHAR(16), pwh VARCHAR(128))
RETURNS INT
BEGIN

  INSERT INTO users (name, password_hash, about, points, posts, comments, date_joined)
  VALUES (name, pwh, "", 0, 0, 0, UNIX_TIMESTAMP());

  RETURN LAST_INSERT_ID();

END;$$


DROP FUNCTION IF EXISTS CreateComment;
CREATE FUNCTION CreateComment(uid INT, pid INT, body VARCHAR(2560))
RETURNS INT
BEGIN

  DECLARE user_name VARCHAR(16);
  DECLARE post_title VARCHAR(640);
  DECLARE cname VARCHAR(32);
  DECLARE cid INT;

  # get user name
  SELECT name INTO user_name FROM users WHERE id = uid;

  # get post title, community name, and community ID
  SELECT title, community_name, community_id INTO post_title, cname, cid FROM posts WHERE id = pid;

  # update user comment count
  UPDATE users SET comments = comments + 1 WHERE id = uid;

  # create new comment
  INSERT INTO comments (post_id, post_title, community_id, community_name, author_id, author_name, body, date_posted)
  VALUES (pid, post_title, cid, cname, uid, user_name, body, UNIX_TIMESTAMP());

  RETURN LAST_INSERT_ID();

END;$$


DROP FUNCTION IF EXISTS CreatePost;
CREATE FUNCTION CreatePost(uid INT, cid INT, title VARCHAR(640), body VARCHAR(2560))
RETURNS INT
BEGIN

  DECLARE user_name VARCHAR(16);
  DECLARE community_name VARCHAR(32);

  # get user name
  SELECT name INTO user_name FROM users WHERE id = uid;

  # get community name
  SELECT name INTO community_name FROM communities WHERE id = cid;

  # increment user post count
  UPDATE users SET posts = posts + 1 WHERE id = uid;

  # create new post
  INSERT INTO posts (community_id, community_name, author_id, author_name, title, body, date_posted)
  VALUES (cid, community_name, uid, user_name, title, body, UNIX_TIMESTAMP());

  RETURN LAST_INSERT_ID();

END;$$


DROP FUNCTION IF EXISTS CreateCommunity;
CREATE FUNCTION CreateCommunity(uid INT, cname VARCHAR(32), about VARCHAR(2560))
RETURNS INT
BEGIN

  DECLARE user_name VARCHAR(16);

  # get user name
  SELECT name INTO user_name FROM users WHERE id = uid;

  # create new community
  INSERT INTO communities (owner_id, owner_name, name, about, date_created)
  VALUES (uid, user_name, cname, about, UNIX_TIMESTAMP());

  RETURN LAST_INSERT_ID();

END;$$


DROP FUNCTION IF EXISTS CreateModerator;
CREATE FUNCTION CreateModerator(uid INT, cid INT)
RETURNS INT
BEGIN

  DECLARE uname VARCHAR(16);
  DECLARE cname VARCHAR(16);

  # get names
  SELECT name INTO uname FROM users WHERE id = uid;
  SELECT name INTO cname FROM communities WHERE id = cid;

  INSERT INTO moderators (user_id, user_name, community_id, community_name)
  VALUES (uid, uname, cid, cname);

  RETURN LAST_INSERT_ID();

END;$$


###################
# Update procedures
###################

DROP PROCEDURE IF EXISTS UpdateUserAbout;
CREATE PROCEDURE UpdateUserAbout(uid INT, new_about VARCHAR(1280))
BEGIN

  UPDATE users SET about = new_about WHERE id = uid;

END;$$


DROP PROCEDURE IF EXISTS UpdateUserPassword;
CREATE PROCEDURE UpdateUserPassword(uid INT, new_pwh VARCHAR(128))
BEGIN

  UPDATE users SET password_hash = npw WHERE id = uid;

END;$$


DROP PROCEDURE IF EXISTS UpdatePost;
CREATE PROCEDURE UpdatePost(post_id INT, new_body VARCHAR(2560))
BEGIN

  UPDATE posts SET body = new_body WHERE id = post_id;

END;$$


DROP PROCEDURE IF EXISTS UpdateComment;
CREATE PROCEDURE UpdateComment(comment_id INT, new_body VARCHAR(1280))
BEGIN

  UPDATE comments SET body = new_body WHERE id = comment_id;

END;


DROP PROCEDURE IF EXISTS UpdateCommunityAbout;
CREATE PROCEDURE UpdateCommunityAbout(community_id INT, new_about VARCHAR(2560))
BEGIN

  UPDATE communities SET about = new_about WHERE id = community_id;

END;$$


###################
# Delete procedures
###################

DROP PROCEDURE IF EXISTS DeleteUser;
CREATE PROCEDURE DeleteUser(IN uid INT) NOT DETERMINISTIC
BEGIN

  # unsubscribe from all communities
  DECLARE finished INT DEFAULT 0;
  DECLARE community_id INTEGER DEFAULT 0;

  DECLARE sub_cursor CURSOR FOR SELECT community_id FROM subscriptions WHERE user_id = uid;
  DECLARE CONTINUE HANDLER FOR NOT FOUND SET finished = 1;

  # remove user
  DELETE FROM users WHERE id = uid;
  DELETE FROM moderators WHERE user_id = uid;
  DELETE FROM administrators WHERE user_id = uid;

  # remove references in posts
  UPDATE posts SET author_name = "[deleted]", author_id = 1 WHERE author_id = uid;

  # remove references in comments
  UPDATE comments SET author_name = "[deleted]", author_id = 1 WHERE author_id = uid;

  # remove references in communities
  UPDATE communities SET owner_name = "[deleted]", owner_id = 1 WHERE owner_id = uid;

  # remove references from votes
  UPDATE post_up_votes SET user_id = 0 WHERE user_id = uid;
  UPDATE post_down_votes SET user_id = 0 WHERE user_id = uid;
  UPDATE comment_up_votes SET user_id = 0 WHERE user_id = uid;
  UPDATE comment_down_votes SET user_id = 0 WHERE user_id = uid;

  OPEN sub_cursor;

  get_sub:LOOP

    FETCH sub_cursor INTO community_id;
    IF finished = 1 THEN
      LEAVE get_sub;
    END IF;

    CALL ToggleSubscription(uid, community_id);

  END LOOP get_sub;

  CLOSE sub_cursor;

END;$$


DROP PROCEDURE IF EXISTS DeleteComment;
CREATE PROCEDURE DeleteComment (IN cid INT) NOT DETERMINISTIC
BEGIN

  # grab author id
  DECLARE aid INT;
  SELECT author_id INTO aid FROM comments WHERE id = cid;

  # delete comment
  DELETE FROM comments WHERE id = cid;

  # decrement user comment count
  UPDATE users SET comments = comments - 1 WHERE id = aid;

END;$$


DROP PROCEDURE IF EXISTS DeletePost;
CREATE PROCEDURE DeletePost (IN pid INT) NOT DETERMINISTIC
BEGIN

  # grab author id
  DECLARE aid INT;
  SELECT author_id INTO aid FROM posts WHERE id = pid;

  # if no comments, just delete the entire post
  IF NOT EXISTS (SELECT * FROM comments WHERE post_id = pid) THEN
    DELETE FROM posts WHERE id = pid;
  ELSE
    UPDATE posts SET author_id = 1, author_name = "[deleted]", body = "[deleted]" WHERE id = pid;
  END IF;

  # decrement user post count
  UPDATE users SET posts = posts - 1 WHERE id = aid;

END;$$


DROP PROCEDURE IF EXISTS DeleteCommunity;
CREATE PROCEDURE DeleteCommunity (IN cid INT) NOT DETERMINISTIC
BEGIN

  # set community posts and comments to [deleted] community
  UPDATE posts SET community_id = 1, community_name = "[deleted]" WHERE community_id = cid;
  UPDATE comments SET community_id = 1, community_name = "[deleted]" WHERE community_id = cid;

  # delete subscriptions
  DELETE FROM subscriptions WHERE community_id = cid;

  # delete moderators
  DELETE FROM moderators WHERE community_id = cid;

  # delete community
  DELETE FROM communities WHERE id = cid;

END;$$


DROP PROCEDURE IF EXISTS DeleteModerator;
CREATE PROCEDURE DeleteModerator (uid INT, cid INT) NOT DETERMINISTIC
BEGIN

  DELETE FROM moderators WHERE user_id = uid AND community_id = cid;

END;$$


DELIMITER ;

STORED_PROCEDURES
