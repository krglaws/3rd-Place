
eval $(cat ./db.config)

mysql -uroot <<TEST_USERS

USE $DBNAME;

# test users
INSERT INTO users (id, name, about, points, posts, comments, date_joined)
  VALUES (1, 'testuser1', 'this is test user 1.', 0, 0, 0, UNIX_TIMESTAMP());
INSERT INTO users (id, name, about, points, posts, comments, date_joined)
  VALUES (2, 'testuser2', 'this is test user 2', 0, 0, 0, UNIX_TIMESTAMP());
INSERT INTO users (id, name, about, points, posts, comments, date_joined)
  VALUES (3, 'testuser3', 'this is test user 3', 0, 0, 0, UNIX_TIMESTAMP());
INSERT INTO users (id, name, about, points, posts, comments, date_joined)
  VALUES (4, 'testuser4', 'this is test user 4', 0, 0, 0, UNIX_TIMESTAMP());
INSERT INTO users (id, name, about, points, posts, comments, date_joined)
  VALUES (5, 'testuser5', 'this is test user 5', 0, 0, 0, UNIX_TIMESTAMP());

TEST_USERS

if [ $? -eq 1 ]; then
  echo "Error while loading test users"
fi

mysql -uroot <<TEST_POSTS

USE $DBNAME;

# test posts
INSERT INTO posts (community_id, community_name, author_id, author_name, title, body, points, date_posted)
  VALUES (1, 'TestCommunity1', 1, 'testuser1', 'Test Post 1', 'This is a test post.', 0, UNIX_TIMESTAMP());
INSERT INTO posts (community_id, community_name, author_id, author_name, title, body, points, date_posted)
  VALUES (2, 'TestCommunity2', 2, 'testuser2', 'Test Post 2', 'This is a test post.', 0, UNIX_TIMESTAMP());
INSERT INTO posts (community_id, community_name, author_id, author_name, title, body, points, date_posted)
  VALUES (3, 'TestCommunity3', 3, 'testuser3', 'Test Post 3', 'This is a test post.', 0, UNIX_TIMESTAMP());
INSERT INTO posts (community_id, community_name, author_id, author_name, title, body, points, date_posted)
  VALUES (4, 'TestCommunity4', 4, 'testuser4', 'Test Post 4', 'This is a test post.', 0, UNIX_TIMESTAMP());
INSERT INTO posts (community_id, community_name, author_id, author_name, title, body, points, date_posted)
  VALUES (5, 'TestCommunity1', 5, 'testuser5', 'Test Post 5', 'This is a test post.', 0, UNIX_TIMESTAMP());

TEST_POSTS

if [ $? -eq 1 ]; then
  echo "Error while loading test posts"
fi


mysql -uroot <<TEST_COMMENTS

USE $DBNAME;

