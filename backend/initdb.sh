#!/bin/bash

eval $(cat db.config)

mysql -uroot <<INIT_SCRIPT

########
# schema
########

CREATE DATABASE IF NOT EXISTS $DBNAME;

CREATE TABLE IF NOT EXISTS $DBNAME.users(
  id INT NOT NULL AUTO_INCREMENT,
  name VARCHAR(16) NOT NULL,
  password_hash VARCHAR(128) NOT NULL,
  about VARCHAR(256),
  points INT NOT NULL,
  posts INT NOT NULL,
  comments INT NOT NULL,
  date_joined INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.posts(
  id INT NOT NULL AUTO_INCREMENT,
  community_id INT NOT NULL,
  community_name VARCHAR(32) NOT NULL,
  author_id INT NOT NULL,
  author_name VARCHAR(16),
  title VARCHAR(32) NOT NULL,
  body VARCHAR(512),
  points INT NOT NULL,
  date_posted INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.comments(
  id INT NOT NULL AUTO_INCREMENT,
  post_id INT NOT NULL,
  post_title VARCHAR(32) NOT NULL,
  community_name VARCHAR(32) NOT NULL,
  author_id INT NOT NULL,
  author_name VARCHAR(16),
  body VARCHAR(512) NOT NULL,
  points INT NOT NULL,
  date_posted INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.communities(
  id INT NOT NULL AUTO_INCREMENT,
  name VARCHAR(32) NOT NULL,
  about VARCHAR(512) NOT NULL,
  members INT NOT NULL,
  date_created INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.subscriptions(
  id INT NOT NULL AUTO_INCREMENT,
  user_id INT NOT NULL,
  user_name VARCHAR(16) NOT NULL,
  community_id INT NOT NULL,
  community_name VARCHAR(32) NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.post_up_votes(
  id INT NOT NULL AUTO_INCREMENT,
  post_id INT NOT NULL,
  user_id INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.post_down_votes(
  id INT NOT NULL AUTO_INCREMENT,
  post_id INT NOT NULL,
  user_id INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.comment_up_votes(
  id INT NOT NULL AUTO_INCREMENT,
  comment_id INT NOT NULL,
  user_id INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.comment_down_votes(
  id INT NOT NULL AUTO_INCREMENT,
  comment_id INT NOT NULL,
  user_id INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE USER IF NOT EXISTS $DBUSER@$DBHOST IDENTIFIED BY '$DBPASS';
GRANT ALL PRIVILEGES ON $DBNAME.* TO $DBUSER@$DBHOST;

FLUSH PRIVILEGES;


###################
# stored procedures
###################

DELIMITER $$


CREATE PROCEDURE $DBNAME.UpvotePost (IN postid INT, IN userid INT)
proc_label:BEGIN

  # check if upvote exists
  IF EXISTS (SELECT * FROM $DBNAME.post_up_votes WHERE post_id = postid AND user_id = userid) THEN
    DELETE FROM $DBNAME.post_up_votes WHERE post_id = postid AND user_id = userid;
    UPDATE $DBNAME.posts SET points  =  points - 1 WHERE id = postid;
    LEAVE proc_label;
  END IF;

  # check if downvote exists
  IF EXISTS (SELECT * FROM $DBNAME.post_down_votes WHERE post_id = postid AND user_id = userid) THEN
    DELETE FROM $DBNAME.post_down_votes WHERE post_id = postid AND user_id = userid;
    UPDATE $DBNAME.posts SET points = points + 1 WHERE id = postid;
  END IF;

  # create upvote entry
  INSERT INTO $DBNAME.post_up_votes (post_id, user_id)
    VALUES (postid, userid);

  UPDATE $DBNAME.posts SET points = points + 1 WHERE id = postid;

END$$


CREATE PROCEDURE $DBNAME.DownvotePost (IN postid INT, IN userid INT)
proc_label:BEGIN

  # check if downvote exists
  IF EXISTS (SELECT * FROM $DBNAME.post_down_votes WHERE post_id = postid AND user_id = userid) THEN
    DELETE FROM $DBNAME.post_down_votes WHERE post_id = postid AND user_id = userid;
    UPDATE $DBNAME.posts SET points = points + 1 WHERE id = postid;
    LEAVE proc_label;
  END IF;

  # check if upvote exists
  IF EXISTS (SELECT * FROM $DBNAME.post_up_votes WHERE post_id = postid AND user_id = userid) THEN
    DELETE FROM $DBNAME.post_up_votes WHERE post_id = postid AND user_id = userid;
    UPDATE $DBNAME.posts SET points = points - 1 WHERE id = postid;
  END IF;

  # create downvote entry
  INSERT INTO $DBNAME.post_down_votes (post_id, user_id)
    VALUES (postid, userid);

  UPDATE $DBNAME.posts SET points = points - 1 WHERE id = postid;

END$$


CREATE PROCEDURE $DBNAME.UpvoteComment (IN commentid INT, IN userid INT)
proc_label:BEGIN

  # check if upvote exists
  IF EXISTS (SELECT * FROM $DBNAME.comment_up_votes WHERE comment_id = commentid AND user_id = userid) THEN
    DELETE FROM $DBNAME.comment_up_votes WHERE comment_id = commentid AND user_id = userid;
    UPDATE $DBNAME.comments SET points = points - 1 WHERE id = commentid;
    LEAVE proc_label;
  END IF;

  # check if downvote exists
  IF EXISTS (SELECT * FROM $DBNAME.comment_down_votes WHERE comment_id = commentid AND user_id = userid) THEN
    DELETE FROM $DBNAME.comment_down_votes WHERE comment_id = commentid AND user_id = userid;
    UPDATE $DBNAME.comments SET points  =  points + 1 WHERE id = commentid;
  END IF;

  # create upvote entry
  INSERT INTO $DBNAME.comment_up_votes (comment_id, user_id)
    VALUES (commentid, userid);

  UPDATE $DBNAME.comments SET points = points + 1 WHERE id = commentid;

END$$


CREATE PROCEDURE $DBNAME.DownvoteComment (IN commentid INT, IN userid INT)
proc_label:BEGIN

  # check if downvote exists
  IF EXISTS (SELECT * FROM $DBNAME.comment_down_votes WHERE comment_id = commentid AND user_id = userid) THEN
    DELETE FROM $DBNAME.comment_down_votes WHERE comment_id = commentid AND user_id = userid;
    UPDATE $DBNAME.comments SET points = points + 1 WHERE id = commentid;
    LEAVE proc_label;
  END IF;

  # check if upvote exists
  IF EXISTS (SELECT * FROM $DBNAME.comment_up_votes WHERE comment_id = commentid AND user_id = userid) THEN
    DELETE FROM $DBNAME.comment_up_votes WHERE comment_id = commentid AND user_id = userid;
    UPDATE $DBNAME.comments SET points = points - 1 WHERE id = commentid;
  END IF;

  # create downvote entry
  INSERT INTO $DBNAME.comment_down_votes (comment_id, user_id)
    VALUES (commentid, userid);

  UPDATE $DBNAME.comments SET points = points - 1 WHERE id = commentid;

END$$


DELIMITER ;

INIT_SCRIPT
