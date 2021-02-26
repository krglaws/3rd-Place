#!/bin/bash

eval $(cat ./db.config)

mysql -uroot <<WIPE_DB
DROP DATABASE $DBNAME;
WIPE_DB

# install database schema
./schema.sh

if [ $? -ne 0 ]; then
  echo "Error while loading schema"
  exit 1;
fi

# load stored procedures
./stored_procedures.sh

if [ $? -ne 0 ]; then
  echo "Error while loading procedures"
  exit 1;
fi

# load test data
./testdata.sh

if [ $? -ne 0 ]; then
  echo "Error while loading test data"
  exit 1;
fi