# test comments
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (1, 1, 'Test Post 1', 'TestCommunity1', 2, 'testuser2', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (2, 1, 'Test Post 1', 'TestCommunity1', 3, 'testuser3', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (3, 1, 'Test Post 1', 'TestCommunity1', 4, 'testuser4', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (4, 1, 'Test Post 1', 'TestCommunity1', 5, 'testuser5', 'this is a test comment.', 0, UNIX_TIMESTAMP());

INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (5, 2, 'Test Post 2', 'TestCommunity2', 1, 'testuser1', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (6, 2, 'Test Post 2', 'TestCommunity2', 2, 'testuser2', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (7, 2, 'Test Post 2', 'TestCommunity2', 3, 'testuser3', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (8, 2, 'Test Post 2', 'TestCommunity2', 4, 'testuser4', 'this is a test comment.', 0, UNIX_TIMESTAMP());

INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (9, 3, 'Test Post 3', 'TestCommunity3', 5, 'testuser5', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (10, 3, 'Test Post 3', 'TestCommunity3', 1, 'testuser1', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (11, 3, 'Test Post 3', 'TestCommunity3', 2, 'testuser2', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (12, 3, 'Test Post 3', 'TestCommunity3', 3, 'testuser3', 'this is a test comment.', 0, UNIX_TIMESTAMP());

INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (13, 4, 'Test Post 4', 'TestCommunity4', 4, 'testuser4', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (14, 4, 'Test Post 4', 'TestCommunity4', 5, 'testuser5', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (15, 4, 'Test Post 4', 'TestCommunity4', 1, 'testuser1', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (16, 4, 'Test Post 4', 'TestCommunity4', 2, 'testuser2', 'this is a test comment.', 0, UNIX_TIMESTAMP());

INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (17, 5, 'Test Post 5', 'TestCommunity5', 3, 'testuser3', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (18, 5, 'Test Post 5', 'TestCommunity5', 4, 'testuser4', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (19, 5, 'Test Post 5', 'TestCommunity5', 5, 'testuser5', 'this is a test comment.', 0, UNIX_TIMESTAMP());
INSERT INTO comments (id, post_id, post_title, community_name, author_id, author_name, body, points, date_posted)
  VALUES (20, 5, 'Test Post 5', 'TestCommunity5', 1, 'testuser1', 'this is a test comment.', 0, UNIX_TIMESTAMP());

TEST_COMMENTS

if [ $? -eq 1 ]; then
  echo "Error while loading test comments"
fi

mysql -uroot <<TEST_COMMUNITIES

USE $DBNAME;

# test communities
INSERT INTO communities (id, name, about, members, date_created)
  VALUES (1, 'TestCommunity1', 'this is a test community.', 0, UNIX_TIMESTAMP());
INSERT INTO communities (id, name, about, members, date_created)
  VALUES (2, 'TestCommunity2', 'this is a test community.', 0, UNIX_TIMESTAMP());
INSERT INTO communities (id, name, about, members, date_created)
  VALUES (3, 'TestCommunity3', 'this is a test community.', 0, UNIX_TIMESTAMP());
INSERT INTO communities (id, name, about, members, date_created)
  VALUES (4, 'TestCommunity4', 'this is a test community.', 0, UNIX_TIMESTAMP());
INSERT INTO communities (id, name, about, members, date_created)
  VALUES (5, 'TestCommunity5', 'this is a test community.', 0, UNIX_TIMESTAMP());

TEST_COMMUNITIES

if [ $? -eq 1 ]; then
  echo "Error while loading test communities"
fi

mysql -uroot <<TEST_SUBSCRIPTIONS

USE $DBNAME;

# test subscriptions
INSERT INTO subscriptions (id, user_id, user_name, community_id, community_name)
  VALUES (1, 1, 'testuser1', 1, 'TestCommunity1');
INSERT INTO subscriptions (id, user_id, user_name, community_id, community_name)
  VALUES (2, 1, 'testuser1', 2, 'TestCommunity2');
INSERT INTO subscriptions (id, user_id, user_name, community_id, community_name)
  VALUES (3, 1, 'testuser1', 3, 'TestCommunity3');
INSERT INTO subscriptions (id, user_id, user_name, community_id, community_name)
  VALUES (4, 1, 'testuser1', 4, 'TestCommunity4');
INSERT INTO subscriptions (id, user_id, user_name, community_id, community_name)
  VALUES (5, 1, 'testuser1', 5, 'TestCommunity5');

TEST_SUBSCRIPTIONS

if [ $? -eq 1 ]; then
  echo "Error while loading test subscriptions"
fi

mysql -uroot <<TEST_VOTES

USE $DBNAME;

# test votes
INSERT INTO post_up_votes (id, user_id, user_name, post_id)
  VALUES (1, 1, 'testuser1', 1);
INSERT INTO post_up_votes (id, user_id, user_name, post_id)
  VALUES (2, 2, 'testuser2', 2);
INSERT INTO post_up_votes (id, user_id, user_name, post_id)
  VALUES (3, 3, 'testuser3', 3);
INSERT INTO post_down_votes (id, user_id, user_name, post_id)
  VALUES (4, 4, 'testuser4', 4);
INSERT INTO post_down_votes (id, user_id, user_name, post_id)
  VALUES (5, 5, 'testuser5', 5);

TEST_VOTES

if [ $? -eq 1 ]; then
  echo "Error while loading test votes"
fi

