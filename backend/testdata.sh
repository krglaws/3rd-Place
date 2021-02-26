
eval $(cat ./db.config)

mysql -uroot <<TEST_USERS

USE $DBNAME;

# test users
SET @foo = (Select CreateUser("testuser1", "", "this is test user 1"));
SET @foo = (Select CreateUser("testuser2", "", "this is test user 2"));
SET @foo = (Select CreateUser("testuser3", "", "this is test user 3"));

TEST_USERS

if [ $? -ne 0 ]; then
  echo "Error while loading test users"
  exit 1;
fi


mysql -uroot <<TEST_COMMUNITIES

USE $DBNAME;

SET @foo = (SELECT CreateCommunity(2, "TestCommunity1", "this is a test community"));
SET @foo = (SELECT CreateCommunity(3, "TestCommunity2", "this is a test community"));
SET @foo = (SELECT CreateCommunity(4, "TestCommunity3", "this is a test community"));

TEST_COMMUNITIES

if [ $? -ne 0 ]; then
  echo "Error while loading test communities"
  exit 1;
fi


mysql -uroot <<TEST_POSTS

USE $DBNAME;

SET @foo = (SELECT CreatePost(2, 1, "Test Post 1", "This is a test post."));
SET @foo = (SELECT CreatePost(2, 2, "Test Post 2", "This is a test post."));
SET @foo = (SELECT CreatePost(2, 3, "Test Post 3", "This is a test post."));

SET @foo = (SELECT CreatePost(3, 1, "Test Post 4", "This is a test post."));
SET @foo = (SELECT CreatePost(3, 2, "Test Post 5", "This is a test post."));
SET @foo = (SELECT CreatePost(3, 3, "Test Post 6", "This is a test post."));

SET @foo = (SELECT CreatePost(4, 1, "Test Post 7", "This is a test post."));
SET @foo = (SELECT CreatePost(4, 2, "Test Post 8", "This is a test post."));
SET @foo = (SELECT CreatePost(4, 3, "Test Post 9", "This is a test post."));

TEST_POSTS

if [ $? -ne 0 ]; then
  echo "Error while loading test posts"
  exit 1;
fi


mysql -uroot <<TEST_COMMENTS

USE $DBNAME;

SET @foo = (SELECT CreateComment(2, 4, 1, "this is a test comment"));
SET @foo = (SELECT CreateComment(2, 5, 2, "this is a test comment"));
SET @foo = (SELECT CreateComment(2, 6, 3, "this is a test comment"));
SET @foo = (SELECT CreateComment(2, 7, 1, "this is a test comment"));
SET @foo = (SELECT CreateComment(2, 8, 2, "this is a test comment"));
SET @foo = (SELECT CreateComment(2, 9, 3, "this is a test comment"));

SET @foo = (SELECT CreateComment(3, 7, 1, "this is a test comment"));
SET @foo = (SELECT CreateComment(3, 8, 2, "this is a test comment"));
SET @foo = (SELECT CreateComment(3, 9, 3, "this is a test comment"));
SET @foo = (SELECT CreateComment(3, 1, 1, "this is a test comment"));
SET @foo = (SELECT CreateComment(3, 2, 2, "this is a test comment"));
SET @foo = (SELECT CreateComment(3, 3, 3, "this is a test comment"));

SET @foo = (SELECT CreateComment(4, 1, 1, "this is a test comment"));
SET @foo = (SELECT CreateComment(4, 2, 2, "this is a test comment"));
SET @foo = (SELECT CreateComment(4, 3, 3, "this is a test comment"));
SET @foo = (SELECT CreateComment(4, 4, 1, "this is a test comment"));
SET @foo = (SELECT CreateComment(4, 5, 2, "this is a test comment"));
SET @foo = (SELECT CreateComment(4, 6, 3, "this is a test comment"));

TEST_COMMENTS

if [ $? -ne 0 ]; then
  echo "Error while loading test comments"
  exit 1;
fi


mysql -uroot <<TEST_SUBSCRIPTIONS

USE $DBNAME;

CALL ToggleSubscribe (1, 2);
CALL ToggleSubscribe (2, 2);
CALL ToggleSubscribe (3, 2);

CALL ToggleSubscribe (1, 3);
CALL ToggleSubscribe (2, 3);
CALL ToggleSubscribe (3, 3);

CALL ToggleSubscribe (1, 4);
CALL ToggleSubscribe (2, 4);
CALL ToggleSubscribe (3, 4);

TEST_SUBSCRIPTIONS

if [ $? -ne 0 ]; then
  echo "Error while loading test subscriptions"
  exit 1;
fi


mysql -uroot <<TEST_VOTES

USE $DBNAME;

# test votes

# testuser1 upvote TestPost1
CALL TogglePostUpVote(1, 2);

# testuser2 upvote TestPost1
CALL TogglePostUpVote(1, 3);

# testuser3 upvote TestPost1
CALL TogglePostUpVote(1, 4);

# testuser1 downvote TestPost2
CALL TogglePostDownVote(2, 2);

# testuser2 downvote TestPost2
CALL TogglePostDownVote(2, 3);

# testuser3 downvote TestPost2
CALL TogglePostDownVote(2, 4);

# testuser1 upvote comment 1
CALL ToggleCommentUpVote(1, 2);

# testuser2 upvote comment 1
CALL ToggleCommentUpvote(1, 3);

# testuser3 upvote comment 1
CALL ToggleCommentUpVote(1, 4);

# testuser1 downvote comment 2
CALL ToggleCommentDownVote(2, 2);

# testuser2 downvote comment 2
CALL ToggleCommentDownVote(2, 3);

# testuser3 downvote comment 2
CALL ToggleCommentDownVote(2, 4);

TEST_VOTES

if [ $? -ne 0 ]; then
  echo "Error while loading test votes"
  exit 1;
fi

