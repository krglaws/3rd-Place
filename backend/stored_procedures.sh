#!/bin/bash

# read database configuration file
eval $(cat ./db.config)


mysql -uroot <<STORED_PROCEDURES

###################
# stored procedures
###################

DELIMITER $$


CREATE PROCEDURE $DBNAME.UpvotePost (IN postid INT, IN user_name VARCHAR(16))
proc_label:BEGIN

  # check if upvote exists
  IF EXISTS (SELECT * FROM $DBNAME.post_up_votes WHERE post_id = postid AND name = user_name) THEN
    DELETE FROM $DBNAME.post_up_votes WHERE post_id = postid AND name = user_name;
    UPDATE $DBNAME.posts SET points  =  points - 1 WHERE id = postid;
    LEAVE proc_label;
  END IF;

  # check if downvote exists
  IF EXISTS (SELECT * FROM $DBNAME.post_down_votes WHERE post_id = postid AND name = user_name) THEN
    DELETE FROM $DBNAME.post_down_votes WHERE post_id = postid AND name = user_name;
    UPDATE $DBNAME.posts SET points = points + 1 WHERE id = postid;
  END IF;

  # create upvote entry
  INSERT INTO $DBNAME.post_up_votes (post_id, user_id)
    VALUES (postid, userid);

  UPDATE $DBNAME.posts SET points = points + 1 WHERE id = postid;

END$$


CREATE PROCEDURE $DBNAME.DownvotePost (IN postid INT, IN user_name VARCHAR(16))
proc_label:BEGIN

  # check if downvote exists
  IF EXISTS (SELECT * FROM $DBNAME.post_down_votes WHERE post_id = postid AND name = user_name) THEN
    DELETE FROM $DBNAME.post_down_votes WHERE post_id = postid AND name = user_name;
    UPDATE $DBNAME.posts SET points = points + 1 WHERE id = postid;
    LEAVE proc_label;
  END IF;

  # check if upvote exists
  IF EXISTS (SELECT * FROM $DBNAME.post_up_votes WHERE post_id = postid AND name = user_name) THEN
    DELETE FROM $DBNAME.post_up_votes WHERE post_id = postid AND name = user_name;
    UPDATE $DBNAME.posts SET points = points - 1 WHERE id = postid;
  END IF;

  # create downvote entry
  INSERT INTO $DBNAME.post_down_votes (post_id, user_id)
    VALUES (postid, userid);

  UPDATE $DBNAME.posts SET points = points - 1 WHERE id = postid;

END$$


CREATE PROCEDURE $DBNAME.UpvoteComment (IN commentid INT, IN user_name VARCHAR(16))
proc_label:BEGIN

  # check if upvote exists
  IF EXISTS (SELECT * FROM $DBNAME.comment_up_votes WHERE comment_id = commentid AND name = user_name) THEN
    DELETE FROM $DBNAME.comment_up_votes WHERE comment_id = commentid AND name = user_name;
    UPDATE $DBNAME.comments SET points = points - 1 WHERE id = commentid;
    LEAVE proc_label;
  END IF;

  # check if downvote exists
  IF EXISTS (SELECT * FROM $DBNAME.comment_down_votes WHERE comment_id = commentid AND name = user_name) THEN
    DELETE FROM $DBNAME.comment_down_votes WHERE comment_id = commentid AND name = user_name;
    UPDATE $DBNAME.comments SET points  =  points + 1 WHERE id = commentid;
  END IF;

  # create upvote entry
  INSERT INTO $DBNAME.comment_up_votes (comment_id, user_id)
    VALUES (commentid, userid);

  UPDATE $DBNAME.comments SET points = points + 1 WHERE id = commentid;

END$$


CREATE PROCEDURE $DBNAME.DownvoteComment (IN commentid INT, IN user_name VARCHAR(16))
proc_label:BEGIN

  # check if downvote exists
  IF EXISTS (SELECT * FROM $DBNAME.comment_down_votes WHERE comment_id = commentid AND name = user_name) THEN
    DELETE FROM $DBNAME.comment_down_votes WHERE comment_id = commentid AND name = user_name;
    UPDATE $DBNAME.comments SET points = points + 1 WHERE id = commentid;
    LEAVE proc_label;
  END IF;

  # check if upvote exists
  IF EXISTS (SELECT * FROM $DBNAME.comment_up_votes WHERE comment_id = commentid AND name = user_name) THEN
    DELETE FROM $DBNAME.comment_up_votes WHERE comment_id = commentid AND name = user_name;
    UPDATE $DBNAME.comments SET points = points - 1 WHERE id = commentid;
  END IF;

  # create downvote entry
  INSERT INTO $DBNAME.comment_down_votes (comment_id, user_id)
    VALUES (commentid, userid);

  UPDATE $DBNAME.comments SET points = points - 1 WHERE id = commentid;

END$$


DELIMITER ;

STORED_PROCEDURES
