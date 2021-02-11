# 3rd Place
This is meant to be a Reddit-like CRUD website with a backend written in C, and a simplistic frontend.


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
