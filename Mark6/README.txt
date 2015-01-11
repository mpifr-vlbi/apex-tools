
VEX-drived recording on Mark6, scripts by Geoff Crew (MIT Hays.)

    create-schedule.py
	This is a fake schedule generator (--help) which can
	generate random Mark6 schedules.  All versions of Mark6
	for the last year have been beaten pretty hard with
	schedules so created.  It creates several products,
	principally an xml schedule file.

    m6cc.py
        When this is linked to /usr/local/bin/M6_CC one can
	use commands to da-client to upload the xml file and
	cplane will arrange for this script to execute the xml

Ideally

    vex2xml -f t15012.vex (with other station depedent arguments)

produces

    t15012.xml

and

    m6cc.py -f t15012.xml -r <Mbps>

will execute the schedule and log scan-checks scan info, etc.
