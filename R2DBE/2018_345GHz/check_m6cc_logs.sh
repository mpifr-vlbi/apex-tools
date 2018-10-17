
echo -e "\e[1;1H\e[2J"  # https://stackoverflow.com/questions/2347770/how-do-you-clear-the-console-screen-in-c

vexroot=/home/oper/vex/October2018/

while true; do

	# echo -e "\e[1;1f"
	clear
	date -u

	echo "--------------------------------------"
	echo "Recorder1:"
	ssh recorder1 "cd $vexroot ; tail -5 \`ls -1tr *.log | tail -1\`"
	echo -e "\n\n"

	echo "Recorder2:"
	ssh recorder2 "cd $vexroot ; tail -5 \`ls -1tr *.log | tail -1\`"
	echo -e "\n\n"

	echo "Recorder3:"
	ssh recorder3 "cd $vexroot ; tail -5 \`ls -1tr *.log | tail -1\`"
	echo -e "\n\n"

## skip in 345G obs Oct2018
#	echo "Recorder4:"
#        ssh recorder4 "cd $vexroot ; tail -5 \`ls -1tr *.log | tail -1\`"
#	echo -e "\n\n"

	sleep 5

done
