project(testscripts)

SET(SCRIPTS testUsers.py)



add_library(testscripts ${version_file} ${SCRIPTS})
