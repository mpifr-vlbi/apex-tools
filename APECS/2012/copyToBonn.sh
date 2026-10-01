#!/bin/bash

tar czvf ~/code_revisions/vlbi_scripts_05may2012.tar.gz ~
rsync -axv ~/* jwagner@portal.mpifr-bonn.mpg.de:/homes/jwagner/apex/apecs/2012/
