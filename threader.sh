#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
END='\033[0m'

# CREDITS: https://unix.stackexchange.com/questions/103920/parallelize-a-bash-for-loop

# initialize a semaphore with a given number of tokens
open_sem(){
    mkfifo pipe-$$
    exec 3<>pipe-$$
    rm pipe-$$
    local i=$1
    for((;i>0;i--)); do
        printf %s 000 >&3
    done
}

# run the given command asynchronously and pop/push tokens
run_with_lock(){
    local x
    # this read waits until there is something to read
    read -u 3 -n 3 x && ((0==x)) || exit $x
    (
     	( "$@"; )
    	# push the return code of the command to the semaphore
    	printf '%.3d' $? >&3
	echo -ne "[+] $@ - Done\r"
    )&
}

N=6 # set the max amount of jobs to execute in parallel (alternatively we could've used a tool such as `parallel`)
open_sem $N

echo "(Deleting previous results...)"
rm -rf {006..020}.txt

echo ""
echo ">>> Processing now ..."
echo ""

for part in $(ls x{a..o}{a..z}.part 2>/dev/null); do
    run_with_lock ./a.out $part
done

wait # wait for all childs to complete

echo ""
echo ""
echo ">>> COMPLETE"
echo

orig=`ls -la  x{a..o}{a..z}.part 2>/dev/null | tr -s ' ' |cut -d ' ' -f 5 | awk '{s+=$1} END {printf "%.0f\n", s}'`
parsed=`ls -la  {006..020}.txt | tr -s ' ' |cut -d ' ' -f 5 | awk '{s+=$1} END {printf "%.0f\n", s}'`

if [ $orig = $parsed ]; then
	printf "${GREEN}[+] INTEGRITY CHECK SUCCESSFULL\n"
else
	printf "${RED}[!] Integrity check failed:\n"
	printf "\tExpected : $orig\n"
	printf "\tGot      : $parsed\n"
fi
printf "${END}"
