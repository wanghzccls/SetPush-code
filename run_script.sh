para1_arr=("0.5" "0.1")
para2_arr=(0.2)

#data_arr=("youtube")
data_arr=("toy")


rm SetPush
make


for data_name in "${data_arr[@]}"
do
	for ((j=0; j<${#para2_arr[@]}; j++))
	do
        for ((i=0; i<${#para1_arr[@]}; i++))
		do
			echo "./SetPush -d dataset/${data_name}/ -f ${data_name} -algo SetPush -er ${para1_arr[$i]} -alpha ${para2_arr[$j]} -qn 10 -qflag 1" |bash;
            echo "./SetPush -d dataset/${data_name}/ -f ${data_name} -algo SetPush -er ${para1_arr[$i]} -alpha ${para2_arr[$j]} -qn 10 -qflag 2" |bash;
		done
	done
done




