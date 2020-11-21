
mysql -uroot <<TEST_DATA

USE falcondb;

# create test users
INSERT INTO users (uname, about, points, posts, comments, bday)
  VALUES ('yopp', 'this is test user 1', 0, 0, 0, UNIX_TIMESTAMP());
INSERT INTO users (uname, about, points, posts, comments, bday)
  VALUES ('rolyat', 'this is test user 2', 0, 0, 0, UNIX_TIMESTAMP());
INSERT INTO users (uname, about, points, posts, comments, bday)
  VALUES ('beans', 'this is test user 3', 0, 0, 0, UNIX_TIMESTAMP());


# create test communities
INSERT INTO communities (name, about, bday)
  VALUES ('Hockey', 'this community is about all things hockey.', UNIX_TIMESTAMP());
INSERT INTO communities (name, about, bday)
  VALUES ('LOTR', 'this community is about The Lord of the Rings.', UNIX_TIMESTAMP());
INSERT INTO communities (name, about, bday)
  VALUES ('Halo', 'this community is about the Halo video game series', UNIX_TIMESTAMP());


# create test posts
INSERT INTO posts (communityid, communityname, authid, author, title, body, bday)
  VALUES (1, 'Hockey', 2, 'rolyat', 'Wayne Gretzky', 'You miss all the shots you don't take', UNIX_TIMESTAMP());
INSERT INTO posts (communityid, communityname, authid, author, title, body, bday)
  VALUES (2, 'LOTR', 3, 'beans', 'Aragorn', 'He was a real G.', UNIX_TIMESTAMP());
INSERT INTO posts (communityid, communityname, authid, author, title, body, bday)
  VALUES (3, 'Halo', 1, 'yopp', 'Arbiter', 'He was a real G too', UNIX_TIMESTAMP());


INSERT INTO comments (postid, posttitle, communityname, authid, author, body, bday)
  VALUES (1, 'Wayne Gretsky', 'Hockey', 1, 'yopp', 'this is a test comment', UNIX_TIMESTAMP());
INSERT INTO comments (postid, posttitle, communityname, authid, author, body, bday)
  VALUES (2, 'Aragorn', 'LOTR', 2, 'rolyat', 'this is a test comment', UNIX_TIMESTAMP());
INSERT INTO comments (postid, posttitle, communityname, authid, author, body, bday)
  VALUES (3, 'Arbiter', 'Halo', 3, 'beans', 'this is a test comment', UNIX_TIMESTAMP());


TEST_DATA
