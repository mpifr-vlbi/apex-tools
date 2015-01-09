#!/bin/bash

tar czvf vlbi_scripts_21march2013.tar.gz * --exclude code_revisions --exclude="*.tar.gz" --exclude="*.avi"
mv vlbi_scripts_21march2013.tar.gz code_revisions/

rsync -Laxv --exclude "*.avi" ~/* jwagner@portal.mpifr-bonn.mpg.de:/homes/jwagner/apex/apecs/2013/

