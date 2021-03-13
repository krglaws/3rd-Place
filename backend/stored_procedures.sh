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

CREATE FUNCTION CreateUser(name VARCHAR(16), pwh VARCHAR(128), about VARCHAR(256))
RETURNS INT
BEGIN

  INSERT INTO users (name, password_hash, about, points, posts, comments, date_joined)
  VALUES (name, pwh, about, 0, 0, 0, UNIX_TIMESTAMP());

  RETURN LAST_INSERT_ID();

END;$$


CREATE FUNCTION CreateComment(uid INT, pid INT, body VARCHAR(512))
RETURNS INT
BEGIN

  DECLARE user_name VARCHAR(16);
  DECLARE post_title VARCHAR(32);
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


CREATE FUNCTION CreatePost(uid INT, cid INT, title VARCHAR(32), body VARCHAR(512))
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


CREATE FUNCTION CreateCommunity(uid INT, name VARCHAR(32), about VARCHAR(512))
RETURNS INT
BEGIN

  DECLARE user_name VARCHAR(16);

  # get user name
  SELECT name INTO user_name FROM users WHERE id = uid;

  # create new community
  INSERT INTO communities (owner_id, owner_name, name, about, date_created)
  VALUES (uid, user_name, name, about, UNIX_TIMESTAMP());

  RETURN LAST_INSERT_ID();

END;$$


###################
# Delete procedures
###################

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
  UPDATE posts SET author_name = "[deleted]", author_id = 0 WHERE author_id = uid;

  # remove references in comments
  UPDATE comments SET author_name = "[deleted]", author_id = 0 WHERE author_id = uid;

  # remove references in communities
  UPDATE communities SET owner_name = "[deleted]", owner_id = 0 WHERE author_id = uid;

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


CREATE PROCEDURE DeleteCommunity (IN cid INT) NOT DETERMINISTIC
proc_label:BEGIN

  DECLARE id INT;
  DECLARE finished INT DEFAULT 0;
  DECLARE post_cursor CURSOR FOR SELECT id FROM posts WHERE community_id = cid;
  DECLARE comment_cursor CURSOR FOR SELECT id FROM comments WHERE community_id = cid;
  DECLARE CONTINUE HANDLER FOR NOT FOUND SET finished = 1;

  # delete subscriptions
  DELETE FROM subscriptions WHERE community_id = cid;

  # delete moderators
  DELETE FROM moderators WHERE community_id = cid;

  # delete community
  DELETE FROM communities WHERE id = cid;

  # delete all community posts
  OPEN post_cursor;
  get_post:LOOP

    FETCH post_cursor INTO id;

    IF finished = 1 THEN
      LEAVE get_post;
    END IF;

    CALL DeletePost(id);

  END LOOP get_post;
  CLOSE post_cursor;

  SET finished = 0;

  # delete all community comments
  OPEN comment_cursor;
  get_comment:LOOP

    FETCH comment_cursor INTO id;

    IF finished = 1 THEN
      LEAVE get_comment;
    END IF;

    CALL DeleteComment(id);

  END LOOP get_comment;
  CLOSE comment_cursor;

END;$$


DELIMITER ;

STORED_PROCEDURES
