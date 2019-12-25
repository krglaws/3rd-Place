
HOST=localhost
DB=falcondb
USER=falcon
PASS=falcon123

mysql -uroot <<INIT_SCRIPT

CREATE DATABASE IF NOT EXISTS $DB;

CREATE TABLE IF NOT EXISTS $DB.users(
	uuid INT NOT NULL AUTO_INCREMENT,
	uname VARCHAR(32) NOT NULL,
	bday DATETIME NOT NULL,
	PRIMARY KEY (uuid)
);

CREATE TABLE IF NOT EXISTS $DB.posts(
	uuid INT NOT NULL AUTO_INCREMENT,
	commid INT NOT NULL,
	authid INT NOT NULL,
	title VARCHAR(256) NOT NULL,
	body VARCHAR(1024) NOT NULL,
	bday DATETIME NOT NULL,
	PRIMARY KEY (uuid)
);

CREATE TABLE IF NOT EXISTS $DB.comments(
	uuid INT NOT NULL AUTO_INCREMENT,
	postid INT NOT NULL,
	authid INT NOT NULL,
	body VARCHAR(512) NOT NULL,
	bday DATETIME NOT NULL,
	PRIMARY KEY (uuid)
);

CREATE TABLE IF NOT EXISTS $DB.communities(
	uuid INT NOT NULL AUTO_INCREMENT,
	name VARCHAR(32) NOT NULL,
	about VARCHAR(512) NOT NULL,
	bday DATETIME NOT NULL,
	PRIMARY KEY (uuid)
);

CREATE TABLE IF NOT EXISTS $DB.subs(
	uuid INT NOT NULL,
	commid INT NOT NULL
);

CREATE USER IF NOT EXISTS $USER@$HOST IDENTIFIED BY "$PASS";
GRANT ALL PRIVILEGES ON $DB.* TO $USER@$HOST;

FLUSH PRIVILEGES;

INIT_SCRIPT

