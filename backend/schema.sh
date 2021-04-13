#!/bin/bash

# read database configuration file
eval $(cat db.config)

mysql -uroot <<SCHEMA

########
# schema
########

CREATE DATABASE IF NOT EXISTS $DBNAME;

CREATE TABLE IF NOT EXISTS $DBNAME.users(
  id INT NOT NULL AUTO_INCREMENT,
  name VARCHAR(16) NOT NULL,
  password_hash VARCHAR(128) NOT NULL,
  about VARCHAR(1280),
  points INT DEFAULT 0,
  posts INT DEFAULT 0,
  comments INT DEFAULT 0,
  date_joined INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.posts(
  id INT NOT NULL AUTO_INCREMENT,
  community_id INT NOT NULL,
  community_name VARCHAR(32) NOT NULL,
  author_id INT NOT NULL,
  author_name VARCHAR(16),
  title VARCHAR(160) NOT NULL,
  body VARCHAR(2560),
  points INT DEFAULT 0,
  date_posted INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.comments(
  id INT NOT NULL AUTO_INCREMENT,
  post_id INT NOT NULL,
  post_title VARCHAR(160) NOT NULL,
  community_id INT NOT NULL,
  community_name VARCHAR(32) NOT NULL,
  author_id INT NOT NULL,
  author_name VARCHAR(16),
  body VARCHAR(1280) NOT NULL,
  points INT DEFAULT 0,
  date_posted INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.communities(
  id INT NOT NULL AUTO_INCREMENT,
  owner_id INT NOT NULL,
  owner_name VARCHAR(16) NOT NULL,
  name VARCHAR(32) NOT NULL,
  about VARCHAR(2560) NOT NULL,
  members INT DEFAULT 0,
  date_created INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.moderators(
  id INT NOT NULL AUTO_INCREMENT,
  user_id INT NOT NULL,
  community_id INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.administrators(
  user_id INT NOT NULL,
  PRIMARY KEY (user_id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.subscriptions(
  id INT NOT NULL AUTO_INCREMENT,
  user_id INT NOT NULL,
  community_id INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.post_up_votes(
  id INT NOT NULL AUTO_INCREMENT,
  user_id INT NOT NULL,
  post_id INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.post_down_votes(
  id INT NOT NULL AUTO_INCREMENT,
  user_id INT NOT NULL,
  post_id INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.comment_up_votes(
  id INT NOT NULL AUTO_INCREMENT,
  user_id INT NOT NULL,
  comment_id INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS $DBNAME.comment_down_votes(
  id INT NOT NULL AUTO_INCREMENT,
  user_id INT NOT NULL,
  comment_id INT NOT NULL,
  PRIMARY KEY (id)
);


# create user [nemo], owner of deleted communities and posts
INSERT INTO $DBNAME.users (id, name, password_hash, date_joined) VALUES (1, "[nemo]", "ab", UNIX_TIMESTAMP());

# create admin user
INSERT INTO $DBNAME.users (id, name, password_hash, date_joined) VALUES(2, "$ADMINUNAME", "`mkpasswd -S $ADMINPWSALT $ADMINPW`", UNIX_TIMESTAMP());
INSERT INTO $DBNAME.administrators (user_id) VALUES (2);

# create community [deleted], parent of posts/comments with deleted community
INSERT INTO $DBNAME.communities (id, name, owner_id, owner_name, about, date_created)
VALUES (1, "[deleted]", 1, "[nemo]", "[deleted]", UNIX_TIMESTAMP());


CREATE USER IF NOT EXISTS $DBUSER@$DBHOST IDENTIFIED BY '$DBPASS';
GRANT ALL PRIVILEGES ON $DBNAME.* TO $DBUSER@$DBHOST;

FLUSH PRIVILEGES;

SCHEMA
