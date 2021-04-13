
eval $(cat ./db.config)

mysql -uroot <<TEST_USERS

USE $DBNAME;

# test users
SET @foo = (Select CreateUser("testuser1", ""));
CALL UpdateUserAbout(@foo, "this is test user 1");

SET @foo = (Select CreateUser("testuser2", ""));
CALL UpdateUserAbout(@foo, "this is test user 2");

SET @foo = (Select CreateUser("testuser3", ""));
CALL UpdateUserAbout(@foo, "this is test user 3");

TEST_USERS

if [ $? -ne 0 ]; then
  echo "Error while loading test users"
  exit 1;
fi


mysql -uroot <<TEST_COMMUNITIES

USE $DBNAME;

SET @foo = (SELECT CreateCommunity(3, "TestCommunity1", "this is a test community"));
SET @foo = (SELECT CreateCommunity(4, "TestCommunity2", "this is a test community"));
SET @foo = (SELECT CreateCommunity(5, "TestCommunity3", "this is a test community"));

TEST_COMMUNITIES

if [ $? -ne 0 ]; then
  echo "Error while loading test communities"
  exit 1;
fi


mysql -uroot <<TEST_POSTS

USE $DBNAME;

SET @foo = (SELECT CreatePost(3, 2, "Test Post 1", "This is a test post."));
SET @foo = (SELECT CreatePost(3, 3, "Test Post 2", "This is a test post."));
SET @foo = (SELECT CreatePost(3, 4, "Test Post 3", "This is a test post."));

SET @foo = (SELECT CreatePost(4, 2, "Test Post 4", "This is a test post."));
SET @foo = (SELECT CreatePost(4, 3, "Test Post 5", "This is a test post."));
SET @foo = (SELECT CreatePost(4, 4, "Test Post 6", "This is a test post."));

SET @foo = (SELECT CreatePost(5, 2, "Test Post 7", "This is a test post."));
SET @foo = (SELECT CreatePost(5, 3, "Test Post 8", "This is a test post."));
SET @foo = (SELECT CreatePost(5, 4, "Test Post 9", "This is a test post."));

TEST_POSTS

if [ $? -ne 0 ]; then
  echo "Error while loading test posts"
  exit 1;
fi


mysql -uroot <<TEST_COMMENTS

USE $DBNAME;

SET @foo = (SELECT CreateComment(3, 4, "this is a test comment"));
SET @foo = (SELECT CreateComment(3, 5, "this is a test comment"));
SET @foo = (SELECT CreateComment(3, 6, "this is a test comment"));
SET @foo = (SELECT CreateComment(3, 7, "this is a test comment"));
SET @foo = (SELECT CreateComment(3, 8, "this is a test comment"));
SET @foo = (SELECT CreateComment(3, 9, "this is a test comment"));

SET @foo = (SELECT CreateComment(4, 7, "this is a test comment"));
SET @foo = (SELECT CreateComment(4, 8, "this is a test comment"));
SET @foo = (SELECT CreateComment(4, 9, "this is a test comment"));
SET @foo = (SELECT CreateComment(4, 1, "this is a test comment"));
SET @foo = (SELECT CreateComment(4, 2, "this is a test comment"));
SET @foo = (SELECT CreateComment(4, 3, "this is a test comment"));

SET @foo = (SELECT CreateComment(5, 1, "this is a test comment"));
SET @foo = (SELECT CreateComment(5, 2, "this is a test comment"));
SET @foo = (SELECT CreateComment(5, 3, "this is a test comment"));
SET @foo = (SELECT CreateComment(5, 4, "this is a test comment"));
SET @foo = (SELECT CreateComment(5, 5, "this is a test comment"));
SET @foo = (SELECT CreateComment(5, 6, "this is a test comment"));

TEST_COMMENTS

if [ $? -ne 0 ]; then
  echo "Error while loading test comments"
  exit 1;
fi


mysql -uroot <<TEST_SUBSCRIPTIONS

USE $DBNAME;

CALL ToggleSubscription (2, 3);
CALL ToggleSubscription (3, 3);
CALL ToggleSubscription (4, 3);

CALL ToggleSubscription (2, 4);
CALL ToggleSubscription (3, 4);
CALL ToggleSubscription (4, 4);

CALL ToggleSubscription (2, 5);
CALL ToggleSubscription (3, 5);
CALL ToggleSubscription (4, 5);

TEST_SUBSCRIPTIONS

if [ $? -ne 0 ]; then
  echo "Error while loading test subscriptions"
  exit 1;
fi


mysql -uroot <<TEST_VOTES

USE $DBNAME;

# test votes

# testuser1 upvote TestPost1
CALL TogglePostUpVote(1, 3);

# testuser2 upvote TestPost1
CALL TogglePostUpVote(1, 4);

# testuser3 upvote TestPost1
CALL TogglePostUpVote(1, 5);

# testuser1 downvote TestPost2
CALL TogglePostDownVote(2, 3);

# testuser2 downvote TestPost2
CALL TogglePostDownVote(2, 4);

# testuser3 downvote TestPost2
CALL TogglePostDownVote(2, 5);

# testuser1 upvote comment 1
CALL ToggleCommentUpVote(1, 3);

# testuser2 upvote comment 1
CALL ToggleCommentUpvote(1, 4);

# testuser3 upvote comment 1
CALL ToggleCommentUpVote(1, 5);

# testuser1 downvote comment 2
CALL ToggleCommentDownVote(2, 3);

# testuser2 downvote comment 2
CALL ToggleCommentDownVote(2, 4);

# testuser3 downvote comment 2
CALL ToggleCommentDownVote(2, 5);

TEST_VOTES

if [ $? -ne 0 ]; then
  echo "Error while loading test votes"
  exit 1;
fi

