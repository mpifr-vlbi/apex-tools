echo -e "\e[1;1H\e[2J"  # https://stackoverflow.com/questions/2347770/how-do-you-clear-the-console-screen-in-c

while true; do
        clear
        date -u

        echo "--------------------------------------"
        echo "Recorder1:"
        echo -e "record?;\n" | nc -q1 recorder1 14242
        ssh recorder1 "ls -ltr /mnt/disks/1/0/data/ | tail -1"
        ssh recorder1 "ls -ltr /mnt/disks/2/0/data/ | tail -1"
        ssh recorder1 "ls -ltr /mnt/disks/3/0/data/ | tail -1"
        ssh recorder1 "ls -ltr /mnt/disks/4/0/data/ | tail -1"
        echo

        echo "Recorder2:"
        echo -e "record?;\n" | nc -q1 recorder2 14242
        ssh recorder2 "ls -ltr /mnt/disks/1/0/data/ | tail -1"
        ssh recorder2 "ls -ltr /mnt/disks/2/0/data/ | tail -1"
        ssh recorder2 "ls -ltr /mnt/disks/3/0/data/ | tail -1"
        ssh recorder2 "ls -ltr /mnt/disks/4/0/data/ | tail -1"
        echo

        echo "Recorder3:"
        echo -e "record?;\n" | nc -q1 recorder3 14242
        ssh recorder3 "ls -ltr /mnt/disks/1/0/data/ | tail -1"
        ssh recorder3 "ls -ltr /mnt/disks/2/0/data/ | tail -1"
        ssh recorder3 "ls -ltr /mnt/disks/3/0/data/ | tail -1"
        ssh recorder3 "ls -ltr /mnt/disks/4/0/data/ | tail -1"
        echo

        echo "Recorder4:"
        echo -e "record?;\n" | nc -q1 recorder4 14242
        ssh recorder4 "ls -ltr /mnt/disks/1/0/data/ | tail -1"
        ssh recorder4 "ls -ltr /mnt/disks/2/0/data/ | tail -1"
        ssh recorder4 "ls -ltr /mnt/disks/3/0/data/ | tail -1"
        ssh recorder4 "ls -ltr /mnt/disks/4/0/data/ | tail -1"
        echo
done

