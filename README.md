# 3rd Place
[live website](http://3rd-Place.krgdev.xyz)

This is the repository for my community-oriented discussion website. Users can sign up, create communities, subscribe to communities, write posts in communities, write comments on posts, and vote on posts and comments. Yes, just like reddit. The frontend is currently text only. There is no capacity for sharing images besides pasting links to them into posted content.

In truth, this website is more of a learning experience and a toy for me rather something that was meant to be actually useful to anyone, although I do welcome people to use it as they please.

If you are interested in looking at the code, I would start by glancing at the database schema definition in [schema.sh](/backend/schema.sh). That will give you an idea as to the "what" of this website. After that I recommend looking at [http_get.c](/backend/src/http_get.c) and [http_post.c](/backend/src/http_post.c), as those two files contain all possible endpoints that the server will respond to, and will give you a sense of the "how". Each endpoint is associated with a similarly named C function, all of which return response structures that are processed and sent to the client via [server.c](/backend/src/server.c). To find the source code for each of the endpoint functions, look in the respective post_\*.c or get_\*.c files, or I guess just grep for the function you want to look at. If you're like me and want to find main() as quickly as possible, just take a look in server.c.

I should also mention that I made heavy use of a data structures library that I wrote called [KyleStructs](http://github.com/krglaws/kylestructs), and if you see functions with a leading "ks_", that indicates that it belongs to libkylestructs. There is also a small amount of wrapper code in src/string_map.c that makes life a little easier when using kylestructs. This project is by no means perfect, absolutely has a few issues, and would by no means be good for a real production website, but really the point of this thing was to have fun, and hopefully learn a few things along the way.

# Contributing
TBH I'm a little tired of working on this, and will probably just leave the site to languish where it is. However, if some random nerd decides to make a few changes and submits a pull request, I would absolutely be okay with looking it over and merging if it looks good. Collaboration is always fun.

# Workspace Setup
Here are some quick instructions on how to set up the project on your linux machine for local development and/or experimentation:
1. Clone, build and install [KyleStructs](https://github.com/krglaws/kylestructs)

2. install mysql server, and the mysql client library:

`sudo apt install mysql libmysqlclient-dev` (This works on my Kubuntu installation, but I've had to do this slightly differently on Mint)

3. configure the database by editing backend/db.config, or just leave the defaults if ya want

4. execute initdb.sh script:

`cd 3rd-Place/backend/; sudo ./initdb.sh`

5. compile backend:

`make`

6. run server from root project dir:

`cd ../;sudo backend/server`
