echo -e "\e[1;1H\e[2J"  # https://stackoverflow.com/questions/2347770/how-do-you-clear-the-console-screen-in-c

while true; do

        echo -e "\e[1;1f"
        date -u

        echo "--------------------------------------"
        echo "Recorder1:"
        ssh recorder1 "cd vex ; tail -5 \`ls -1tr *.log | tail -1\`"
        echo -e "\n\n"

        echo "Recorder2:"
        ssh recorder2 "cd vex ; tail -5 \`ls -1tr *.log | tail -1\`"
        echo -e "\n\n"

        echo "Recorder3:"
        ssh recorder3 "cd vex ; tail -5 \`ls -1tr *.log | tail -1\`"
        echo -e "\n\n"

        echo "Recorder4:"
        ssh recorder4 "cd vex ; tail -5 \`ls -1tr *.log | tail -1\`"
        echo -e "\n\n"

        sleep 5

done

