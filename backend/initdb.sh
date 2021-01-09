#!/bin/bash

eval $(cat ./db.config)

mysql -uroot <<WIPE_DB
DROP DATABASE $DBNAME;
WIPE_DB

# install database schema
./schema.sh

# load stored procedures
./stored_procedures.sh

# load test data
./testdata.sh
