# 3rd Place
This is meant to be a Reddit-like content sharing website with a backend written entirely in C, and a bare-bones frontend built with only basic HTML, CSS, and JavaScript. For some reason I really like to do everything myself. Probably unnecessary, but I like the challenge. The backend is about 60% complete (estimating here). So far it can serve up templated files for the front-end in response to GET requests, but no capacity for POST, DELETE, or UPDATE yet. A rough sketch of the database schema has been mapped out, but probably needs work. Not screenshots yet, but hoping to get this website done by January.


# Workspace Setup

1. clone repository:

`git clone https://github.com/krglaws/3rd-Place`

2. install mysql + client library:

`sudo apt install mysql libmysqlclient-dev`

3. configure the database by editing 3rd-Place/backend/db.config 

4. execute initdb.sh script:

`cd 3rd-Place/backend/; sudo ./initdb.sh`

5. execute testdata.sh script:

`sudo ./testdata.sh`

6. compile backend:

`make debug`

7. run server from root project dir:

`cd ../;backend/serverdebug.out`
