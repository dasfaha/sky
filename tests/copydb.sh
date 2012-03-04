# This script is run by the tests to setup a database. It must be run from the
# project root.
rm -rf tmp/db
cp -r tests/fixtures/db/$1 tmp/db
