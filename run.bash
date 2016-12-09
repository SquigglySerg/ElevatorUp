make clean
make
# printf "\n ************check STOPS with ELEVATORS=FLOORS=PEOPLE=1 should output 2************ \n"
# printf " ************need to change PEOPLE from 100 to 1 in SIM.cpp************ \n\n"
# ./SIM 1 1 0.1 15 5 uniform-0-1-00.dat 1
# ./SIM 2 2 0.1 15 5 uniform-0-1-00.dat 2
# ./SIM 3 3 0 15 5 uniform-0-1-00.dat 100

# echo "./SIM 2 1 0 15 5 uniform-0-1-00.dat 1"
# ./SIM 2 1 0 15 5 uniform-0-1-00.dat 1


# echo "./SIM 3 1 0 15 5 uniform-0-1-00.dat 1"
# ./SIM 3 1 0 15 5 uniform-0-1-00.dat 1


# ./SIM 2 4 0.1 15 5 uniform-0-1-00.dat 300
#
# ./SIM 5 4 0.1 15 5 uniform-0-1-00.dat 300


#
# for j in `seq 4 15`;
for j in `seq 6 10`;
do
  for i in `seq 1 50`;
  do
    # echo "./SIM $j $i 0.1 15 5 uniform-0-1-00.dat 300"
    ./SIM $j $i 0 15 5 uniform-0-1-00.dat 300 > results.txt
    # cat results.txt
    lineNum=`cat results.txt | wc -l`
    if (($lineNum <= 14)); then  # 14 is based on how many lines are outputted
      # echo All under 6 minutes

      # echo $i
      # cat results.txt
      # echo $lineNum
      SPLUSONE=$(($i+1))

      echo //////////////////////////////////////////////////
      echo ////////// FLOORS = $j, S = $i, S+1 = $SPLUSONE ////////////
      echo //////////////////////////////////////////////////
      echo ./SIM $j $SPLUSONE 0 15 5 uniform-0-1-00.dat 300
      ./SIM $j $SPLUSONE 0 15 5 uniform-0-1-00.dat 300 > results.txt
      lineNum=`cat results.txt | wc -l`
      cat results.txt
      # echo $lineNum
      echo ./SIM $j $i 0.07 15 5 uniform-0-1-00.dat 300
      ./SIM $j $i 0.07 15 5 uniform-0-1-00.dat 300 > results.txt
      cat results.txt
      break;
    fi


  done
done
