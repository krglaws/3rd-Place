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

CREATE USER IF NOT EXISTS $DBUSER@$DBHOST IDENTIFIED BY '$DBPASS';
GRANT ALL PRIVILEGES ON $DBNAME.* TO $DBUSER@$DBHOST;

FLUSH PRIVILEGES;

SCHEMA
