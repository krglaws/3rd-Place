#!/bin/bash

# read database configuration file
eval $(cat ./db.config)


mysql -uroot <<STORED_PROCEDURES

###################
# stored procedures
###################

DELIMITER $$


CREATE PROCEDURE $DBNAME.TogglePostUpVote (IN pid INT, IN uid INT)
proc_label:BEGIN

  # get post owner id
  SET @owner_id = (SELECT author_id FROM posts WHERE id = pid);

  # check if upvote exists
  IF EXISTS (SELECT * FROM $DBNAME.post_up_votes WHERE post_id = pid AND user_id = uid) THEN
    DELETE FROM $DBNAME.post_up_votes WHERE post_id = pid AND user_id = uid;

    # update points
    UPDATE $DBNAME.posts SET points = points - 1 WHERE id = pid;
    UPDATE $DBNAME.users SET points = points - 1 WHERE id = @owner_id;
    LEAVE proc_label;
  END IF;

  # check if downvote exists
  IF EXISTS (SELECT * FROM $DBNAME.post_down_votes WHERE post_id = pid AND user_id = uid) THEN
    DELETE FROM $DBNAME.post_down_votes WHERE post_id = pid AND user_id = uid;

    # update points
    UPDATE $DBNAME.posts SET points = points + 1 WHERE id = pid;
    UPDATE $DBNAME.users SET points = points + 1 WHERE id = @owner_id;
  END IF;

  # create upvote entry
  INSERT INTO $DBNAME.post_up_votes (post_id, user_id) VALUES (pid, uid);

  # update points
  UPDATE $DBNAME.posts SET points = points + 1 WHERE id = pid;
  UPDATE $DBNAME.users SET points = points + 1 WHERE id = @owner_id;

END$$


CREATE PROCEDURE $DBNAME.TogglePostDownVote (IN pid INT, IN uid INT)
proc_label:BEGIN

  # get post owner id
  SET @owner_id = (SELECT author_id FROM posts WHERE id = pid);

  # check if downvote exists
  IF EXISTS (SELECT * FROM $DBNAME.post_down_votes WHERE post_id = pid AND user_id = uid) THEN
    DELETE FROM $DBNAME.post_down_votes WHERE post_id = pid AND user_id = uid;

    # update points
    UPDATE $DBNAME.posts SET points = points + 1 WHERE id = pid;
    UPDATE $DBNAME.users SET points = points + 1 WHERE id = @owner_id;
    LEAVE proc_label;
  END IF;

  # check if upvote exists
  IF EXISTS (SELECT * FROM $DBNAME.post_up_votes WHERE post_id = pid AND user_id = uid) THEN
    DELETE FROM $DBNAME.post_up_votes WHERE post_id = pid AND user_id = uid;

    # update points
    UPDATE $DBNAME.posts SET points = points - 1 WHERE id = pid;
    UPDATE $DBNAME.users SET points = points - 1 WHERE id = @owner_id;
  END IF;

  # create downvote entry
  INSERT INTO $DBNAME.post_down_votes (post_id, user_id) VALUES (pid, uid);

  # update points
  UPDATE $DBNAME.posts SET points = points - 1 WHERE id = pid;
  UPDATE $DBNAME.users SET points = points - 1 WHERE id = @owner_id;

END$$


CREATE PROCEDURE $DBNAME.ToggleCommentUpVote(IN cid INT, IN uid INT)
proc_label:BEGIN

  # get post owner id
  SET @owner_id = (SELECT author_id FROM comments WHERE id = cid);

  # check if upvote exists
  IF EXISTS (SELECT * FROM $DBNAME.comment_up_votes WHERE comment_id = cid AND user_id = uid) THEN
    DELETE FROM $DBNAME.comment_up_votes WHERE comment_id = cid AND user_id = uid;

    # update points
    UPDATE $DBNAME.comments SET points = points - 1 WHERE id = cid;
    UPDATE $DBNAME.users SET points = points - 1 WHERE id = @owner_id;
    LEAVE proc_label;
  END IF;

  # check if downvote exists
  IF EXISTS (SELECT * FROM $DBNAME.comment_down_votes WHERE comment_id = cid AND user_id = uid) THEN
    DELETE FROM $DBNAME.comment_down_votes WHERE comment_id = cid AND user_id = uid;

    # update points
    UPDATE $DBNAME.comments SET points = points + 1 WHERE id = cid;
    UPDATE $DBNAME.users SET points = points + 1 WHERE id = @owner_id;
  END IF;

  # create upvote entry
  INSERT INTO $DBNAME.comment_up_votes (comment_id, user_id) VALUES (cid, uid);

  # update points
  UPDATE $DBNAME.comments SET points = points + 1 WHERE id = cid;
  UPDATE $DBNAME.users SET points = points + 1 WHERE id = @owner_id;

END$$


CREATE PROCEDURE $DBNAME.ToggleCommentDownVote(IN cid INT, IN uid INT)
proc_label:BEGIN

  # get post owner id
  SET @owner_id = (SELECT author_id FROM comments WHERE id = cid);

  # check if downvote exists
  IF EXISTS (SELECT * FROM $DBNAME.comment_down_votes WHERE comment_id = cid AND user_id = uid) THEN
    DELETE FROM $DBNAME.comment_down_votes WHERE comment_id = cid AND user_id = uid;

    # update points
    UPDATE $DBNAME.comments SET points = points + 1 WHERE id = cid;
    UPDATE $DBNAME.users SET points = points + 1 WHERE id = @owner_id;
    LEAVE proc_label;
  END IF;

  # check if upvote exists
  IF EXISTS (SELECT * FROM $DBNAME.comment_up_votes WHERE comment_id = cid AND user_id = uid) THEN
    DELETE FROM $DBNAME.comment_up_votes WHERE comment_id = cid AND user_id = uid;

    # update points
    UPDATE $DBNAME.comments SET points = points - 1 WHERE id = cid;
    UPDATE $DBNAME.users SET points = points - 1 WHERE id = @owner_id;
  END IF;

  # create downvote entry
  INSERT INTO $DBNAME.comment_down_votes (comment_id, user_id) VALUES (cid, uid);

  # update points
  UPDATE $DBNAME.comments SET points = points - 1 WHERE id = cid;
  UPDATE $DBNAME.users SET points = points - 1 WHERE id = @owner_id;

END$$


DELIMITER ;

STORED_PROCEDURES
