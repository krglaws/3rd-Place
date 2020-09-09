
mysql -uroot <<TEST_DATA

USE falcondb;

INSERT INTO users (uname, about, points, posts, comments, bday)
	VALUES ('testuser1', 'this is a test user', 2, 1, 1, UNIX_TIMESTAMP());

INSERT INTO communities (name, about, bday)
	VALUES ('TestCommunity', 'this is a test community', UNIX_TIMESTAMP());

INSERT INTO posts (communityid, communityname, authid, author, title, body, bday)
	VALUES (1, 'TestCommunity', 1, 'testuser', 'Test Post', 'this is a test post', UNIX_TIMESTAMP());

INSERT INTO comments (postid, posttitle, authid, author, body, bday)
	VALUES (1, 'Test Post', 1, 'testuser', 'this is a test comment', UNIX_TIMESTAMP());

INSERT INTO subs (userid, communityid)
	VALUES (1, 1);

TEST_DATA

